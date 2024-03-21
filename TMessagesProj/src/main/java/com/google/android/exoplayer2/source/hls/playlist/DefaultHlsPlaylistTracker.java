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
package com.google.android.exoplayer2.source.hls.playlist;

import static com.google.android.exoplayer2.util.Assertions.checkNotNull;
import static com.google.android.exoplayer2.util.Util.castNonNull;
import static java.lang.Math.max;

import android.net.Uri;
import android.os.Handler;
import android.os.SystemClock;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ParserException;
import com.google.android.exoplayer2.source.LoadEventInfo;
import com.google.android.exoplayer2.source.MediaLoadData;
import com.google.android.exoplayer2.source.MediaSourceEventListener.EventDispatcher;
import com.google.android.exoplayer2.source.hls.HlsDataSourceFactory;
import com.google.android.exoplayer2.source.hls.playlist.HlsMediaPlaylist.Part;
import com.google.android.exoplayer2.source.hls.playlist.HlsMediaPlaylist.RenditionReport;
import com.google.android.exoplayer2.source.hls.playlist.HlsMediaPlaylist.Segment;
import com.google.android.exoplayer2.source.hls.playlist.HlsMultivariantPlaylist.Variant;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.HttpDataSource;
import com.google.android.exoplayer2.upstream.LoadErrorHandlingPolicy;
import com.google.android.exoplayer2.upstream.LoadErrorHandlingPolicy.LoadErrorInfo;
import com.google.android.exoplayer2.upstream.Loader;
import com.google.android.exoplayer2.upstream.Loader.LoadErrorAction;
import com.google.android.exoplayer2.upstream.ParsingLoadable;
import com.google.android.exoplayer2.util.Assertions;
import com.google.android.exoplayer2.util.Util;
import com.google.common.collect.Iterables;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/** Default implementation for {@link HlsPlaylistTracker}. */
public final class DefaultHlsPlaylistTracker
    implements HlsPlaylistTracker, Loader.Callback<ParsingLoadable<HlsPlaylist>> {

  /** Factory for {@link DefaultHlsPlaylistTracker} instances. */
  public static final Factory FACTORY = DefaultHlsPlaylistTracker::new;

  /**
   * Default coefficient applied on the target duration of a playlist to determine the amount of
   * time after which an unchanging playlist is considered stuck.
   */
  public static final double DEFAULT_PLAYLIST_STUCK_TARGET_DURATION_COEFFICIENT = 3.5;

  private final HlsDataSourceFactory dataSourceFactory;
  private final HlsPlaylistParserFactory playlistParserFactory;
  private final LoadErrorHandlingPolicy loadErrorHandlingPolicy;
  private final HashMap<Uri, MediaPlaylistBundle> playlistBundles;
  private final CopyOnWriteArrayList<PlaylistEventListener> listeners;
  private final double playlistStuckTargetDurationCoefficient;

  @Nullable private EventDispatcher eventDispatcher;
  @Nullable private Loader initialPlaylistLoader;
  @Nullable private Handler playlistRefreshHandler;
  @Nullable private PrimaryPlaylistListener primaryPlaylistListener;
  @Nullable private HlsMultivariantPlaylist multivariantPlaylist;
  @Nullable private Uri primaryMediaPlaylistUrl;
  @Nullable private HlsMediaPlaylist primaryMediaPlaylistSnapshot;
  private boolean isLive;
  private long initialStartTimeUs;

  /**
   * Creates an instance.
   *
   * @param dataSourceFactory A factory for {@link DataSource} instances.
   * @param loadErrorHandlingPolicy The {@link LoadErrorHandlingPolicy}.
   * @param playlistParserFactory An {@link HlsPlaylistParserFactory}.
   */
  public DefaultHlsPlaylistTracker(
      HlsDataSourceFactory dataSourceFactory,
      LoadErrorHandlingPolicy loadErrorHandlingPolicy,
      HlsPlaylistParserFactory playlistParserFactory) {
    this(
        dataSourceFactory,
        loadErrorHandlingPolicy,
        playlistParserFactory,
        DEFAULT_PLAYLIST_STUCK_TARGET_DURATION_COEFFICIENT);
  }

  /**
   * Creates an instance.
   *
   * @param dataSourceFactory A factory for {@link DataSource} instances.
   * @param loadErrorHandlingPolicy The {@link LoadErrorHandlingPolicy}.
   * @param playlistParserFactory An {@link HlsPlaylistParserFactory}.
   * @param playlistStuckTargetDurationCoefficient A coefficient to apply to the target duration of
   *     media playlists in order to determine that a non-changing playlist is stuck. Once a
   *     playlist is deemed stuck, a {@link PlaylistStuckException} is thrown via {@link
   *     #maybeThrowPlaylistRefreshError(Uri)}.
   */
  public DefaultHlsPlaylistTracker(
      HlsDataSourceFactory dataSourceFactory,
      LoadErrorHandlingPolicy loadErrorHandlingPolicy,
      HlsPlaylistParserFactory playlistParserFactory,
      double playlistStuckTargetDurationCoefficient) {
    this.dataSourceFactory = dataSourceFactory;
    this.playlistParserFactory = playlistParserFactory;
    this.loadErrorHandlingPolicy = loadErrorHandlingPolicy;
    this.playlistStuckTargetDurationCoefficient = playlistStuckTargetDurationCoefficient;
    listeners = new CopyOnWriteArrayList<>();
    playlistBundles = new HashMap<>();
    initialStartTimeUs = C.TIME_UNSET;
  }


  @Override
  public void start(
      Uri initialPlaylistUri,
      EventDispatcher eventDispatcher,
      PrimaryPlaylistListener primaryPlaylistListener) {
    this.playlistRefreshHandler = Util.createHandlerForCurrentLooper();
    this.eventDispatcher = eventDispatcher;
    this.primaryPlaylistListener = primaryPlaylistListener;
    ParsingLoadable<HlsPlaylist> multivariantPlaylistLoadable =
        new ParsingLoadable<>(
            dataSourceFactory.createDataSource(C.DATA_TYPE_MANIFEST),
            initialPlaylistUri,
            C.DATA_TYPE_MANIFEST,
            playlistParserFactory.createPlaylistParser());
    Assertions.checkState(initialPlaylistLoader == null);
    initialPlaylistLoader = new Loader("DefaultHlsPlaylistTracker:MultivariantPlaylist");
    long elapsedRealtime =
        initialPlaylistLoader.startLoading(
            multivariantPlaylistLoadable,
            this,
            loadErrorHandlingPolicy.getMinimumLoadableRetryCount(
                multivariantPlaylistLoadable.type));
    eventDispatcher.loadStarted(
        new LoadEventInfo(
            multivariantPlaylistLoadable.loadTaskId,
            multivariantPlaylistLoadable.dataSpec,
            elapsedRealtime),
        multivariantPlaylistLoadable.type);
  }

  @Override
  public void stop() {
    primaryMediaPlaylistUrl = null;
    primaryMediaPlaylistSnapshot = null;
    multivariantPlaylist = null;
    initialStartTimeUs = C.TIME_UNSET;
    initialPlaylistLoader.release();
    initialPlaylistLoader = null;
    for (MediaPlaylistBundle bundle : playlistBundles.values()) {
      bundle.release();
    }
    playlistRefreshHandler.removeCallbacksAndMessages(null);
    playlistRefreshHandler = null;
    playlistBundles.clear();
  }

  @Override
  public void addListener(PlaylistEventListener listener) {
    checkNotNull(listener);
    listeners.add(listener);
  }

  @Override
  public void removeListener(PlaylistEventListener listener) {
    listeners.remove(listener);
  }

  @Override
  @Nullable
  public HlsMultivariantPlaylist getMultivariantPlaylist() {
    return multivariantPlaylist;
  }

  @Override
  @Nullable
  public HlsMediaPlaylist getPlaylistSnapshot(Uri url, boolean isForPlayback) {
    @Nullable HlsMediaPlaylist snapshot = playlistBundles.get(url).getPlaylistSnapshot();
    if (snapshot != null && isForPlayback) {
      maybeSetPrimaryUrl(url);
    }
    return snapshot;
  }

  @Override
  public long getInitialStartTimeUs() {
    return initialStartTimeUs;
  }

  @Override
  public boolean isSnapshotValid(Uri url) {
    return playlistBundles.get(url).isSnapshotValid();
  }

  @Override
  public void maybeThrowPrimaryPlaylistRefreshError() throws IOException {
    if (initialPlaylistLoader != null) {
      initialPlaylistLoader.maybeThrowError();
    }
    if (primaryMediaPlaylistUrl != null) {
      maybeThrowPlaylistRefreshError(primaryMediaPlaylistUrl);
    }
  }

  @Override
  public void maybeThrowPlaylistRefreshError(Uri url) throws IOException {
    playlistBundles.get(url).maybeThrowPlaylistRefreshError();
  }

  @Override
  public void refreshPlaylist(Uri url) {
    playlistBundles.get(url).loadPlaylist();
  }

  @Override
  public boolean isLive() {
    return isLive;
  }

  @Override
  public boolean excludeMediaPlaylist(Uri playlistUrl, long exclusionDurationMs) {
    @Nullable MediaPlaylistBundle bundle = playlistBundles.get(playlistUrl);
    if (bundle != null) {
      return !bundle.excludePlaylist(exclusionDurationMs);
    }
    return false;
  }


  @Override
  public void onLoadCompleted(
      ParsingLoadable<HlsPlaylist> loadable, long elapsedRealtimeMs, long loadDurationMs) {
    HlsPlaylist result = loadable.getResult();
    HlsMultivariantPlaylist multivariantPlaylist;
    boolean isMediaPlaylist = result instanceof HlsMediaPlaylist;
    if (isMediaPlaylist) {
      multivariantPlaylist =
          HlsMultivariantPlaylist.createSingleVariantMultivariantPlaylist(result.baseUri);
    } else /* result instanceof HlsMultivariantPlaylist */ {
      multivariantPlaylist = (HlsMultivariantPlaylist) result;
    }
    this.multivariantPlaylist = multivariantPlaylist;
    primaryMediaPlaylistUrl = multivariantPlaylist.variants.get(0).url;

    listeners.add(new FirstPrimaryMediaPlaylistListener());
    createBundles(multivariantPlaylist.mediaPlaylistUrls);
    LoadEventInfo loadEventInfo =
        new LoadEventInfo(
            loadable.loadTaskId,
            loadable.dataSpec,
            loadable.getUri(),
            loadable.getResponseHeaders(),
            elapsedRealtimeMs,
            loadDurationMs,
            loadable.bytesLoaded());
    MediaPlaylistBundle primaryBundle = playlistBundles.get(primaryMediaPlaylistUrl);
    if (isMediaPlaylist) {

      primaryBundle.processLoadedPlaylist((HlsMediaPlaylist) result, loadEventInfo);
    } else {
      primaryBundle.loadPlaylist();
    }
    loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
    eventDispatcher.loadCompleted(loadEventInfo, C.DATA_TYPE_MANIFEST);
  }

  @Override
  public void onLoadCanceled(
      ParsingLoadable<HlsPlaylist> loadable,
      long elapsedRealtimeMs,
      long loadDurationMs,
      boolean released) {
    LoadEventInfo loadEventInfo =
        new LoadEventInfo(
            loadable.loadTaskId,
            loadable.dataSpec,
            loadable.getUri(),
            loadable.getResponseHeaders(),
            elapsedRealtimeMs,
            loadDurationMs,
            loadable.bytesLoaded());
    loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
    eventDispatcher.loadCanceled(loadEventInfo, C.DATA_TYPE_MANIFEST);
  }

  @Override
  public LoadErrorAction onLoadError(
      ParsingLoadable<HlsPlaylist> loadable,
      long elapsedRealtimeMs,
      long loadDurationMs,
      IOException error,
      int errorCount) {
    LoadEventInfo loadEventInfo =
        new LoadEventInfo(
            loadable.loadTaskId,
            loadable.dataSpec,
            loadable.getUri(),
            loadable.getResponseHeaders(),
            elapsedRealtimeMs,
            loadDurationMs,
            loadable.bytesLoaded());
    MediaLoadData mediaLoadData = new MediaLoadData(loadable.type);
    long retryDelayMs =
        loadErrorHandlingPolicy.getRetryDelayMsFor(
            new LoadErrorInfo(loadEventInfo, mediaLoadData, error, errorCount));
    boolean isFatal = retryDelayMs == C.TIME_UNSET;
    eventDispatcher.loadError(loadEventInfo, loadable.type, error, isFatal);
    if (isFatal) {
      loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
    }
    return isFatal
        ? Loader.DONT_RETRY_FATAL
        : Loader.createRetryAction(/* resetErrorCount= */ false, retryDelayMs);
  }


  private boolean maybeSelectNewPrimaryUrl() {
    List<Variant> variants = multivariantPlaylist.variants;
    int variantsSize = variants.size();
    long currentTimeMs = SystemClock.elapsedRealtime();
    for (int i = 0; i < variantsSize; i++) {
      MediaPlaylistBundle bundle = checkNotNull(playlistBundles.get(variants.get(i).url));
      if (currentTimeMs > bundle.excludeUntilMs) {
        primaryMediaPlaylistUrl = bundle.playlistUrl;
        bundle.loadPlaylistInternal(getRequestUriForPrimaryChange(primaryMediaPlaylistUrl));
        return true;
      }
    }
    return false;
  }

  private void maybeSetPrimaryUrl(Uri url) {
    if (url.equals(primaryMediaPlaylistUrl)
        || !isVariantUrl(url)
        || (primaryMediaPlaylistSnapshot != null && primaryMediaPlaylistSnapshot.hasEndTag)) {


      return;
    }
    primaryMediaPlaylistUrl = url;
    MediaPlaylistBundle newPrimaryBundle = playlistBundles.get(primaryMediaPlaylistUrl);
    @Nullable HlsMediaPlaylist newPrimarySnapshot = newPrimaryBundle.playlistSnapshot;
    if (newPrimarySnapshot != null && newPrimarySnapshot.hasEndTag) {
      primaryMediaPlaylistSnapshot = newPrimarySnapshot;
      primaryPlaylistListener.onPrimaryPlaylistRefreshed(newPrimarySnapshot);
    } else {


      newPrimaryBundle.loadPlaylistInternal(getRequestUriForPrimaryChange(url));
    }
  }

  private Uri getRequestUriForPrimaryChange(Uri newPrimaryPlaylistUri) {
    if (primaryMediaPlaylistSnapshot != null
        && primaryMediaPlaylistSnapshot.serverControl.canBlockReload) {
      @Nullable
      RenditionReport renditionReport =
          primaryMediaPlaylistSnapshot.renditionReports.get(newPrimaryPlaylistUri);
      if (renditionReport != null) {
        Uri.Builder uriBuilder = newPrimaryPlaylistUri.buildUpon();
        uriBuilder.appendQueryParameter(
            MediaPlaylistBundle.BLOCK_MSN_PARAM, String.valueOf(renditionReport.lastMediaSequence));
        if (renditionReport.lastPartIndex != C.INDEX_UNSET) {
          uriBuilder.appendQueryParameter(
              MediaPlaylistBundle.BLOCK_PART_PARAM, String.valueOf(renditionReport.lastPartIndex));
        }
        return uriBuilder.build();
      }
    }
    return newPrimaryPlaylistUri;
  }

  /**
   * Returns whether any of the variants in the multivariant playlist have the specified playlist
   * URL.
   */
  private boolean isVariantUrl(Uri playlistUrl) {
    List<Variant> variants = multivariantPlaylist.variants;
    for (int i = 0; i < variants.size(); i++) {
      if (playlistUrl.equals(variants.get(i).url)) {
        return true;
      }
    }
    return false;
  }

  private void createBundles(List<Uri> urls) {
    int listSize = urls.size();
    for (int i = 0; i < listSize; i++) {
      Uri url = urls.get(i);
      MediaPlaylistBundle bundle = new MediaPlaylistBundle(url);
      playlistBundles.put(url, bundle);
    }
  }

  /**
   * Called by the bundles when a snapshot changes.
   *
   * @param url The url of the playlist.
   * @param newSnapshot The new snapshot.
   */
  private void onPlaylistUpdated(Uri url, HlsMediaPlaylist newSnapshot) {
    if (url.equals(primaryMediaPlaylistUrl)) {
      if (primaryMediaPlaylistSnapshot == null) {

        isLive = !newSnapshot.hasEndTag;
        initialStartTimeUs = newSnapshot.startTimeUs;
      }
      primaryMediaPlaylistSnapshot = newSnapshot;
      primaryPlaylistListener.onPrimaryPlaylistRefreshed(newSnapshot);
    }
    for (PlaylistEventListener listener : listeners) {
      listener.onPlaylistChanged();
    }
  }

  private boolean notifyPlaylistError(
      Uri playlistUrl, LoadErrorInfo loadErrorInfo, boolean forceRetry) {
    boolean anyExclusionFailed = false;
    for (PlaylistEventListener listener : listeners) {
      anyExclusionFailed |= !listener.onPlaylistError(playlistUrl, loadErrorInfo, forceRetry);
    }
    return anyExclusionFailed;
  }

  private HlsMediaPlaylist getLatestPlaylistSnapshot(
      @Nullable HlsMediaPlaylist oldPlaylist, HlsMediaPlaylist loadedPlaylist) {
    if (!loadedPlaylist.isNewerThan(oldPlaylist)) {
      if (loadedPlaylist.hasEndTag) {




        return oldPlaylist.copyWithEndTag();
      } else {
        return oldPlaylist;
      }
    }
    long startTimeUs = getLoadedPlaylistStartTimeUs(oldPlaylist, loadedPlaylist);
    int discontinuitySequence = getLoadedPlaylistDiscontinuitySequence(oldPlaylist, loadedPlaylist);
    return loadedPlaylist.copyWith(startTimeUs, discontinuitySequence);
  }

  private long getLoadedPlaylistStartTimeUs(
      @Nullable HlsMediaPlaylist oldPlaylist, HlsMediaPlaylist loadedPlaylist) {
    if (loadedPlaylist.hasProgramDateTime) {
      return loadedPlaylist.startTimeUs;
    }
    long primarySnapshotStartTimeUs =
        primaryMediaPlaylistSnapshot != null ? primaryMediaPlaylistSnapshot.startTimeUs : 0;
    if (oldPlaylist == null) {
      return primarySnapshotStartTimeUs;
    }
    int oldPlaylistSize = oldPlaylist.segments.size();
    Segment firstOldOverlappingSegment = getFirstOldOverlappingSegment(oldPlaylist, loadedPlaylist);
    if (firstOldOverlappingSegment != null) {
      return oldPlaylist.startTimeUs + firstOldOverlappingSegment.relativeStartTimeUs;
    } else if (oldPlaylistSize == loadedPlaylist.mediaSequence - oldPlaylist.mediaSequence) {
      return oldPlaylist.getEndTimeUs();
    } else {

      return primarySnapshotStartTimeUs;
    }
  }

  private int getLoadedPlaylistDiscontinuitySequence(
      @Nullable HlsMediaPlaylist oldPlaylist, HlsMediaPlaylist loadedPlaylist) {
    if (loadedPlaylist.hasDiscontinuitySequence) {
      return loadedPlaylist.discontinuitySequence;
    }

    int primaryUrlDiscontinuitySequence =
        primaryMediaPlaylistSnapshot != null
            ? primaryMediaPlaylistSnapshot.discontinuitySequence
            : 0;
    if (oldPlaylist == null) {
      return primaryUrlDiscontinuitySequence;
    }
    Segment firstOldOverlappingSegment = getFirstOldOverlappingSegment(oldPlaylist, loadedPlaylist);
    if (firstOldOverlappingSegment != null) {
      return oldPlaylist.discontinuitySequence
          + firstOldOverlappingSegment.relativeDiscontinuitySequence
          - loadedPlaylist.segments.get(0).relativeDiscontinuitySequence;
    }
    return primaryUrlDiscontinuitySequence;
  }

  private static Segment getFirstOldOverlappingSegment(
      HlsMediaPlaylist oldPlaylist, HlsMediaPlaylist loadedPlaylist) {
    int mediaSequenceOffset = (int) (loadedPlaylist.mediaSequence - oldPlaylist.mediaSequence);
    List<Segment> oldSegments = oldPlaylist.segments;
    return mediaSequenceOffset < oldSegments.size() ? oldSegments.get(mediaSequenceOffset) : null;
  }

  /** Holds all information related to a specific Media Playlist. */
  private final class MediaPlaylistBundle implements Loader.Callback<ParsingLoadable<HlsPlaylist>> {

    private static final String BLOCK_MSN_PARAM = "_HLS_msn";
    private static final String BLOCK_PART_PARAM = "_HLS_part";
    private static final String SKIP_PARAM = "_HLS_skip";

    private final Uri playlistUrl;
    private final Loader mediaPlaylistLoader;
    private final DataSource mediaPlaylistDataSource;

    @Nullable private HlsMediaPlaylist playlistSnapshot;
    private long lastSnapshotLoadMs;
    private long lastSnapshotChangeMs;
    private long earliestNextLoadTimeMs;
    private long excludeUntilMs;
    private boolean loadPending;
    @Nullable private IOException playlistError;

    public MediaPlaylistBundle(Uri playlistUrl) {
      this.playlistUrl = playlistUrl;
      mediaPlaylistLoader = new Loader("DefaultHlsPlaylistTracker:MediaPlaylist");
      mediaPlaylistDataSource = dataSourceFactory.createDataSource(C.DATA_TYPE_MANIFEST);
    }

    @Nullable
    public HlsMediaPlaylist getPlaylistSnapshot() {
      return playlistSnapshot;
    }

    public boolean isSnapshotValid() {
      if (playlistSnapshot == null) {
        return false;
      }
      long currentTimeMs = SystemClock.elapsedRealtime();
      long snapshotValidityDurationMs = max(30000, Util.usToMs(playlistSnapshot.durationUs));
      return playlistSnapshot.hasEndTag
          || playlistSnapshot.playlistType == HlsMediaPlaylist.PLAYLIST_TYPE_EVENT
          || playlistSnapshot.playlistType == HlsMediaPlaylist.PLAYLIST_TYPE_VOD
          || lastSnapshotLoadMs + snapshotValidityDurationMs > currentTimeMs;
    }

    public void loadPlaylist() {
      loadPlaylistInternal(playlistUrl);
    }

    public void maybeThrowPlaylistRefreshError() throws IOException {
      mediaPlaylistLoader.maybeThrowError();
      if (playlistError != null) {
        throw playlistError;
      }
    }

    public void release() {
      mediaPlaylistLoader.release();
    }


    @Override
    public void onLoadCompleted(
        ParsingLoadable<HlsPlaylist> loadable, long elapsedRealtimeMs, long loadDurationMs) {
      @Nullable HlsPlaylist result = loadable.getResult();
      LoadEventInfo loadEventInfo =
          new LoadEventInfo(
              loadable.loadTaskId,
              loadable.dataSpec,
              loadable.getUri(),
              loadable.getResponseHeaders(),
              elapsedRealtimeMs,
              loadDurationMs,
              loadable.bytesLoaded());
      if (result instanceof HlsMediaPlaylist) {
        processLoadedPlaylist((HlsMediaPlaylist) result, loadEventInfo);
        eventDispatcher.loadCompleted(loadEventInfo, C.DATA_TYPE_MANIFEST);
      } else {
        playlistError =
            ParserException.createForMalformedManifest(
                "Loaded playlist has unexpected type.", /* cause= */ null);
        eventDispatcher.loadError(
            loadEventInfo, C.DATA_TYPE_MANIFEST, playlistError, /* wasCanceled= */ true);
      }
      loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
    }

    @Override
    public void onLoadCanceled(
        ParsingLoadable<HlsPlaylist> loadable,
        long elapsedRealtimeMs,
        long loadDurationMs,
        boolean released) {
      LoadEventInfo loadEventInfo =
          new LoadEventInfo(
              loadable.loadTaskId,
              loadable.dataSpec,
              loadable.getUri(),
              loadable.getResponseHeaders(),
              elapsedRealtimeMs,
              loadDurationMs,
              loadable.bytesLoaded());
      loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
      eventDispatcher.loadCanceled(loadEventInfo, C.DATA_TYPE_MANIFEST);
    }

    @Override
    public LoadErrorAction onLoadError(
        ParsingLoadable<HlsPlaylist> loadable,
        long elapsedRealtimeMs,
        long loadDurationMs,
        IOException error,
        int errorCount) {
      LoadEventInfo loadEventInfo =
          new LoadEventInfo(
              loadable.loadTaskId,
              loadable.dataSpec,
              loadable.getUri(),
              loadable.getResponseHeaders(),
              elapsedRealtimeMs,
              loadDurationMs,
              loadable.bytesLoaded());
      boolean isBlockingRequest = loadable.getUri().getQueryParameter(BLOCK_MSN_PARAM) != null;
      boolean deltaUpdateFailed = error instanceof HlsPlaylistParser.DeltaUpdateException;
      if (isBlockingRequest || deltaUpdateFailed) {
        int responseCode = Integer.MAX_VALUE;
        if (error instanceof HttpDataSource.InvalidResponseCodeException) {
          responseCode = ((HttpDataSource.InvalidResponseCodeException) error).responseCode;
        }
        if (deltaUpdateFailed || responseCode == 400 || responseCode == 503) {



          earliestNextLoadTimeMs = SystemClock.elapsedRealtime();
          loadPlaylist();
          castNonNull(eventDispatcher)
              .loadError(loadEventInfo, loadable.type, error, /* wasCanceled= */ true);
          return Loader.DONT_RETRY;
        }
      }
      MediaLoadData mediaLoadData = new MediaLoadData(loadable.type);
      LoadErrorInfo loadErrorInfo =
          new LoadErrorInfo(loadEventInfo, mediaLoadData, error, errorCount);
      boolean exclusionFailed =
          notifyPlaylistError(playlistUrl, loadErrorInfo, /* forceRetry= */ false);
      LoadErrorAction loadErrorAction;
      if (exclusionFailed) {
        long retryDelay = loadErrorHandlingPolicy.getRetryDelayMsFor(loadErrorInfo);
        loadErrorAction =
            retryDelay != C.TIME_UNSET
                ? Loader.createRetryAction(false, retryDelay)
                : Loader.DONT_RETRY_FATAL;
      } else {
        loadErrorAction = Loader.DONT_RETRY;
      }

      boolean wasCanceled = !loadErrorAction.isRetry();
      eventDispatcher.loadError(loadEventInfo, loadable.type, error, wasCanceled);
      if (wasCanceled) {
        loadErrorHandlingPolicy.onLoadTaskConcluded(loadable.loadTaskId);
      }
      return loadErrorAction;
    }


    private void loadPlaylistInternal(Uri playlistRequestUri) {
      excludeUntilMs = 0;
      if (loadPending || mediaPlaylistLoader.isLoading() || mediaPlaylistLoader.hasFatalError()) {

        return;
      }
      long currentTimeMs = SystemClock.elapsedRealtime();
      if (currentTimeMs < earliestNextLoadTimeMs) {
        loadPending = true;
        playlistRefreshHandler.postDelayed(
            () -> {
              loadPending = false;
              loadPlaylistImmediately(playlistRequestUri);
            },
            earliestNextLoadTimeMs - currentTimeMs);
      } else {
        loadPlaylistImmediately(playlistRequestUri);
      }
    }

    private void loadPlaylistImmediately(Uri playlistRequestUri) {
      ParsingLoadable.Parser<HlsPlaylist> mediaPlaylistParser =
          playlistParserFactory.createPlaylistParser(multivariantPlaylist, playlistSnapshot);
      ParsingLoadable<HlsPlaylist> mediaPlaylistLoadable =
          new ParsingLoadable<>(
              mediaPlaylistDataSource,
              playlistRequestUri,
              C.DATA_TYPE_MANIFEST,
              mediaPlaylistParser);
      long elapsedRealtime =
          mediaPlaylistLoader.startLoading(
              mediaPlaylistLoadable,
              /* callback= */ this,
              loadErrorHandlingPolicy.getMinimumLoadableRetryCount(mediaPlaylistLoadable.type));
      eventDispatcher.loadStarted(
          new LoadEventInfo(
              mediaPlaylistLoadable.loadTaskId, mediaPlaylistLoadable.dataSpec, elapsedRealtime),
          mediaPlaylistLoadable.type);
    }

    private void processLoadedPlaylist(
        HlsMediaPlaylist loadedPlaylist, LoadEventInfo loadEventInfo) {
      @Nullable HlsMediaPlaylist oldPlaylist = playlistSnapshot;
      long currentTimeMs = SystemClock.elapsedRealtime();
      lastSnapshotLoadMs = currentTimeMs;
      playlistSnapshot = getLatestPlaylistSnapshot(oldPlaylist, loadedPlaylist);
      if (playlistSnapshot != oldPlaylist) {
        playlistError = null;
        lastSnapshotChangeMs = currentTimeMs;
        onPlaylistUpdated(playlistUrl, playlistSnapshot);
      } else if (!playlistSnapshot.hasEndTag) {
        boolean forceRetry = false;
        @Nullable IOException playlistError = null;
        if (loadedPlaylist.mediaSequence + loadedPlaylist.segments.size()
            < playlistSnapshot.mediaSequence) {



          forceRetry = true;
          playlistError = new PlaylistResetException(playlistUrl);
        } else if (currentTimeMs - lastSnapshotChangeMs
            > Util.usToMs(playlistSnapshot.targetDurationUs)
                * playlistStuckTargetDurationCoefficient) {

          playlistError = new PlaylistStuckException(playlistUrl);
        }
        if (playlistError != null) {
          this.playlistError = playlistError;
          notifyPlaylistError(
              playlistUrl,
              new LoadErrorInfo(
                  loadEventInfo,
                  new MediaLoadData(C.DATA_TYPE_MANIFEST),
                  playlistError,
                  /* errorCount= */ 1),
              forceRetry);
        }
      }
      long durationUntilNextLoadUs = 0L;
      if (!playlistSnapshot.serverControl.canBlockReload) {


        durationUntilNextLoadUs =
            playlistSnapshot != oldPlaylist
                ? playlistSnapshot.targetDurationUs
                : (playlistSnapshot.targetDurationUs / 2);
      }
      earliestNextLoadTimeMs = currentTimeMs + Util.usToMs(durationUntilNextLoadUs);



      boolean scheduleLoad =
          playlistSnapshot.partTargetDurationUs != C.TIME_UNSET
              || playlistUrl.equals(primaryMediaPlaylistUrl);
      if (scheduleLoad && !playlistSnapshot.hasEndTag) {
        loadPlaylistInternal(getMediaPlaylistUriForReload());
      }
    }

    private Uri getMediaPlaylistUriForReload() {
      if (playlistSnapshot == null
          || (playlistSnapshot.serverControl.skipUntilUs == C.TIME_UNSET
              && !playlistSnapshot.serverControl.canBlockReload)) {
        return playlistUrl;
      }
      Uri.Builder uriBuilder = playlistUrl.buildUpon();
      if (playlistSnapshot.serverControl.canBlockReload) {
        long targetMediaSequence =
            playlistSnapshot.mediaSequence + playlistSnapshot.segments.size();
        uriBuilder.appendQueryParameter(BLOCK_MSN_PARAM, String.valueOf(targetMediaSequence));
        if (playlistSnapshot.partTargetDurationUs != C.TIME_UNSET) {
          List<Part> trailingParts = playlistSnapshot.trailingParts;
          int targetPartIndex = trailingParts.size();
          if (!trailingParts.isEmpty() && Iterables.getLast(trailingParts).isPreload) {

            targetPartIndex--;
          }
          uriBuilder.appendQueryParameter(BLOCK_PART_PARAM, String.valueOf(targetPartIndex));
        }
      }
      if (playlistSnapshot.serverControl.skipUntilUs != C.TIME_UNSET) {
        uriBuilder.appendQueryParameter(
            SKIP_PARAM, playlistSnapshot.serverControl.canSkipDateRanges ? "v2" : "YES");
      }
      return uriBuilder.build();
    }

    /**
     * Excludes the playlist.
     *
     * @param exclusionDurationMs The number of milliseconds for which the playlist should be
     *     excluded.
     * @return Whether the playlist is the primary, despite being excluded.
     */
    private boolean excludePlaylist(long exclusionDurationMs) {
      excludeUntilMs = SystemClock.elapsedRealtime() + exclusionDurationMs;
      return playlistUrl.equals(primaryMediaPlaylistUrl) && !maybeSelectNewPrimaryUrl();
    }
  }

  /**
   * Takes care of handling load errors of the first media playlist and applies exclusion according
   * to the {@link LoadErrorHandlingPolicy} before the first media period has been created and
   * prepared.
   */
  private class FirstPrimaryMediaPlaylistListener implements PlaylistEventListener {

    @Override
    public void onPlaylistChanged() {

      listeners.remove(this);
    }

    @Override
    public boolean onPlaylistError(Uri url, LoadErrorInfo loadErrorInfo, boolean forceRetry) {
      if (primaryMediaPlaylistSnapshot == null) {
        long nowMs = SystemClock.elapsedRealtime();
        int variantExclusionCounter = 0;
        List<Variant> variants = castNonNull(multivariantPlaylist).variants;
        for (int i = 0; i < variants.size(); i++) {
          @Nullable
          MediaPlaylistBundle mediaPlaylistBundle = playlistBundles.get(variants.get(i).url);
          if (mediaPlaylistBundle != null && nowMs < mediaPlaylistBundle.excludeUntilMs) {
            variantExclusionCounter++;
          }
        }
        LoadErrorHandlingPolicy.FallbackOptions fallbackOptions =
            new LoadErrorHandlingPolicy.FallbackOptions(
                /* numberOfLocations= */ 1,
                /* numberOfExcludedLocations= */ 0,
                /* numberOfTracks= */ multivariantPlaylist.variants.size(),
                /* numberOfExcludedTracks= */ variantExclusionCounter);
        @Nullable
        LoadErrorHandlingPolicy.FallbackSelection fallbackSelection =
            loadErrorHandlingPolicy.getFallbackSelectionFor(fallbackOptions, loadErrorInfo);
        if (fallbackSelection != null
            && fallbackSelection.type == LoadErrorHandlingPolicy.FALLBACK_TYPE_TRACK) {
          @Nullable MediaPlaylistBundle mediaPlaylistBundle = playlistBundles.get(url);
          if (mediaPlaylistBundle != null) {
            mediaPlaylistBundle.excludePlaylist(fallbackSelection.exclusionDurationMs);
          }
        }
      }
      return false;
    }
  }
}
