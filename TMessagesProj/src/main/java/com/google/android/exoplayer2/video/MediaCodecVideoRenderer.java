/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.google.android.exoplayer2.video;

import static android.view.Display.DEFAULT_DISPLAY;
import static com.google.android.exoplayer2.decoder.DecoderReuseEvaluation.DISCARD_REASON_MAX_INPUT_SIZE_EXCEEDED;
import static com.google.android.exoplayer2.decoder.DecoderReuseEvaluation.DISCARD_REASON_VIDEO_MAX_RESOLUTION_EXCEEDED;
import static com.google.android.exoplayer2.decoder.DecoderReuseEvaluation.REUSE_RESULT_NO;
import static com.google.android.exoplayer2.util.Assertions.checkNotNull;
import static com.google.android.exoplayer2.util.Assertions.checkState;
import static java.lang.Math.max;
import static java.lang.Math.min;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Point;
import android.hardware.display.DisplayManager;
import android.media.MediaCodec;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.media.MediaCrypto;
import android.media.MediaFormat;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Pair;
import android.view.Display;
import android.view.Surface;

import androidx.annotation.CallSuper;
import androidx.annotation.DoNotInline;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.FormatHolder;
import com.google.android.exoplayer2.PlayerMessage.Target;
import com.google.android.exoplayer2.RendererCapabilities;
import com.google.android.exoplayer2.decoder.DecoderCounters;
import com.google.android.exoplayer2.decoder.DecoderInputBuffer;
import com.google.android.exoplayer2.decoder.DecoderReuseEvaluation;
import com.google.android.exoplayer2.decoder.DecoderReuseEvaluation.DecoderDiscardReasons;
import com.google.android.exoplayer2.drm.DrmInitData;
import com.google.android.exoplayer2.mediacodec.MediaCodecAdapter;
import com.google.android.exoplayer2.mediacodec.MediaCodecDecoderException;
import com.google.android.exoplayer2.mediacodec.MediaCodecInfo;
import com.google.android.exoplayer2.mediacodec.MediaCodecRenderer;
import com.google.android.exoplayer2.mediacodec.MediaCodecSelector;
import com.google.android.exoplayer2.mediacodec.MediaCodecUtil;
import com.google.android.exoplayer2.mediacodec.MediaCodecUtil.DecoderQueryException;
import com.google.android.exoplayer2.util.Log;
import com.google.android.exoplayer2.util.MediaFormatUtil;
import com.google.android.exoplayer2.util.MimeTypes;
import com.google.android.exoplayer2.util.TraceUtil;
import com.google.android.exoplayer2.util.Util;
import com.google.android.exoplayer2.video.VideoRendererEventListener.EventDispatcher;
import com.google.common.collect.ImmutableList;

import java.nio.ByteBuffer;
import java.util.List;

/**
 * Decodes and renders video using {@link MediaCodec}.
 *
 * <p>This renderer accepts the following messages sent via {@link ExoPlayer#createMessage(Target)}
 * on the playback thread:
 *
 * <ul>
 *   <li>Message with type {@link #MSG_SET_VIDEO_OUTPUT} to set the output. The message payload
 *       should be the target {@link Surface}, or null to clear the output. Other non-null payloads
 *       have the effect of clearing the output.
 *   <li>Message with type {@link #MSG_SET_SCALING_MODE} to set the video scaling mode. The message
 *       payload should be one of the integer scaling modes in {@link C.VideoScalingMode}. Note that
 *       the scaling mode only applies if the {@link Surface} targeted by this renderer is owned by
 *       a {@link android.view.SurfaceView}.
 *   <li>Message with type {@link #MSG_SET_CHANGE_FRAME_RATE_STRATEGY} to set the strategy used to
 *       call {@link Surface#setFrameRate}.
 *   <li>Message with type {@link #MSG_SET_VIDEO_FRAME_METADATA_LISTENER} to set a listener for
 *       metadata associated with frames being rendered. The message payload should be the {@link
 *       VideoFrameMetadataListener}, or null.
 * </ul>
 */
public class MediaCodecVideoRenderer extends MediaCodecRenderer {

  private static final String TAG = "MediaCodecVideoRenderer";
  private static final String KEY_CROP_LEFT = "crop-left";
  private static final String KEY_CROP_RIGHT = "crop-right";
  private static final String KEY_CROP_BOTTOM = "crop-bottom";
  private static final String KEY_CROP_TOP = "crop-top";

  private static final int[] STANDARD_LONG_EDGE_VIDEO_PX =
      new int[] {1920, 1600, 1440, 1280, 960, 854, 640, 540, 480};

  /**
   * Scale factor for the initial maximum input size used to configure the codec in non-adaptive
   * playbacks. See {@link #getCodecMaxValues(MediaCodecInfo, Format, Format[])}.
   */
  private static final float INITIAL_FORMAT_MAX_INPUT_SIZE_SCALE_FACTOR = 1.5f;

  /** Magic frame render timestamp that indicates the EOS in tunneling mode. */
  private static final long TUNNELING_EOS_PRESENTATION_TIME_US = Long.MAX_VALUE;

  /** The minimum input buffer size for HEVC. */
  private static final int HEVC_MAX_INPUT_SIZE_THRESHOLD = 2 * 1024 * 1024;

  private static boolean evaluatedDeviceNeedsSetOutputSurfaceWorkaround;
  private static boolean deviceNeedsSetOutputSurfaceWorkaround;

  private final Context context;
  private final VideoFrameReleaseHelper frameReleaseHelper;
  private final EventDispatcher eventDispatcher;
  private final long allowedJoiningTimeMs;
  private final int maxDroppedFramesToNotify;
  private final boolean deviceNeedsNoPostProcessWorkaround;

  private CodecMaxValues codecMaxValues;
  private boolean codecNeedsSetOutputSurfaceWorkaround;
  private boolean codecHandlesHdr10PlusOutOfBandMetadata;

  @Nullable private Surface surface;
  @Nullable private PlaceholderSurface placeholderSurface;
  private boolean haveReportedFirstFrameRenderedForCurrentSurface;
  private @C.VideoScalingMode int scalingMode;
  private boolean renderedFirstFrameAfterReset;
  private boolean mayRenderFirstFrameAfterEnableIfNotStarted;
  private boolean renderedFirstFrameAfterEnable;
  private long initialPositionUs;
  private long joiningDeadlineMs;
  private long droppedFrameAccumulationStartTimeMs;
  private int droppedFrames;
  private int consecutiveDroppedFrameCount;
  private int buffersInCodecCount;
  private long lastBufferPresentationTimeUs;
  private long lastRenderRealtimeUs;
  private long totalVideoFrameProcessingOffsetUs;
  private int videoFrameProcessingOffsetCount;

  private int currentWidth;
  private int currentHeight;
  private int currentUnappliedRotationDegrees;
  private float currentPixelWidthHeightRatio;
  @Nullable private VideoSize reportedVideoSize;

  private boolean tunneling;
  private int tunnelingAudioSessionId;
  /* package */ @Nullable OnFrameRenderedListenerV23 tunnelingOnFrameRenderedListener;
  @Nullable private VideoFrameMetadataListener frameMetadataListener;

  /**
   * @param context A context.
   * @param mediaCodecSelector A decoder selector.
   */
  public MediaCodecVideoRenderer(Context context, MediaCodecSelector mediaCodecSelector) {
    this(context, mediaCodecSelector, 0);
  }

  /**
   * @param context A context.
   * @param mediaCodecSelector A decoder selector.
   * @param allowedJoiningTimeMs The maximum duration in milliseconds for which this video renderer
   *     can attempt to seamlessly join an ongoing playback.
   */
  public MediaCodecVideoRenderer(
      Context context, MediaCodecSelector mediaCodecSelector, long allowedJoiningTimeMs) {
    this(
        context,
        mediaCodecSelector,
        allowedJoiningTimeMs,
        /* eventHandler= */ null,
        /* eventListener= */ null,
        /* maxDroppedFramesToNotify= */ 0);
  }

  /**
   * @param context A context.
   * @param mediaCodecSelector A decoder selector.
   * @param allowedJoiningTimeMs The maximum duration in milliseconds for which this video renderer
   *     can attempt to seamlessly join an ongoing playback.
   * @param eventHandler A handler to use when delivering events to {@code eventListener}. May be
   *     null if delivery of events is not required.
   * @param eventListener A listener of events. May be null if delivery of events is not required.
   * @param maxDroppedFramesToNotify The maximum number of frames that can be dropped between
   *     invocations of {@link VideoRendererEventListener#onDroppedFrames(int, long)}.
   */
  public MediaCodecVideoRenderer(
      Context context,
      MediaCodecSelector mediaCodecSelector,
      long allowedJoiningTimeMs,
      @Nullable Handler eventHandler,
      @Nullable VideoRendererEventListener eventListener,
      int maxDroppedFramesToNotify) {
    this(
        context,
        MediaCodecAdapter.Factory.DEFAULT,
        mediaCodecSelector,
        allowedJoiningTimeMs,
        /* enableDecoderFallback= */ false,
        eventHandler,
        eventListener,
        maxDroppedFramesToNotify,
        /* assumedMinimumCodecOperatingRate= */ 30);
  }

  /**
   * @param context A context.
   * @param mediaCodecSelector A decoder selector.
   * @param allowedJoiningTimeMs The maximum duration in milliseconds for which this video renderer
   *     can attempt to seamlessly join an ongoing playback.
   * @param enableDecoderFallback Whether to enable fallback to lower-priority decoders if decoder
   *     initialization fails. This may result in using a decoder that is slower/less efficient than
   *     the primary decoder.
   * @param eventHandler A handler to use when delivering events to {@code eventListener}. May be
   *     null if delivery of events is not required.
   * @param eventListener A listener of events. May be null if delivery of events is not required.
   * @param maxDroppedFramesToNotify The maximum number of frames that can be dropped between
   *     invocations of {@link VideoRendererEventListener#onDroppedFrames(int, long)}.
   */
  public MediaCodecVideoRenderer(
      Context context,
      MediaCodecSelector mediaCodecSelector,
      long allowedJoiningTimeMs,
      boolean enableDecoderFallback,
      @Nullable Handler eventHandler,
      @Nullable VideoRendererEventListener eventListener,
      int maxDroppedFramesToNotify) {
    this(
        context,
        MediaCodecAdapter.Factory.DEFAULT,
        mediaCodecSelector,
        allowedJoiningTimeMs,
        enableDecoderFallback,
        eventHandler,
        eventListener,
        maxDroppedFramesToNotify,
        /* assumedMinimumCodecOperatingRate= */ 30);
  }

  /**
   * @param context A context.
   * @param codecAdapterFactory The {@link MediaCodecAdapter.Factory} used to create {@link
   *     MediaCodecAdapter} instances.
   * @param mediaCodecSelector A decoder selector.
   * @param allowedJoiningTimeMs The maximum duration in milliseconds for which this video renderer
   *     can attempt to seamlessly join an ongoing playback.
   * @param enableDecoderFallback Whether to enable fallback to lower-priority decoders if decoder
   *     initialization fails. This may result in using a decoder that is slower/less efficient than
   *     the primary decoder.
   * @param eventHandler A handler to use when delivering events to {@code eventListener}. May be
   *     null if delivery of events is not required.
   * @param eventListener A listener of events. May be null if delivery of events is not required.
   * @param maxDroppedFramesToNotify The maximum number of frames that can be dropped between
   *     invocations of {@link VideoRendererEventListener#onDroppedFrames(int, long)}.
   */
  public MediaCodecVideoRenderer(
      Context context,
      MediaCodecAdapter.Factory codecAdapterFactory,
      MediaCodecSelector mediaCodecSelector,
      long allowedJoiningTimeMs,
      boolean enableDecoderFallback,
      @Nullable Handler eventHandler,
      @Nullable VideoRendererEventListener eventListener,
      int maxDroppedFramesToNotify) {

    this(
        context,
        codecAdapterFactory,
        mediaCodecSelector,
        allowedJoiningTimeMs,
        enableDecoderFallback,
        eventHandler,
        eventListener,
        maxDroppedFramesToNotify,
        /* assumedMinimumCodecOperatingRate= */ 30);
  }

  /**
   * Creates a new instance.
   *
   * @param context A context.
   * @param codecAdapterFactory The {@link MediaCodecAdapter.Factory} used to create {@link
   *     MediaCodecAdapter} instances.
   * @param mediaCodecSelector A decoder selector.
   * @param allowedJoiningTimeMs The maximum duration in milliseconds for which this video renderer
   *     can attempt to seamlessly join an ongoing playback.
   * @param enableDecoderFallback Whether to enable fallback to lower-priority decoders if decoder
   *     initialization fails. This may result in using a decoder that is slower/less efficient than
   *     the primary decoder.
   * @param eventHandler A handler to use when delivering events to {@code eventListener}. May be
   *     null if delivery of events is not required.
   * @param eventListener A listener of events. May be null if delivery of events is not required.
   * @param maxDroppedFramesToNotify The maximum number of frames that can be dropped between
   *     invocations of {@link VideoRendererEventListener#onDroppedFrames(int, long)}.
   * @param assumedMinimumCodecOperatingRate A codec operating rate that all codecs instantiated by
   *     this renderer are assumed to meet implicitly (i.e. without the operating rate being set
   *     explicitly using {@link MediaFormat#KEY_OPERATING_RATE}).
   */
  public MediaCodecVideoRenderer(
      Context context,
      MediaCodecAdapter.Factory codecAdapterFactory,
      MediaCodecSelector mediaCodecSelector,
      long allowedJoiningTimeMs,
      boolean enableDecoderFallback,
      @Nullable Handler eventHandler,
      @Nullable VideoRendererEventListener eventListener,
      int maxDroppedFramesToNotify,
      float assumedMinimumCodecOperatingRate) {
    super(
        C.TRACK_TYPE_VIDEO,
        codecAdapterFactory,
        mediaCodecSelector,
        enableDecoderFallback,
        assumedMinimumCodecOperatingRate);
    this.allowedJoiningTimeMs = allowedJoiningTimeMs;
    this.maxDroppedFramesToNotify = maxDroppedFramesToNotify;
    this.context = context.getApplicationContext();
    frameReleaseHelper = new VideoFrameReleaseHelper(this.context);
    eventDispatcher = new EventDispatcher(eventHandler, eventListener);
    deviceNeedsNoPostProcessWorkaround = deviceNeedsNoPostProcessWorkaround();
    joiningDeadlineMs = C.TIME_UNSET;
    currentWidth = Format.NO_VALUE;
    currentHeight = Format.NO_VALUE;
    currentPixelWidthHeightRatio = Format.NO_VALUE;
    scalingMode = C.VIDEO_SCALING_MODE_DEFAULT;
    tunnelingAudioSessionId = C.AUDIO_SESSION_ID_UNSET;
    clearReportedVideoSize();
  }

  @Override
  public String getName() {
    return TAG;
  }

  @Override
  protected @Capabilities int supportsFormat(MediaCodecSelector mediaCodecSelector, Format format)
      throws DecoderQueryException {
    String mimeType = format.sampleMimeType;
    if (!MimeTypes.isVideo(mimeType)) {
      return RendererCapabilities.create(C.FORMAT_UNSUPPORTED_TYPE);
    }
    @Nullable DrmInitData drmInitData = format.drmInitData;

    boolean requiresSecureDecryption = drmInitData != null;
    List<MediaCodecInfo> decoderInfos =
        getDecoderInfos(
            context,
            mediaCodecSelector,
            format,
            requiresSecureDecryption,
            /* requiresTunnelingDecoder= */ false);
    if (requiresSecureDecryption && decoderInfos.isEmpty()) {

      decoderInfos =
          getDecoderInfos(
              context,
              mediaCodecSelector,
              format,
              /* requiresSecureDecoder= */ false,
              /* requiresTunnelingDecoder= */ false);
    }
    if (decoderInfos.isEmpty()) {
      return RendererCapabilities.create(C.FORMAT_UNSUPPORTED_SUBTYPE);
    }
    if (!supportsFormatDrm(format)) {
      return RendererCapabilities.create(C.FORMAT_UNSUPPORTED_DRM);
    }


    MediaCodecInfo decoderInfo = decoderInfos.get(0);
    boolean isFormatSupported = decoderInfo.isFormatSupported(format);
    boolean isPreferredDecoder = true;
    if (!isFormatSupported) {

      for (int i = 1; i < decoderInfos.size(); i++) {
        MediaCodecInfo otherDecoderInfo = decoderInfos.get(i);
        if (otherDecoderInfo.isFormatSupported(format)) {
          decoderInfo = otherDecoderInfo;
          isFormatSupported = true;
          isPreferredDecoder = false;
          break;
        }
      }
    }
    @C.FormatSupport
    int formatSupport = isFormatSupported ? C.FORMAT_HANDLED : C.FORMAT_EXCEEDS_CAPABILITIES;
    @AdaptiveSupport
    int adaptiveSupport =
        decoderInfo.isSeamlessAdaptationSupported(format)
            ? ADAPTIVE_SEAMLESS
            : ADAPTIVE_NOT_SEAMLESS;
    @HardwareAccelerationSupport
    int hardwareAccelerationSupport =
        decoderInfo.hardwareAccelerated
            ? HARDWARE_ACCELERATION_SUPPORTED
            : HARDWARE_ACCELERATION_NOT_SUPPORTED;
    @DecoderSupport
    int decoderSupport = isPreferredDecoder ? DECODER_SUPPORT_PRIMARY : DECODER_SUPPORT_FALLBACK;

    if (Util.SDK_INT >= 26
        && MimeTypes.VIDEO_DOLBY_VISION.equals(format.sampleMimeType)
        && !Api26.doesDisplaySupportDolbyVision(context)) {
      decoderSupport = DECODER_SUPPORT_FALLBACK_MIMETYPE;
    }

    @TunnelingSupport int tunnelingSupport = TUNNELING_NOT_SUPPORTED;
    if (isFormatSupported) {
      List<MediaCodecInfo> tunnelingDecoderInfos =
          getDecoderInfos(
              context,
              mediaCodecSelector,
              format,
              requiresSecureDecryption,
              /* requiresTunnelingDecoder= */ true);
      if (!tunnelingDecoderInfos.isEmpty()) {
        MediaCodecInfo tunnelingDecoderInfo =
            MediaCodecUtil.getDecoderInfosSortedByFormatSupport(tunnelingDecoderInfos, format)
                .get(0);
        if (tunnelingDecoderInfo.isFormatSupported(format)
            && tunnelingDecoderInfo.isSeamlessAdaptationSupported(format)) {
          tunnelingSupport = TUNNELING_SUPPORTED;
        }
      }
    }

    return RendererCapabilities.create(
        formatSupport,
        adaptiveSupport,
        tunnelingSupport,
        hardwareAccelerationSupport,
        decoderSupport);
  }

  @Override
  protected List<MediaCodecInfo> getDecoderInfos(
      MediaCodecSelector mediaCodecSelector, Format format, boolean requiresSecureDecoder)
      throws DecoderQueryException {
    return MediaCodecUtil.getDecoderInfosSortedByFormatSupport(
        getDecoderInfos(context, mediaCodecSelector, format, requiresSecureDecoder, tunneling),
        format);
  }

  /**
   * Returns a list of decoders that can decode media in the specified format, in the priority order
   * specified by the {@link MediaCodecSelector}. Note that since the {@link MediaCodecSelector}
   * only has access to {@link Format#sampleMimeType}, the list is not ordered to account for
   * whether each decoder supports the details of the format (e.g., taking into account the format's
   * profile, level, resolution and so on). {@link
   * MediaCodecUtil#getDecoderInfosSortedByFormatSupport} can be used to further sort the list into
   * an order where decoders that fully support the format come first.
   *
   * @param mediaCodecSelector The decoder selector.
   * @param format The {@link Format} for which a decoder is required.
   * @param requiresSecureDecoder Whether a secure decoder is required.
   * @param requiresTunnelingDecoder Whether a tunneling decoder is required.
   * @return A list of {@link MediaCodecInfo}s corresponding to decoders. May be empty.
   * @throws DecoderQueryException Thrown if there was an error querying decoders.
   */
  private static List<MediaCodecInfo> getDecoderInfos(
      Context context,
      MediaCodecSelector mediaCodecSelector,
      Format format,
      boolean requiresSecureDecoder,
      boolean requiresTunnelingDecoder)
      throws DecoderQueryException {
    @Nullable String mimeType = format.sampleMimeType;
    if (mimeType == null) {
      return ImmutableList.of();
    }
    List<MediaCodecInfo> decoderInfos =
        mediaCodecSelector.getDecoderInfos(
            mimeType, requiresSecureDecoder, requiresTunnelingDecoder);
    @Nullable String alternativeMimeType = MediaCodecUtil.getAlternativeCodecMimeType(format);
    if (alternativeMimeType == null) {
      return ImmutableList.copyOf(decoderInfos);
    }
    List<MediaCodecInfo> alternativeDecoderInfos =
        mediaCodecSelector.getDecoderInfos(
            alternativeMimeType, requiresSecureDecoder, requiresTunnelingDecoder);
    if (Util.SDK_INT >= 26
        && MimeTypes.VIDEO_DOLBY_VISION.equals(format.sampleMimeType)
        && !alternativeDecoderInfos.isEmpty()
        && !Api26.doesDisplaySupportDolbyVision(context)) {
      return ImmutableList.copyOf(alternativeDecoderInfos);
    }
    return ImmutableList.<MediaCodecInfo>builder()
        .addAll(decoderInfos)
        .addAll(alternativeDecoderInfos)
        .build();
  }

  @RequiresApi(26)
  private static final class Api26 {
    @DoNotInline
    public static boolean doesDisplaySupportDolbyVision(Context context) {
      boolean supportsDolbyVision = false;
      DisplayManager displayManager =
          (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
      Display display =
          (displayManager != null) ? displayManager.getDisplay(DEFAULT_DISPLAY) : null;
      if (display != null && display.isHdr()) {
        int[] supportedHdrTypes = display.getHdrCapabilities().getSupportedHdrTypes();
        for (int hdrType : supportedHdrTypes) {
          if (hdrType == Display.HdrCapabilities.HDR_TYPE_DOLBY_VISION) {
            supportsDolbyVision = true;
            break;
          }
        }
      }
      return supportsDolbyVision;
    }
  }

  @Override
  protected void onEnabled(boolean joining, boolean mayRenderStartOfStream)
      throws ExoPlaybackException {
    super.onEnabled(joining, mayRenderStartOfStream);
    boolean tunneling = getConfiguration().tunneling;
    checkState(!tunneling || tunnelingAudioSessionId != C.AUDIO_SESSION_ID_UNSET);
    if (this.tunneling != tunneling) {
      this.tunneling = tunneling;
      releaseCodec();
    }
    eventDispatcher.enabled(decoderCounters);
    mayRenderFirstFrameAfterEnableIfNotStarted = mayRenderStartOfStream;
    renderedFirstFrameAfterEnable = false;
  }

  @Override
  protected void onPositionReset(long positionUs, boolean joining) throws ExoPlaybackException {
    super.onPositionReset(positionUs, joining);
    clearRenderedFirstFrame();
    frameReleaseHelper.onPositionReset();
    lastBufferPresentationTimeUs = C.TIME_UNSET;
    initialPositionUs = C.TIME_UNSET;
    consecutiveDroppedFrameCount = 0;
    if (joining) {
      setJoiningDeadlineMs();
    } else {
      joiningDeadlineMs = C.TIME_UNSET;
    }
  }

  @Override
  public boolean isReady() {
    if (super.isReady()
        && (renderedFirstFrameAfterReset
            || (placeholderSurface != null && surface == placeholderSurface)
            || getCodec() == null
            || tunneling)) {

      joiningDeadlineMs = C.TIME_UNSET;
      return true;
    } else if (joiningDeadlineMs == C.TIME_UNSET) {

      return false;
    } else if (SystemClock.elapsedRealtime() < joiningDeadlineMs) {

      return true;
    } else {

      joiningDeadlineMs = C.TIME_UNSET;
      return false;
    }
  }

  @Override
  protected void onStarted() {
    super.onStarted();
    droppedFrames = 0;
    droppedFrameAccumulationStartTimeMs = SystemClock.elapsedRealtime();
    lastRenderRealtimeUs = SystemClock.elapsedRealtime() * 1000;
    totalVideoFrameProcessingOffsetUs = 0;
    videoFrameProcessingOffsetCount = 0;
    frameReleaseHelper.onStarted();
  }

  @Override
  protected void onStopped() {
    joiningDeadlineMs = C.TIME_UNSET;
    maybeNotifyDroppedFrames();
    maybeNotifyVideoFrameProcessingOffset();
    frameReleaseHelper.onStopped();
    super.onStopped();
  }

  @Override
  protected void onDisabled() {
    clearReportedVideoSize();
    clearRenderedFirstFrame();
    haveReportedFirstFrameRenderedForCurrentSurface = false;
    tunnelingOnFrameRenderedListener = null;
    try {
      super.onDisabled();
    } finally {
      eventDispatcher.disabled(decoderCounters);
    }
  }

  @TargetApi(17) // Needed for placeholderSurface usage, as it is always null on API level 16.
  @Override
  protected void onReset() {
    try {
      super.onReset();
    } finally {
      if (placeholderSurface != null) {
        releasePlaceholderSurface();
      }
    }
  }

  @Override
  public void handleMessage(@MessageType int messageType, @Nullable Object message)
      throws ExoPlaybackException {
    switch (messageType) {
      case MSG_SET_VIDEO_OUTPUT:
        setOutput(message);
        break;
      case MSG_SET_SCALING_MODE:
        scalingMode = (Integer) message;
        @Nullable MediaCodecAdapter codec = getCodec();
        if (codec != null) {
          codec.setVideoScalingMode(scalingMode);
        }
        break;
      case MSG_SET_CHANGE_FRAME_RATE_STRATEGY:
        frameReleaseHelper.setChangeFrameRateStrategy((int) message);
        break;
      case MSG_SET_VIDEO_FRAME_METADATA_LISTENER:
        frameMetadataListener = (VideoFrameMetadataListener) message;
        break;
      case MSG_SET_AUDIO_SESSION_ID:
        int tunnelingAudioSessionId = (int) message;
        if (this.tunnelingAudioSessionId != tunnelingAudioSessionId) {
          this.tunnelingAudioSessionId = tunnelingAudioSessionId;
          if (tunneling) {
            releaseCodec();
          }
        }
        break;
      case MSG_SET_AUDIO_ATTRIBUTES:
      case MSG_SET_AUX_EFFECT_INFO:
      case MSG_SET_CAMERA_MOTION_LISTENER:
      case MSG_SET_SKIP_SILENCE_ENABLED:
      case MSG_SET_VOLUME:
      case MSG_SET_WAKEUP_LISTENER:
      default:
        super.handleMessage(messageType, message);
    }
  }

  private void setOutput(@Nullable Object output) throws ExoPlaybackException {

    @Nullable Surface surface = output instanceof Surface ? (Surface) output : null;

    if (surface == null) {

      if (placeholderSurface != null) {
        surface = placeholderSurface;
      } else {
        MediaCodecInfo codecInfo = getCodecInfo();
        if (codecInfo != null && shouldUsePlaceholderSurface(codecInfo)) {
          placeholderSurface = PlaceholderSurface.newInstanceV17(context, codecInfo.secure);
          surface = placeholderSurface;
        }
      }
    }

    if (this.surface != surface) {
      this.surface = surface;
      frameReleaseHelper.onSurfaceChanged(surface);
      haveReportedFirstFrameRenderedForCurrentSurface = false;

      @State int state = getState();
      @Nullable MediaCodecAdapter codec = getCodec();
      if (codec != null) {
        if (Util.SDK_INT >= 23 && surface != null && !codecNeedsSetOutputSurfaceWorkaround) {
          try {
            setOutputSurfaceV23(codec, surface);
          } catch (Throwable e) {
            throw new SurfaceNotValidException(e);
          }
        } else {
          releaseCodec();
          maybeInitCodecOrBypass();
        }
      }
      if (surface != null && surface != placeholderSurface) {

        maybeRenotifyVideoSizeChanged();

        clearRenderedFirstFrame();
        if (state == STATE_STARTED) {
          setJoiningDeadlineMs();
        }
      } else {

        clearReportedVideoSize();
        clearRenderedFirstFrame();
      }
    } else if (surface != null && surface != placeholderSurface) {


      maybeRenotifyVideoSizeChanged();
      maybeRenotifyRenderedFirstFrame();
    }
  }

  @Override
  protected boolean shouldInitCodec(MediaCodecInfo codecInfo) {
    return surface != null || shouldUsePlaceholderSurface(codecInfo);
  }

  @Override
  protected boolean getCodecNeedsEosPropagation() {

    return tunneling && Util.SDK_INT < 23;
  }

  @TargetApi(17) // Needed for placeHolderSurface usage, as it is always null on API level 16.
  @Override
  protected MediaCodecAdapter.Configuration getMediaCodecConfiguration(
      MediaCodecInfo codecInfo,
      Format format,
      @Nullable MediaCrypto crypto,
      float codecOperatingRate) {
    if (placeholderSurface != null && placeholderSurface.secure != codecInfo.secure) {

      releasePlaceholderSurface();
    }
    String codecMimeType = codecInfo.codecMimeType;
    codecMaxValues = getCodecMaxValues(codecInfo, format, getStreamFormats());
    MediaFormat mediaFormat =
        getMediaFormat(
            format,
            codecMimeType,
            codecMaxValues,
            codecOperatingRate,
            deviceNeedsNoPostProcessWorkaround,
            tunneling ? tunnelingAudioSessionId : C.AUDIO_SESSION_ID_UNSET);
    if (surface == null) {
      if (!shouldUsePlaceholderSurface(codecInfo)) {
        throw new IllegalStateException();
      }
      if (placeholderSurface == null) {
        placeholderSurface = PlaceholderSurface.newInstanceV17(context, codecInfo.secure);
      }
      surface = placeholderSurface;
    }
    return MediaCodecAdapter.Configuration.createForVideoDecoding(
        codecInfo, mediaFormat, format, surface, crypto);
  }

  @Override
  protected DecoderReuseEvaluation canReuseCodec(
      MediaCodecInfo codecInfo, Format oldFormat, Format newFormat) {
    DecoderReuseEvaluation evaluation = codecInfo.canReuseCodec(oldFormat, newFormat);

    @DecoderDiscardReasons int discardReasons = evaluation.discardReasons;
    if (newFormat.width > codecMaxValues.width || newFormat.height > codecMaxValues.height) {
      discardReasons |= DISCARD_REASON_VIDEO_MAX_RESOLUTION_EXCEEDED;
    }
    if (getMaxInputSize(codecInfo, newFormat) > codecMaxValues.inputSize) {
      discardReasons |= DISCARD_REASON_MAX_INPUT_SIZE_EXCEEDED;
    }

    return new DecoderReuseEvaluation(
        codecInfo.name,
        oldFormat,
        newFormat,
        discardReasons != 0 ? REUSE_RESULT_NO : evaluation.result,
        discardReasons);
  }

  @CallSuper
  @Override
  protected void resetCodecStateForFlush() {
    super.resetCodecStateForFlush();
    buffersInCodecCount = 0;
  }

  @Override
  public void setPlaybackSpeed(float currentPlaybackSpeed, float targetPlaybackSpeed)
      throws ExoPlaybackException {
    super.setPlaybackSpeed(currentPlaybackSpeed, targetPlaybackSpeed);
    frameReleaseHelper.onPlaybackSpeed(currentPlaybackSpeed);
  }

  /**
   * Returns a maximum input size for a given codec and format.
   *
   * @param codecInfo Information about the {@link MediaCodec} being configured.
   * @param format The format.
   * @return A maximum input size in bytes, or {@link Format#NO_VALUE} if a maximum could not be
   *     determined.
   */
  public static int getCodecMaxInputSize(MediaCodecInfo codecInfo, Format format) {
    int width = format.width;
    int height = format.height;
    if (width == Format.NO_VALUE || height == Format.NO_VALUE) {

      return Format.NO_VALUE;
    }

    String sampleMimeType = format.sampleMimeType;
    if (MimeTypes.VIDEO_DOLBY_VISION.equals(sampleMimeType)) {





      sampleMimeType = MimeTypes.VIDEO_H265;
      @Nullable
      Pair<Integer, Integer> codecProfileAndLevel = MediaCodecUtil.getCodecProfileAndLevel(format);
      if (codecProfileAndLevel != null) {
        int profile = codecProfileAndLevel.first;
        if (profile == CodecProfileLevel.DolbyVisionProfileDvavSe
            || profile == CodecProfileLevel.DolbyVisionProfileDvavPer
            || profile == CodecProfileLevel.DolbyVisionProfileDvavPen) {
          sampleMimeType = MimeTypes.VIDEO_H264;
        }
      }
    }

    switch (sampleMimeType) {
      case MimeTypes.VIDEO_H263:
      case MimeTypes.VIDEO_MP4V:
      case MimeTypes.VIDEO_AV1:

      case MimeTypes.VIDEO_VP8:

        return getMaxSampleSize(/* pixelCount= */ width * height, /* minCompressionRatio= */ 2);
      case MimeTypes.VIDEO_H265:


        return max(
            HEVC_MAX_INPUT_SIZE_THRESHOLD,
            getMaxSampleSize(/* pixelCount= */ width * height, /* minCompressionRatio= */ 2));
      case MimeTypes.VIDEO_H264:
        if ("BRAVIA 4K 2015".equals(Util.MODEL) // Sony Bravia 4K
            || ("Amazon".equals(Util.MANUFACTURER)
                && ("KFSOWI".equals(Util.MODEL) // Kindle Soho
                    || ("AFTS".equals(Util.MODEL) && codecInfo.secure)))) { // Fire TV Gen 2


          return Format.NO_VALUE;
        }

        int maxPixels = Util.ceilDivide(width, 16) * Util.ceilDivide(height, 16) * 16 * 16;
        return getMaxSampleSize(maxPixels, /* minCompressionRatio= */ 2);
      case MimeTypes.VIDEO_VP9:
        return getMaxSampleSize(/* pixelCount= */ width * height, /* minCompressionRatio= */ 4);
      default:

        return Format.NO_VALUE;
    }
  }

  @Override
  protected float getCodecOperatingRateV23(
      float targetPlaybackSpeed, Format format, Format[] streamFormats) {


    float maxFrameRate = -1;
    for (Format streamFormat : streamFormats) {
      float streamFrameRate = streamFormat.frameRate;
      if (streamFrameRate != Format.NO_VALUE) {
        maxFrameRate = max(maxFrameRate, streamFrameRate);
      }
    }
    return maxFrameRate == -1 ? CODEC_OPERATING_RATE_UNSET : (maxFrameRate * targetPlaybackSpeed);
  }

  @Override
  protected void onCodecInitialized(
      String name,
      MediaCodecAdapter.Configuration configuration,
      long initializedTimestampMs,
      long initializationDurationMs) {
    eventDispatcher.decoderInitialized(name, initializedTimestampMs, initializationDurationMs);
    codecNeedsSetOutputSurfaceWorkaround = codecNeedsSetOutputSurfaceWorkaround(name);
    codecHandlesHdr10PlusOutOfBandMetadata =
        checkNotNull(getCodecInfo()).isHdr10PlusOutOfBandMetadataSupported();
    if (Util.SDK_INT >= 23 && tunneling) {
      tunnelingOnFrameRenderedListener = new OnFrameRenderedListenerV23(checkNotNull(getCodec()));
    }
  }

  @Override
  protected void onCodecReleased(String name) {
    eventDispatcher.decoderReleased(name);
  }

  @Override
  protected void onCodecError(Exception codecError) {
    Log.e(TAG, "Video codec error", codecError);
    eventDispatcher.videoCodecError(codecError);
  }

  @Override
  @Nullable
  protected DecoderReuseEvaluation onInputFormatChanged(FormatHolder formatHolder)
      throws ExoPlaybackException {
    @Nullable DecoderReuseEvaluation evaluation = super.onInputFormatChanged(formatHolder);
    eventDispatcher.inputFormatChanged(formatHolder.format, evaluation);
    return evaluation;
  }

  /**
   * Called immediately before an input buffer is queued into the codec.
   *
   * <p>In tunneling mode for pre Marshmallow, the buffer is treated as if immediately output.
   *
   * @param buffer The buffer to be queued.
   * @throws ExoPlaybackException Thrown if an error occurs handling the input buffer.
   */
  @CallSuper
  @Override
  protected void onQueueInputBuffer(DecoderInputBuffer buffer) throws ExoPlaybackException {


    if (!tunneling) {
      buffersInCodecCount++;
    }
    if (Util.SDK_INT < 23 && tunneling) {


      onProcessedTunneledBuffer(buffer.timeUs);
    }
  }

  @Override
  protected void onOutputFormatChanged(Format format, @Nullable MediaFormat mediaFormat) {
    @Nullable MediaCodecAdapter codec = getCodec();
    if (codec != null) {

      codec.setVideoScalingMode(scalingMode);
    }
    if (tunneling) {
      currentWidth = format.width;
      currentHeight = format.height;
    } else {
      checkNotNull(mediaFormat);
      boolean hasCrop =
          mediaFormat.containsKey(KEY_CROP_RIGHT)
              && mediaFormat.containsKey(KEY_CROP_LEFT)
              && mediaFormat.containsKey(KEY_CROP_BOTTOM)
              && mediaFormat.containsKey(KEY_CROP_TOP);
      currentWidth =
          hasCrop
              ? mediaFormat.getInteger(KEY_CROP_RIGHT) - mediaFormat.getInteger(KEY_CROP_LEFT) + 1
              : mediaFormat.getInteger(MediaFormat.KEY_WIDTH);
      currentHeight =
          hasCrop
              ? mediaFormat.getInteger(KEY_CROP_BOTTOM) - mediaFormat.getInteger(KEY_CROP_TOP) + 1
              : mediaFormat.getInteger(MediaFormat.KEY_HEIGHT);
    }
    currentPixelWidthHeightRatio = format.pixelWidthHeightRatio;
    if (Util.SDK_INT >= 21) {



      if (format.rotationDegrees == 90 || format.rotationDegrees == 270) {
        int rotatedHeight = currentWidth;
        currentWidth = currentHeight;
        currentHeight = rotatedHeight;
        currentPixelWidthHeightRatio = 1 / currentPixelWidthHeightRatio;
      }
    } else {

      currentUnappliedRotationDegrees = format.rotationDegrees;
    }
    frameReleaseHelper.onFormatChanged(format.frameRate);
  }

  @Override
  @TargetApi(29) // codecHandlesHdr10PlusOutOfBandMetadata is false if Util.SDK_INT < 29
  protected void handleInputBufferSupplementalData(DecoderInputBuffer buffer)
      throws ExoPlaybackException {
    if (!codecHandlesHdr10PlusOutOfBandMetadata) {
      return;
    }
    ByteBuffer data = checkNotNull(buffer.supplementalData);
    if (data.remaining() >= 7) {

      byte ituTT35CountryCode = data.get();
      int ituTT35TerminalProviderCode = data.getShort();
      int ituTT35TerminalProviderOrientedCode = data.getShort();
      byte applicationIdentifier = data.get();
      byte applicationVersion = data.get();
      data.position(0);
      if (ituTT35CountryCode == (byte) 0xB5
          && ituTT35TerminalProviderCode == 0x003C
          && ituTT35TerminalProviderOrientedCode == 0x0001
          && applicationIdentifier == 4
          && (applicationVersion == 0 || applicationVersion == 1)) {


        byte[] hdr10PlusInfo = new byte[data.remaining()];
        data.get(hdr10PlusInfo);
        data.position(0);
        setHdr10PlusInfoV29(getCodec(), hdr10PlusInfo);
      }
    }
  }

  @Override
  protected boolean processOutputBuffer(
      long positionUs,
      long elapsedRealtimeUs,
      @Nullable MediaCodecAdapter codec,
      @Nullable ByteBuffer buffer,
      int bufferIndex,
      int bufferFlags,
      int sampleCount,
      long bufferPresentationTimeUs,
      boolean isDecodeOnlyBuffer,
      boolean isLastBuffer,
      Format format)
      throws ExoPlaybackException {
    checkNotNull(codec); // Can not render video without codec

    if (initialPositionUs == C.TIME_UNSET) {
      initialPositionUs = positionUs;
    }

    if (bufferPresentationTimeUs != lastBufferPresentationTimeUs) {
      frameReleaseHelper.onNextFrame(bufferPresentationTimeUs);
      this.lastBufferPresentationTimeUs = bufferPresentationTimeUs;
    }

    long outputStreamOffsetUs = getOutputStreamOffsetUs();
    long presentationTimeUs = bufferPresentationTimeUs - outputStreamOffsetUs;

    if (isDecodeOnlyBuffer && !isLastBuffer) {
      skipOutputBuffer(codec, bufferIndex, presentationTimeUs);
      return true;
    }

    double playbackSpeed = getPlaybackSpeed();
    boolean isStarted = getState() == STATE_STARTED;
    long elapsedRealtimeNowUs = SystemClock.elapsedRealtime() * 1000;



    long earlyUs = (long) ((bufferPresentationTimeUs - positionUs) / playbackSpeed);
    if (isStarted) {

      earlyUs -= elapsedRealtimeNowUs - elapsedRealtimeUs;
    }

    if (surface == placeholderSurface) {

      if (isBufferLate(earlyUs)) {
        skipOutputBuffer(codec, bufferIndex, presentationTimeUs);
        updateVideoFrameProcessingOffsetCounters(earlyUs);
        return true;
      }
      return false;
    }

    long elapsedSinceLastRenderUs = elapsedRealtimeNowUs - lastRenderRealtimeUs;
    boolean shouldRenderFirstFrame =
        !renderedFirstFrameAfterEnable
            ? (isStarted || mayRenderFirstFrameAfterEnableIfNotStarted)
            : !renderedFirstFrameAfterReset;

    boolean forceRenderOutputBuffer =
        joiningDeadlineMs == C.TIME_UNSET
            && positionUs >= outputStreamOffsetUs
            && (shouldRenderFirstFrame
                || (isStarted && shouldForceRenderOutputBuffer(earlyUs, elapsedSinceLastRenderUs)));
    if (forceRenderOutputBuffer) {
      long releaseTimeNs = System.nanoTime();
      notifyFrameMetadataListener(presentationTimeUs, releaseTimeNs, format);
      if (Util.SDK_INT >= 21) {
        renderOutputBufferV21(codec, bufferIndex, presentationTimeUs, releaseTimeNs);
      } else {
        renderOutputBuffer(codec, bufferIndex, presentationTimeUs);
      }
      updateVideoFrameProcessingOffsetCounters(earlyUs);
      return true;
    }

    if (!isStarted || positionUs == initialPositionUs) {
      return false;
    }

    long systemTimeNs = System.nanoTime();
    long unadjustedFrameReleaseTimeNs = systemTimeNs + (earlyUs * 1000);

    long adjustedReleaseTimeNs = frameReleaseHelper.adjustReleaseTime(unadjustedFrameReleaseTimeNs);
    earlyUs = (adjustedReleaseTimeNs - systemTimeNs) / 1000;

    boolean treatDroppedBuffersAsSkipped = joiningDeadlineMs != C.TIME_UNSET;
    if (shouldDropBuffersToKeyframe(earlyUs, elapsedRealtimeUs, isLastBuffer)
        && maybeDropBuffersToKeyframe(positionUs, treatDroppedBuffersAsSkipped)) {
      return false;
    } else if (shouldDropOutputBuffer(earlyUs, elapsedRealtimeUs, isLastBuffer)) {
      if (treatDroppedBuffersAsSkipped) {
        skipOutputBuffer(codec, bufferIndex, presentationTimeUs);
      } else {
        dropOutputBuffer(codec, bufferIndex, presentationTimeUs);
      }
      updateVideoFrameProcessingOffsetCounters(earlyUs);
      return true;
    }

    if (Util.SDK_INT >= 21) {

      if (earlyUs < 50000) {
        notifyFrameMetadataListener(presentationTimeUs, adjustedReleaseTimeNs, format);
        renderOutputBufferV21(codec, bufferIndex, presentationTimeUs, adjustedReleaseTimeNs);
        updateVideoFrameProcessingOffsetCounters(earlyUs);
        return true;
      }
    } else {

      if (earlyUs < 30000) {
        if (earlyUs > 11000) {


          try {

            Thread.sleep((earlyUs - 10000) / 1000);
          } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return false;
          }
        }
        notifyFrameMetadataListener(presentationTimeUs, adjustedReleaseTimeNs, format);
        renderOutputBuffer(codec, bufferIndex, presentationTimeUs);
        updateVideoFrameProcessingOffsetCounters(earlyUs);
        return true;
      }
    }

    return false;
  }

  private void notifyFrameMetadataListener(
      long presentationTimeUs, long releaseTimeNs, Format format) {
    if (frameMetadataListener != null) {
      frameMetadataListener.onVideoFrameAboutToBeRendered(
          presentationTimeUs, releaseTimeNs, format, getCodecOutputMediaFormat());
    }
  }

  /** Called when a buffer was processed in tunneling mode. */
  protected void onProcessedTunneledBuffer(long presentationTimeUs) throws ExoPlaybackException {
    updateOutputFormatForTime(presentationTimeUs);
    maybeNotifyVideoSizeChanged();
    decoderCounters.renderedOutputBufferCount++;
    maybeNotifyRenderedFirstFrame();
    onProcessedOutputBuffer(presentationTimeUs);
  }

  /** Called when a output EOS was received in tunneling mode. */
  private void onProcessedTunneledEndOfStream() {
    setPendingOutputEndOfStream();
  }

  @CallSuper
  @Override
  protected void onProcessedOutputBuffer(long presentationTimeUs) {
    super.onProcessedOutputBuffer(presentationTimeUs);
    if (!tunneling) {
      buffersInCodecCount--;
    }
  }

  @Override
  protected void onProcessedStreamChange() {
    super.onProcessedStreamChange();
    clearRenderedFirstFrame();
  }

  /**
   * Returns whether the buffer being processed should be dropped.
   *
   * @param earlyUs The time until the buffer should be presented in microseconds. A negative value
   *     indicates that the buffer is late.
   * @param elapsedRealtimeUs {@link android.os.SystemClock#elapsedRealtime()} in microseconds,
   *     measured at the start of the current iteration of the rendering loop.
   * @param isLastBuffer Whether the buffer is the last buffer in the current stream.
   */
  protected boolean shouldDropOutputBuffer(
      long earlyUs, long elapsedRealtimeUs, boolean isLastBuffer) {
    return isBufferLate(earlyUs) && !isLastBuffer;
  }

  /**
   * Returns whether to drop all buffers from the buffer being processed to the keyframe at or after
   * the current playback position, if possible.
   *
   * @param earlyUs The time until the current buffer should be presented in microseconds. A
   *     negative value indicates that the buffer is late.
   * @param elapsedRealtimeUs {@link android.os.SystemClock#elapsedRealtime()} in microseconds,
   *     measured at the start of the current iteration of the rendering loop.
   * @param isLastBuffer Whether the buffer is the last buffer in the current stream.
   */
  protected boolean shouldDropBuffersToKeyframe(
      long earlyUs, long elapsedRealtimeUs, boolean isLastBuffer) {
    return isBufferVeryLate(earlyUs) && !isLastBuffer;
  }

  /**
   * Returns whether to force rendering an output buffer.
   *
   * @param earlyUs The time until the current buffer should be presented in microseconds. A
   *     negative value indicates that the buffer is late.
   * @param elapsedSinceLastRenderUs The elapsed time since the last output buffer was rendered, in
   *     microseconds.
   * @return Returns whether to force rendering an output buffer.
   */
  protected boolean shouldForceRenderOutputBuffer(long earlyUs, long elapsedSinceLastRenderUs) {

    return isBufferLate(earlyUs) && elapsedSinceLastRenderUs > 100000;
  }

  /**
   * Skips the output buffer with the specified index.
   *
   * @param codec The codec that owns the output buffer.
   * @param index The index of the output buffer to skip.
   * @param presentationTimeUs The presentation time of the output buffer, in microseconds.
   */
  protected void skipOutputBuffer(MediaCodecAdapter codec, int index, long presentationTimeUs) {
    TraceUtil.beginSection("skipVideoBuffer");
    codec.releaseOutputBuffer(index, false);
    TraceUtil.endSection();
    decoderCounters.skippedOutputBufferCount++;
  }

  /**
   * Drops the output buffer with the specified index.
   *
   * @param codec The codec that owns the output buffer.
   * @param index The index of the output buffer to drop.
   * @param presentationTimeUs The presentation time of the output buffer, in microseconds.
   */
  protected void dropOutputBuffer(MediaCodecAdapter codec, int index, long presentationTimeUs) {
    TraceUtil.beginSection("dropVideoBuffer");
    codec.releaseOutputBuffer(index, false);
    TraceUtil.endSection();
    updateDroppedBufferCounters(
        /* droppedInputBufferCount= */ 0, /* droppedDecoderBufferCount= */ 1);
  }

  /**
   * Drops frames from the current output buffer to the next keyframe at or before the playback
   * position. If no such keyframe exists, as the playback position is inside the same group of
   * pictures as the buffer being processed, returns {@code false}. Returns {@code true} otherwise.
   *
   * @param positionUs The current playback position, in microseconds.
   * @param treatDroppedBuffersAsSkipped Whether dropped buffers should be treated as intentionally
   *     skipped.
   * @return Whether any buffers were dropped.
   * @throws ExoPlaybackException If an error occurs flushing the codec.
   */
  protected boolean maybeDropBuffersToKeyframe(
      long positionUs, boolean treatDroppedBuffersAsSkipped) throws ExoPlaybackException {
    int droppedSourceBufferCount = skipSource(positionUs);
    if (droppedSourceBufferCount == 0) {
      return false;
    }


    if (treatDroppedBuffersAsSkipped) {
      decoderCounters.skippedInputBufferCount += droppedSourceBufferCount;
      decoderCounters.skippedOutputBufferCount += buffersInCodecCount;
    } else {
      decoderCounters.droppedToKeyframeCount++;
      updateDroppedBufferCounters(
          droppedSourceBufferCount, /* droppedDecoderBufferCount= */ buffersInCodecCount);
    }
    flushOrReinitializeCodec();
    return true;
  }

  /**
   * Updates local counters and {@link #decoderCounters} to reflect that buffers were dropped.
   *
   * @param droppedInputBufferCount The number of buffers dropped from the source before being
   *     passed to the decoder.
   * @param droppedDecoderBufferCount The number of buffers dropped after being passed to the
   *     decoder.
   */
  protected void updateDroppedBufferCounters(
      int droppedInputBufferCount, int droppedDecoderBufferCount) {
    decoderCounters.droppedInputBufferCount += droppedInputBufferCount;
    int totalDroppedBufferCount = droppedInputBufferCount + droppedDecoderBufferCount;
    decoderCounters.droppedBufferCount += totalDroppedBufferCount;
    droppedFrames += totalDroppedBufferCount;
    consecutiveDroppedFrameCount += totalDroppedBufferCount;
    decoderCounters.maxConsecutiveDroppedBufferCount =
        max(consecutiveDroppedFrameCount, decoderCounters.maxConsecutiveDroppedBufferCount);
    if (maxDroppedFramesToNotify > 0 && droppedFrames >= maxDroppedFramesToNotify) {
      maybeNotifyDroppedFrames();
    }
  }

  /**
   * Updates local counters and {@link DecoderCounters} with a new video frame processing offset.
   *
   * @param processingOffsetUs The video frame processing offset.
   */
  protected void updateVideoFrameProcessingOffsetCounters(long processingOffsetUs) {
    decoderCounters.addVideoFrameProcessingOffset(processingOffsetUs);
    totalVideoFrameProcessingOffsetUs += processingOffsetUs;
    videoFrameProcessingOffsetCount++;
  }

  /**
   * Renders the output buffer with the specified index. This method is only called if the platform
   * API version of the device is less than 21.
   *
   * @param codec The codec that owns the output buffer.
   * @param index The index of the output buffer to drop.
   * @param presentationTimeUs The presentation time of the output buffer, in microseconds.
   */
  protected void renderOutputBuffer(MediaCodecAdapter codec, int index, long presentationTimeUs) {
    maybeNotifyVideoSizeChanged();
    TraceUtil.beginSection("releaseOutputBuffer");
    codec.releaseOutputBuffer(index, true);
    TraceUtil.endSection();
    lastRenderRealtimeUs = SystemClock.elapsedRealtime() * 1000;
    decoderCounters.renderedOutputBufferCount++;
    consecutiveDroppedFrameCount = 0;
    maybeNotifyRenderedFirstFrame();
  }

  /**
   * Renders the output buffer with the specified index. This method is only called if the platform
   * API version of the device is 21 or later.
   *
   * @param codec The codec that owns the output buffer.
   * @param index The index of the output buffer to drop.
   * @param presentationTimeUs The presentation time of the output buffer, in microseconds.
   * @param releaseTimeNs The wallclock time at which the frame should be displayed, in nanoseconds.
   */
  @RequiresApi(21)
  protected void renderOutputBufferV21(
      MediaCodecAdapter codec, int index, long presentationTimeUs, long releaseTimeNs) {
    maybeNotifyVideoSizeChanged();
    TraceUtil.beginSection("releaseOutputBuffer");
    codec.releaseOutputBuffer(index, releaseTimeNs);
    TraceUtil.endSection();
    lastRenderRealtimeUs = SystemClock.elapsedRealtime() * 1000;
    decoderCounters.renderedOutputBufferCount++;
    consecutiveDroppedFrameCount = 0;
    maybeNotifyRenderedFirstFrame();
  }

  private boolean shouldUsePlaceholderSurface(MediaCodecInfo codecInfo) {
    return Util.SDK_INT >= 23
        && !tunneling
        && !codecNeedsSetOutputSurfaceWorkaround(codecInfo.name)
        && (!codecInfo.secure || PlaceholderSurface.isSecureSupported(context));
  }

  @RequiresApi(17)
  private void releasePlaceholderSurface() {
    if (surface == placeholderSurface) {
      surface = null;
    }
    placeholderSurface.release();
    placeholderSurface = null;
  }

  private void setJoiningDeadlineMs() {
    joiningDeadlineMs =
        allowedJoiningTimeMs > 0
            ? (SystemClock.elapsedRealtime() + allowedJoiningTimeMs)
            : C.TIME_UNSET;
  }

  private void clearRenderedFirstFrame() {
    renderedFirstFrameAfterReset = false;




    if (Util.SDK_INT >= 23 && tunneling) {
      @Nullable MediaCodecAdapter codec = getCodec();

      if (codec != null) {
        tunnelingOnFrameRenderedListener = new OnFrameRenderedListenerV23(codec);
      }
    }
  }

  /* package */ void maybeNotifyRenderedFirstFrame() {
    renderedFirstFrameAfterEnable = true;
    if (!renderedFirstFrameAfterReset) {
      renderedFirstFrameAfterReset = true;
      eventDispatcher.renderedFirstFrame(surface);
      haveReportedFirstFrameRenderedForCurrentSurface = true;
    }
  }

  private void maybeRenotifyRenderedFirstFrame() {
    if (haveReportedFirstFrameRenderedForCurrentSurface) {
      eventDispatcher.renderedFirstFrame(surface);
    }
  }

  private void clearReportedVideoSize() {
    reportedVideoSize = null;
  }

  private void maybeNotifyVideoSizeChanged() {
    if ((currentWidth != Format.NO_VALUE || currentHeight != Format.NO_VALUE)
        && (reportedVideoSize == null
            || reportedVideoSize.width != currentWidth
            || reportedVideoSize.height != currentHeight
            || reportedVideoSize.unappliedRotationDegrees != currentUnappliedRotationDegrees
            || reportedVideoSize.pixelWidthHeightRatio != currentPixelWidthHeightRatio)) {
      reportedVideoSize =
          new VideoSize(
              currentWidth,
              currentHeight,
              currentUnappliedRotationDegrees,
              currentPixelWidthHeightRatio);
      eventDispatcher.videoSizeChanged(reportedVideoSize);
    }
  }

  private void maybeRenotifyVideoSizeChanged() {
    if (reportedVideoSize != null) {
      eventDispatcher.videoSizeChanged(reportedVideoSize);
    }
  }

  private void maybeNotifyDroppedFrames() {
    if (droppedFrames > 0) {
      long now = SystemClock.elapsedRealtime();
      long elapsedMs = now - droppedFrameAccumulationStartTimeMs;
      eventDispatcher.droppedFrames(droppedFrames, elapsedMs);
      droppedFrames = 0;
      droppedFrameAccumulationStartTimeMs = now;
    }
  }

  private void maybeNotifyVideoFrameProcessingOffset() {
    if (videoFrameProcessingOffsetCount != 0) {
      eventDispatcher.reportVideoFrameProcessingOffset(
          totalVideoFrameProcessingOffsetUs, videoFrameProcessingOffsetCount);
      totalVideoFrameProcessingOffsetUs = 0;
      videoFrameProcessingOffsetCount = 0;
    }
  }

  private static boolean isBufferLate(long earlyUs) {

    return earlyUs < -30000;
  }

  private static boolean isBufferVeryLate(long earlyUs) {

    return earlyUs < -500000;
  }

  @RequiresApi(29)
  private static void setHdr10PlusInfoV29(MediaCodecAdapter codec, byte[] hdr10PlusInfo) {
    Bundle codecParameters = new Bundle();
    codecParameters.putByteArray(MediaCodec.PARAMETER_KEY_HDR10_PLUS_INFO, hdr10PlusInfo);
    codec.setParameters(codecParameters);
  }

  @RequiresApi(23)
  protected void setOutputSurfaceV23(MediaCodecAdapter codec, Surface surface) {
    codec.setOutputSurface(surface);
  }

  @RequiresApi(21)
  private static void configureTunnelingV21(MediaFormat mediaFormat, int tunnelingAudioSessionId) {
    mediaFormat.setFeatureEnabled(CodecCapabilities.FEATURE_TunneledPlayback, true);
    mediaFormat.setInteger(MediaFormat.KEY_AUDIO_SESSION_ID, tunnelingAudioSessionId);
  }

  /**
   * Returns the framework {@link MediaFormat} that should be used to configure the decoder.
   *
   * @param format The {@link Format} of media.
   * @param codecMimeType The MIME type handled by the codec.
   * @param codecMaxValues Codec max values that should be used when configuring the decoder.
   * @param codecOperatingRate The codec operating rate, or {@link #CODEC_OPERATING_RATE_UNSET} if
   *     no codec operating rate should be set.
   * @param deviceNeedsNoPostProcessWorkaround Whether the device is known to do post processing by
   *     default that isn't compatible with ExoPlayer.
   * @param tunnelingAudioSessionId The audio session id to use for tunneling, or {@link
   *     C#AUDIO_SESSION_ID_UNSET} if tunneling should not be enabled.
   * @return The framework {@link MediaFormat} that should be used to configure the decoder.
   */
  @SuppressLint("InlinedApi")
  @TargetApi(21) // tunnelingAudioSessionId is unset if Util.SDK_INT < 21
  protected MediaFormat getMediaFormat(
      Format format,
      String codecMimeType,
      CodecMaxValues codecMaxValues,
      float codecOperatingRate,
      boolean deviceNeedsNoPostProcessWorkaround,
      int tunnelingAudioSessionId) {
    MediaFormat mediaFormat = new MediaFormat();

    mediaFormat.setString(MediaFormat.KEY_MIME, codecMimeType);
    mediaFormat.setInteger(MediaFormat.KEY_WIDTH, format.width);
    mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, format.height);
    MediaFormatUtil.setCsdBuffers(mediaFormat, format.initializationData);

    MediaFormatUtil.maybeSetFloat(mediaFormat, MediaFormat.KEY_FRAME_RATE, format.frameRate);
    MediaFormatUtil.maybeSetInteger(mediaFormat, MediaFormat.KEY_ROTATION, format.rotationDegrees);
    MediaFormatUtil.maybeSetColorInfo(mediaFormat, format.colorInfo);
    if (MimeTypes.VIDEO_DOLBY_VISION.equals(format.sampleMimeType)) {


      Pair<Integer, Integer> codecProfileAndLevel = MediaCodecUtil.getCodecProfileAndLevel(format);
      if (codecProfileAndLevel != null) {
        MediaFormatUtil.maybeSetInteger(
            mediaFormat, MediaFormat.KEY_PROFILE, codecProfileAndLevel.first);
      }
    }

    mediaFormat.setInteger(MediaFormat.KEY_MAX_WIDTH, codecMaxValues.width);
    mediaFormat.setInteger(MediaFormat.KEY_MAX_HEIGHT, codecMaxValues.height);
    MediaFormatUtil.maybeSetInteger(
        mediaFormat, MediaFormat.KEY_MAX_INPUT_SIZE, codecMaxValues.inputSize);

    if (Util.SDK_INT >= 23) {
      mediaFormat.setInteger(MediaFormat.KEY_PRIORITY, 0 /* realtime priority */);
      if (codecOperatingRate != CODEC_OPERATING_RATE_UNSET) {
        mediaFormat.setFloat(MediaFormat.KEY_OPERATING_RATE, codecOperatingRate);
      }
    }
    if (deviceNeedsNoPostProcessWorkaround) {
      mediaFormat.setInteger("no-post-process", 1);
      mediaFormat.setInteger("auto-frc", 0);
    }
    if (tunnelingAudioSessionId != C.AUDIO_SESSION_ID_UNSET) {
      configureTunnelingV21(mediaFormat, tunnelingAudioSessionId);
    }
    return mediaFormat;
  }

  /**
   * Returns {@link CodecMaxValues} suitable for configuring a codec for {@code format} in a way
   * that will allow possible adaptation to other compatible formats in {@code streamFormats}.
   *
   * @param codecInfo Information about the {@link MediaCodec} being configured.
   * @param format The {@link Format} for which the codec is being configured.
   * @param streamFormats The possible stream formats.
   * @return Suitable {@link CodecMaxValues}.
   */
  protected CodecMaxValues getCodecMaxValues(
      MediaCodecInfo codecInfo, Format format, Format[] streamFormats) {
    int maxWidth = format.width;
    int maxHeight = format.height;
    int maxInputSize = getMaxInputSize(codecInfo, format);
    if (streamFormats.length == 1) {


      if (maxInputSize != Format.NO_VALUE) {
        int codecMaxInputSize = getCodecMaxInputSize(codecInfo, format);
        if (codecMaxInputSize != Format.NO_VALUE) {



          int scaledMaxInputSize =
              (int) (maxInputSize * INITIAL_FORMAT_MAX_INPUT_SIZE_SCALE_FACTOR);

          maxInputSize = min(scaledMaxInputSize, codecMaxInputSize);
        }
      }
      return new CodecMaxValues(maxWidth, maxHeight, maxInputSize);
    }
    boolean haveUnknownDimensions = false;
    for (Format streamFormat : streamFormats) {
      if (format.colorInfo != null && streamFormat.colorInfo == null) {


        streamFormat = streamFormat.buildUpon().setColorInfo(format.colorInfo).build();
      }
      if (codecInfo.canReuseCodec(format, streamFormat).result != REUSE_RESULT_NO) {
        haveUnknownDimensions |=
            (streamFormat.width == Format.NO_VALUE || streamFormat.height == Format.NO_VALUE);
        maxWidth = max(maxWidth, streamFormat.width);
        maxHeight = max(maxHeight, streamFormat.height);
        maxInputSize = max(maxInputSize, getMaxInputSize(codecInfo, streamFormat));
      }
    }
    if (haveUnknownDimensions) {
      Log.w(TAG, "Resolutions unknown. Codec max resolution: " + maxWidth + "x" + maxHeight);
      @Nullable Point codecMaxSize = getCodecMaxSize(codecInfo, format);
      if (codecMaxSize != null) {
        maxWidth = max(maxWidth, codecMaxSize.x);
        maxHeight = max(maxHeight, codecMaxSize.y);
        maxInputSize =
            max(
                maxInputSize,
                getCodecMaxInputSize(
                    codecInfo, format.buildUpon().setWidth(maxWidth).setHeight(maxHeight).build()));
        Log.w(TAG, "Codec max resolution adjusted to: " + maxWidth + "x" + maxHeight);
      }
    }
    return new CodecMaxValues(maxWidth, maxHeight, maxInputSize);
  }

  @Override
  protected MediaCodecDecoderException createDecoderException(
      Throwable cause, @Nullable MediaCodecInfo codecInfo) {
    return new MediaCodecVideoDecoderException(cause, codecInfo, surface);
  }

  /**
   * Returns a maximum video size to use when configuring a codec for {@code format} in a way that
   * will allow possible adaptation to other compatible formats that are expected to have the same
   * aspect ratio, but whose sizes are unknown.
   *
   * @param codecInfo Information about the {@link MediaCodec} being configured.
   * @param format The {@link Format} for which the codec is being configured.
   * @return The maximum video size to use, or {@code null} if the size of {@code format} should be
   *     used.
   */
  @Nullable
  private static Point getCodecMaxSize(MediaCodecInfo codecInfo, Format format) {
    boolean isVerticalVideo = format.height > format.width;
    int formatLongEdgePx = isVerticalVideo ? format.height : format.width;
    int formatShortEdgePx = isVerticalVideo ? format.width : format.height;
    float aspectRatio = (float) formatShortEdgePx / formatLongEdgePx;
    for (int longEdgePx : STANDARD_LONG_EDGE_VIDEO_PX) {
      int shortEdgePx = (int) (longEdgePx * aspectRatio);
      if (longEdgePx <= formatLongEdgePx || shortEdgePx <= formatShortEdgePx) {

        return null;
      } else if (Util.SDK_INT >= 21) {
        Point alignedSize =
            codecInfo.alignVideoSizeV21(
                isVerticalVideo ? shortEdgePx : longEdgePx,
                isVerticalVideo ? longEdgePx : shortEdgePx);
        float frameRate = format.frameRate;
        if (codecInfo.isVideoSizeAndRateSupportedV21(alignedSize.x, alignedSize.y, frameRate)) {
          return alignedSize;
        }
      } else {
        try {

          longEdgePx = Util.ceilDivide(longEdgePx, 16) * 16;
          shortEdgePx = Util.ceilDivide(shortEdgePx, 16) * 16;
          if (longEdgePx * shortEdgePx <= MediaCodecUtil.maxH264DecodableFrameSize()) {
            return new Point(
                isVerticalVideo ? shortEdgePx : longEdgePx,
                isVerticalVideo ? longEdgePx : shortEdgePx);
          }
        } catch (DecoderQueryException e) {

          return null;
        }
      }
    }
    return null;
  }

  /**
   * Returns a maximum input buffer size for a given {@link MediaCodec} and {@link Format}.
   *
   * @param codecInfo Information about the {@link MediaCodec} being configured.
   * @param format The format.
   * @return A maximum input buffer size in bytes, or {@link Format#NO_VALUE} if a maximum could not
   *     be determined.
   */
  protected static int getMaxInputSize(MediaCodecInfo codecInfo, Format format) {
    if (format.maxInputSize != Format.NO_VALUE) {


      int totalInitializationDataSize = 0;
      int initializationDataCount = format.initializationData.size();
      for (int i = 0; i < initializationDataCount; i++) {
        totalInitializationDataSize += format.initializationData.get(i).length;
      }
      return format.maxInputSize + totalInitializationDataSize;
    } else {
      return getCodecMaxInputSize(codecInfo, format);
    }
  }

  /**
   * Returns whether the device is known to do post processing by default that isn't compatible with
   * ExoPlayer.
   *
   * @return Whether the device is known to do post processing by default that isn't compatible with
   *     ExoPlayer.
   */
  private static boolean deviceNeedsNoPostProcessWorkaround() {







    return "NVIDIA".equals(Util.MANUFACTURER);
  }

  /*
   * TODO:
   *
   * 1. Validate that Android device certification now ensures correct behavior, and add a
   *    corresponding SDK_INT upper bound for applying the workaround (probably SDK_INT < 26).
   * 2. Determine a complete list of affected devices.
   * 3. Some of the devices in this list only fail to support setOutputSurface when switching from
   *    a SurfaceView provided Surface to a Surface of another type (e.g. TextureView/DummySurface),
   *    and vice versa. One hypothesis is that setOutputSurface fails when the surfaces have
   *    different pixel formats. If we can find a way to query the Surface instances to determine
   *    whether this case applies, then we'll be able to provide a more targeted workaround.
   */
  /**
   * Returns whether the codec is known to implement {@link MediaCodec#setOutputSurface(Surface)}
   * incorrectly.
   *
   * <p>If true is returned then we fall back to releasing and re-instantiating the codec instead.
   *
   * @param name The name of the codec.
   * @return True if the device is known to implement {@link MediaCodec#setOutputSurface(Surface)}
   *     incorrectly.
   */
  protected boolean codecNeedsSetOutputSurfaceWorkaround(String name) {
    if (name.startsWith("OMX.google")) {

      return false;
    }
    synchronized (MediaCodecVideoRenderer.class) {
      if (!evaluatedDeviceNeedsSetOutputSurfaceWorkaround) {
        deviceNeedsSetOutputSurfaceWorkaround = evaluateDeviceNeedsSetOutputSurfaceWorkaround();
        evaluatedDeviceNeedsSetOutputSurfaceWorkaround = true;
      }
    }
    return deviceNeedsSetOutputSurfaceWorkaround;
  }

  protected Surface getSurface() {
    return surface;
  }

  protected static final class CodecMaxValues {

    public final int width;
    public final int height;
    public final int inputSize;

    public CodecMaxValues(int width, int height, int inputSize) {
      this.width = width;
      this.height = height;
      this.inputSize = inputSize;
    }
  }

  /**
   * Returns the maximum sample size assuming three channel 4:2:0 subsampled input frames with the
   * specified {@code minCompressionRatio}
   *
   * @param pixelCount The number of pixels
   * @param minCompressionRatio The minimum compression ratio
   */
  private static int getMaxSampleSize(int pixelCount, int minCompressionRatio) {
    return (pixelCount * 3) / (2 * minCompressionRatio);
  }

  private static boolean evaluateDeviceNeedsSetOutputSurfaceWorkaround() {
    if (Util.SDK_INT <= 28) {






      switch (Util.DEVICE) {
        case "aquaman":
        case "dangal":
        case "dangalUHD":
        case "dangalFHD":
        case "magnolia":
        case "machuca":
        case "once":
        case "oneday":
          return true;
        default:
          break; // Do nothing.
      }
    }
    if (Util.SDK_INT <= 27 && "HWEML".equals(Util.DEVICE)) {


      return true;
    }
    switch (Util.MODEL) {

      case "AFTA":
      case "AFTN":
      case "AFTR":
      case "AFTEU011":
      case "AFTEU014":
      case "AFTEUFF014":
      case "AFTJMST12":
      case "AFTKMST12":
      case "AFTSO001":
        return true;
      default:
        break; // Do nothing.
    }
    if (Util.SDK_INT <= 26) {



















      switch (Util.DEVICE) {
        case "1601":
        case "1713":
        case "1714":
        case "601LV":
        case "602LV":
        case "A10-70F":
        case "A10-70L":
        case "A1601":
        case "A2016a40":
        case "A7000-a":
        case "A7000plus":
        case "A7010a48":
        case "A7020a48":
        case "AquaPowerM":
        case "ASUS_X00AD_2":
        case "Aura_Note_2":
        case "b5":
        case "BLACK-1X":
        case "BRAVIA_ATV2":
        case "BRAVIA_ATV3_4K":
        case "C1":
        case "ComioS1":
        case "CP8676_I02":
        case "CPH1609":
        case "CPH1715":
        case "CPY83_I00":
        case "cv1":
        case "cv3":
        case "deb":
        case "DM-01K":
        case "E5643":
        case "ELUGA_A3_Pro":
        case "ELUGA_Note":
        case "ELUGA_Prim":
        case "ELUGA_Ray_X":
        case "EverStar_S":
        case "F01H":
        case "F01J":
        case "F02H":
        case "F03H":
        case "F04H":
        case "F04J":
        case "F3111":
        case "F3113":
        case "F3116":
        case "F3211":
        case "F3213":
        case "F3215":
        case "F3311":
        case "flo":
        case "fugu":
        case "GiONEE_CBL7513":
        case "GiONEE_GBL7319":
        case "GIONEE_GBL7360":
        case "GIONEE_SWW1609":
        case "GIONEE_SWW1627":
        case "GIONEE_SWW1631":
        case "GIONEE_WBL5708":
        case "GIONEE_WBL7365":
        case "GIONEE_WBL7519":
        case "griffin":
        case "htc_e56ml_dtul":
        case "hwALE-H":
        case "HWBLN-H":
        case "HWCAM-H":
        case "HWVNS-H":
        case "HWWAS-H":
        case "i9031":
        case "iball8735_9806":
        case "Infinix-X572":
        case "iris60":
        case "itel_S41":
        case "j2xlteins":
        case "JGZ":
        case "K50a40":
        case "kate":
        case "l5460":
        case "le_x6":
        case "LS-5017":
        case "M04":
        case "M5c":
        case "manning":
        case "marino_f":
        case "MEIZU_M5":
        case "mh":
        case "mido":
        case "MX6":
        case "namath":
        case "nicklaus_f":
        case "NX541J":
        case "NX573J":
        case "OnePlus5T":
        case "p212":
        case "P681":
        case "P85":
        case "pacificrim":
        case "panell_d":
        case "panell_dl":
        case "panell_ds":
        case "panell_dt":
        case "PB2-670M":
        case "PGN528":
        case "PGN610":
        case "PGN611":
        case "Phantom6":
        case "Pixi4-7_3G":
        case "Pixi5-10_4G":
        case "PLE":
        case "PRO7S":
        case "Q350":
        case "Q4260":
        case "Q427":
        case "Q4310":
        case "Q5":
        case "QM16XE_U":
        case "QX1":
        case "RAIJIN":
        case "santoni":
        case "Slate_Pro":
        case "SVP-DTV15":
        case "s905x018":
        case "taido_row":
        case "TB3-730F":
        case "TB3-730X":
        case "TB3-850F":
        case "TB3-850M":
        case "tcl_eu":
        case "V1":
        case "V23GB":
        case "V5":
        case "vernee_M5":
        case "watson":
        case "whyred":
        case "woods_f":
        case "woods_fn":
        case "X3_HK":
        case "XE2X":
        case "XT1663":
        case "Z12_PRO":
        case "Z80":
          return true;
        default:
          break; // Do nothing.
      }
      switch (Util.MODEL) {
        case "JSN-L21":
          return true;
        default:
          break; // Do nothing.
      }
    }
    return false;
  }

  @RequiresApi(23)
  private final class OnFrameRenderedListenerV23
      implements MediaCodecAdapter.OnFrameRenderedListener, Handler.Callback {

    private static final int HANDLE_FRAME_RENDERED = 0;

    private final Handler handler;

    public OnFrameRenderedListenerV23(MediaCodecAdapter codec) {
      handler = Util.createHandlerForCurrentLooper(/* callback= */ this);
      codec.setOnFrameRenderedListener(/* listener= */ this, handler);
    }

    @Override
    public void onFrameRendered(MediaCodecAdapter codec, long presentationTimeUs, long nanoTime) {







      if (Util.SDK_INT < 30) {
        Message message =
            Message.obtain(
                handler,
                /* what= */ HANDLE_FRAME_RENDERED,
                /* arg1= */ (int) (presentationTimeUs >> 32),
                /* arg2= */ (int) presentationTimeUs);
        handler.sendMessageAtFrontOfQueue(message);
      } else {
        handleFrameRendered(presentationTimeUs);
      }
    }

    @Override
    public boolean handleMessage(Message message) {
      switch (message.what) {
        case HANDLE_FRAME_RENDERED:
          handleFrameRendered(Util.toLong(message.arg1, message.arg2));
          return true;
        default:
          return false;
      }
    }

    private void handleFrameRendered(long presentationTimeUs) {
      if (this != tunnelingOnFrameRenderedListener || getCodec() == null) {

        return;
      }
      if (presentationTimeUs == TUNNELING_EOS_PRESENTATION_TIME_US) {
        onProcessedTunneledEndOfStream();
      } else {
        try {
          onProcessedTunneledBuffer(presentationTimeUs);
        } catch (ExoPlaybackException e) {
          setPendingPlaybackException(e);
        }
      }
    }
  }
}
