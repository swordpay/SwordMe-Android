/*
 * Copyright (C) 2019 The Android Open Source Project
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
package com.google.android.exoplayer2.analytics;

import static java.lang.Math.max;

import android.util.Base64;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.Player.DiscontinuityReason;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.analytics.AnalyticsListener.EventTime;
import com.google.android.exoplayer2.source.MediaSource.MediaPeriodId;
import com.google.android.exoplayer2.util.Assertions;
import com.google.android.exoplayer2.util.Util;
import com.google.common.base.Supplier;

import org.checkerframework.checker.nullness.qual.MonotonicNonNull;
import org.checkerframework.checker.nullness.qual.RequiresNonNull;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Random;

/**
 * Default {@link PlaybackSessionManager} which instantiates a new session for each window in the
 * timeline and also for each ad within the windows.
 *
 * <p>By default, sessions are identified by Base64-encoded, URL-safe, random strings.
 */
public final class DefaultPlaybackSessionManager implements PlaybackSessionManager {

  /** Default generator for unique session ids that are random, Based64-encoded and URL-safe. */
  public static final Supplier<String> DEFAULT_SESSION_ID_GENERATOR =
      DefaultPlaybackSessionManager::generateDefaultSessionId;

  private static final Random RANDOM = new Random();
  private static final int SESSION_ID_LENGTH = 12;

  private final Timeline.Window window;
  private final Timeline.Period period;
  private final HashMap<String, SessionDescriptor> sessions;
  private final Supplier<String> sessionIdGenerator;

  private @MonotonicNonNull Listener listener;
  private Timeline currentTimeline;
  @Nullable private String currentSessionId;

  /**
   * Creates session manager with a {@link #DEFAULT_SESSION_ID_GENERATOR} to generate session ids.
   */
  public DefaultPlaybackSessionManager() {
    this(DEFAULT_SESSION_ID_GENERATOR);
  }

  /**
   * Creates session manager.
   *
   * @param sessionIdGenerator A generator for new session ids. All generated session ids must be
   *     unique.
   */
  public DefaultPlaybackSessionManager(Supplier<String> sessionIdGenerator) {
    this.sessionIdGenerator = sessionIdGenerator;
    window = new Timeline.Window();
    period = new Timeline.Period();
    sessions = new HashMap<>();
    currentTimeline = Timeline.EMPTY;
  }

  @Override
  public void setListener(Listener listener) {
    this.listener = listener;
  }

  @Override
  public synchronized String getSessionForMediaPeriodId(
      Timeline timeline, MediaPeriodId mediaPeriodId) {
    int windowIndex = timeline.getPeriodByUid(mediaPeriodId.periodUid, period).windowIndex;
    return getOrAddSession(windowIndex, mediaPeriodId).sessionId;
  }

  @Override
  public synchronized boolean belongsToSession(EventTime eventTime, String sessionId) {
    SessionDescriptor sessionDescriptor = sessions.get(sessionId);
    if (sessionDescriptor == null) {
      return false;
    }
    sessionDescriptor.maybeSetWindowSequenceNumber(eventTime.windowIndex, eventTime.mediaPeriodId);
    return sessionDescriptor.belongsToSession(eventTime.windowIndex, eventTime.mediaPeriodId);
  }

  @Override
  public synchronized void updateSessions(EventTime eventTime) {
    Assertions.checkNotNull(listener);
    if (eventTime.timeline.isEmpty()) {

      return;
    }
    @Nullable SessionDescriptor currentSession = sessions.get(currentSessionId);
    if (eventTime.mediaPeriodId != null && currentSession != null) {




      boolean isAlreadyFinished =
          currentSession.windowSequenceNumber == C.INDEX_UNSET
              ? currentSession.windowIndex != eventTime.windowIndex
              : eventTime.mediaPeriodId.windowSequenceNumber < currentSession.windowSequenceNumber;
      if (isAlreadyFinished) {
        return;
      }
    }
    SessionDescriptor eventSession =
        getOrAddSession(eventTime.windowIndex, eventTime.mediaPeriodId);
    if (currentSessionId == null) {
      currentSessionId = eventSession.sessionId;
    }
    if (eventTime.mediaPeriodId != null && eventTime.mediaPeriodId.isAd()) {

      MediaPeriodId contentMediaPeriodId =
          new MediaPeriodId(
              eventTime.mediaPeriodId.periodUid,
              eventTime.mediaPeriodId.windowSequenceNumber,
              eventTime.mediaPeriodId.adGroupIndex);
      SessionDescriptor contentSession =
          getOrAddSession(eventTime.windowIndex, contentMediaPeriodId);
      if (!contentSession.isCreated) {
        contentSession.isCreated = true;
        eventTime.timeline.getPeriodByUid(eventTime.mediaPeriodId.periodUid, period);
        long adGroupPositionMs =
            Util.usToMs(period.getAdGroupTimeUs(eventTime.mediaPeriodId.adGroupIndex))
                + period.getPositionInWindowMs();

        adGroupPositionMs = max(0, adGroupPositionMs);
        EventTime eventTimeForContent =
            new EventTime(
                eventTime.realtimeMs,
                eventTime.timeline,
                eventTime.windowIndex,
                contentMediaPeriodId,
                /* eventPlaybackPositionMs= */ adGroupPositionMs,
                eventTime.currentTimeline,
                eventTime.currentWindowIndex,
                eventTime.currentMediaPeriodId,
                eventTime.currentPlaybackPositionMs,
                eventTime.totalBufferedDurationMs);
        listener.onSessionCreated(eventTimeForContent, contentSession.sessionId);
      }
    }
    if (!eventSession.isCreated) {
      eventSession.isCreated = true;
      listener.onSessionCreated(eventTime, eventSession.sessionId);
    }
    if (eventSession.sessionId.equals(currentSessionId) && !eventSession.isActive) {
      eventSession.isActive = true;
      listener.onSessionActive(eventTime, eventSession.sessionId);
    }
  }

  @Override
  public synchronized void updateSessionsWithTimelineChange(EventTime eventTime) {
    Assertions.checkNotNull(listener);
    Timeline previousTimeline = currentTimeline;
    currentTimeline = eventTime.timeline;
    Iterator<SessionDescriptor> iterator = sessions.values().iterator();
    while (iterator.hasNext()) {
      SessionDescriptor session = iterator.next();
      if (!session.tryResolvingToNewTimeline(previousTimeline, currentTimeline)
          || session.isFinishedAtEventTime(eventTime)) {
        iterator.remove();
        if (session.isCreated) {
          if (session.sessionId.equals(currentSessionId)) {
            currentSessionId = null;
          }
          listener.onSessionFinished(
              eventTime, session.sessionId, /* automaticTransitionToNextPlayback= */ false);
        }
      }
    }
    updateCurrentSession(eventTime);
  }

  @Override
  public synchronized void updateSessionsWithDiscontinuity(
      EventTime eventTime, @DiscontinuityReason int reason) {
    Assertions.checkNotNull(listener);
    boolean hasAutomaticTransition = reason == Player.DISCONTINUITY_REASON_AUTO_TRANSITION;
    Iterator<SessionDescriptor> iterator = sessions.values().iterator();
    while (iterator.hasNext()) {
      SessionDescriptor session = iterator.next();
      if (session.isFinishedAtEventTime(eventTime)) {
        iterator.remove();
        if (session.isCreated) {
          boolean isRemovingCurrentSession = session.sessionId.equals(currentSessionId);
          boolean isAutomaticTransition =
              hasAutomaticTransition && isRemovingCurrentSession && session.isActive;
          if (isRemovingCurrentSession) {
            currentSessionId = null;
          }
          listener.onSessionFinished(eventTime, session.sessionId, isAutomaticTransition);
        }
      }
    }
    updateCurrentSession(eventTime);
  }

  @Override
  @Nullable
  public synchronized String getActiveSessionId() {
    return currentSessionId;
  }

  @Override
  public synchronized void finishAllSessions(EventTime eventTime) {
    currentSessionId = null;
    Iterator<SessionDescriptor> iterator = sessions.values().iterator();
    while (iterator.hasNext()) {
      SessionDescriptor session = iterator.next();
      iterator.remove();
      if (session.isCreated && listener != null) {
        listener.onSessionFinished(
            eventTime, session.sessionId, /* automaticTransitionToNextPlayback= */ false);
      }
    }
  }

  @RequiresNonNull("listener")
  private void updateCurrentSession(EventTime eventTime) {
    if (eventTime.timeline.isEmpty()) {

      currentSessionId = null;
      return;
    }
    @Nullable SessionDescriptor previousSessionDescriptor = sessions.get(currentSessionId);
    SessionDescriptor currentSessionDescriptor =
        getOrAddSession(eventTime.windowIndex, eventTime.mediaPeriodId);
    currentSessionId = currentSessionDescriptor.sessionId;
    updateSessions(eventTime);
    if (eventTime.mediaPeriodId != null
        && eventTime.mediaPeriodId.isAd()
        && (previousSessionDescriptor == null
            || previousSessionDescriptor.windowSequenceNumber
                != eventTime.mediaPeriodId.windowSequenceNumber
            || previousSessionDescriptor.adMediaPeriodId == null
            || previousSessionDescriptor.adMediaPeriodId.adGroupIndex
                != eventTime.mediaPeriodId.adGroupIndex
            || previousSessionDescriptor.adMediaPeriodId.adIndexInAdGroup
                != eventTime.mediaPeriodId.adIndexInAdGroup)) {

      MediaPeriodId contentMediaPeriodId =
          new MediaPeriodId(
              eventTime.mediaPeriodId.periodUid, eventTime.mediaPeriodId.windowSequenceNumber);
      SessionDescriptor contentSession =
          getOrAddSession(eventTime.windowIndex, contentMediaPeriodId);
      listener.onAdPlaybackStarted(
          eventTime, contentSession.sessionId, currentSessionDescriptor.sessionId);
    }
  }

  private SessionDescriptor getOrAddSession(
      int windowIndex, @Nullable MediaPeriodId mediaPeriodId) {




    SessionDescriptor bestMatch = null;
    long bestMatchWindowSequenceNumber = Long.MAX_VALUE;
    for (SessionDescriptor sessionDescriptor : sessions.values()) {
      sessionDescriptor.maybeSetWindowSequenceNumber(windowIndex, mediaPeriodId);
      if (sessionDescriptor.belongsToSession(windowIndex, mediaPeriodId)) {
        long windowSequenceNumber = sessionDescriptor.windowSequenceNumber;
        if (windowSequenceNumber == C.INDEX_UNSET
            || windowSequenceNumber < bestMatchWindowSequenceNumber) {
          bestMatch = sessionDescriptor;
          bestMatchWindowSequenceNumber = windowSequenceNumber;
        } else if (windowSequenceNumber == bestMatchWindowSequenceNumber
            && Util.castNonNull(bestMatch).adMediaPeriodId != null
            && sessionDescriptor.adMediaPeriodId != null) {
          bestMatch = sessionDescriptor;
        }
      }
    }
    if (bestMatch == null) {
      String sessionId = sessionIdGenerator.get();
      bestMatch = new SessionDescriptor(sessionId, windowIndex, mediaPeriodId);
      sessions.put(sessionId, bestMatch);
    }
    return bestMatch;
  }

  private static String generateDefaultSessionId() {
    byte[] randomBytes = new byte[SESSION_ID_LENGTH];
    RANDOM.nextBytes(randomBytes);
    return Base64.encodeToString(randomBytes, Base64.URL_SAFE | Base64.NO_WRAP);
  }

  /**
   * Descriptor for a session.
   *
   * <p>The session may be described in one of three ways:
   *
   * <ul>
   *   <li>A window index with unset window sequence number and a null ad media period id
   *   <li>A content window with index and sequence number, but a null ad media period id.
   *   <li>An ad with all values set.
   * </ul>
   */
  private final class SessionDescriptor {

    private final String sessionId;

    private int windowIndex;
    private long windowSequenceNumber;
    private @MonotonicNonNull MediaPeriodId adMediaPeriodId;

    private boolean isCreated;
    private boolean isActive;

    public SessionDescriptor(
        String sessionId, int windowIndex, @Nullable MediaPeriodId mediaPeriodId) {
      this.sessionId = sessionId;
      this.windowIndex = windowIndex;
      this.windowSequenceNumber =
          mediaPeriodId == null ? C.INDEX_UNSET : mediaPeriodId.windowSequenceNumber;
      if (mediaPeriodId != null && mediaPeriodId.isAd()) {
        this.adMediaPeriodId = mediaPeriodId;
      }
    }

    public boolean tryResolvingToNewTimeline(Timeline oldTimeline, Timeline newTimeline) {
      windowIndex = resolveWindowIndexToNewTimeline(oldTimeline, newTimeline, windowIndex);
      if (windowIndex == C.INDEX_UNSET) {
        return false;
      }
      if (adMediaPeriodId == null) {
        return true;
      }
      int newPeriodIndex = newTimeline.getIndexOfPeriod(adMediaPeriodId.periodUid);
      return newPeriodIndex != C.INDEX_UNSET;
    }

    public boolean belongsToSession(
        int eventWindowIndex, @Nullable MediaPeriodId eventMediaPeriodId) {
      if (eventMediaPeriodId == null) {

        return eventWindowIndex == windowIndex;
      }
      if (adMediaPeriodId == null) {


        return !eventMediaPeriodId.isAd()
            && eventMediaPeriodId.windowSequenceNumber == windowSequenceNumber;
      }

      return eventMediaPeriodId.windowSequenceNumber == adMediaPeriodId.windowSequenceNumber
          && eventMediaPeriodId.adGroupIndex == adMediaPeriodId.adGroupIndex
          && eventMediaPeriodId.adIndexInAdGroup == adMediaPeriodId.adIndexInAdGroup;
    }

    public void maybeSetWindowSequenceNumber(
        int eventWindowIndex, @Nullable MediaPeriodId eventMediaPeriodId) {
      if (windowSequenceNumber == C.INDEX_UNSET
          && eventWindowIndex == windowIndex
          && eventMediaPeriodId != null) {

        windowSequenceNumber = eventMediaPeriodId.windowSequenceNumber;
      }
    }

    public boolean isFinishedAtEventTime(EventTime eventTime) {
      if (eventTime.mediaPeriodId == null) {


        return windowIndex != eventTime.windowIndex;
      }
      if (windowSequenceNumber == C.INDEX_UNSET) {

        return false;
      }
      if (eventTime.mediaPeriodId.windowSequenceNumber > windowSequenceNumber) {

        return true;
      }
      if (adMediaPeriodId == null) {

        return false;
      }
      int eventPeriodIndex = eventTime.timeline.getIndexOfPeriod(eventTime.mediaPeriodId.periodUid);
      int adPeriodIndex = eventTime.timeline.getIndexOfPeriod(adMediaPeriodId.periodUid);
      if (eventTime.mediaPeriodId.windowSequenceNumber < adMediaPeriodId.windowSequenceNumber
          || eventPeriodIndex < adPeriodIndex) {

        return false;
      }
      if (eventPeriodIndex > adPeriodIndex) {

        return true;
      }
      if (eventTime.mediaPeriodId.isAd()) {
        int eventAdGroup = eventTime.mediaPeriodId.adGroupIndex;
        int eventAdIndex = eventTime.mediaPeriodId.adIndexInAdGroup;

        return eventAdGroup > adMediaPeriodId.adGroupIndex
            || (eventAdGroup == adMediaPeriodId.adGroupIndex
                && eventAdIndex > adMediaPeriodId.adIndexInAdGroup);
      } else {

        return eventTime.mediaPeriodId.nextAdGroupIndex == C.INDEX_UNSET
            || eventTime.mediaPeriodId.nextAdGroupIndex > adMediaPeriodId.adGroupIndex;
      }
    }

    private int resolveWindowIndexToNewTimeline(
        Timeline oldTimeline, Timeline newTimeline, int windowIndex) {
      if (windowIndex >= oldTimeline.getWindowCount()) {
        return windowIndex < newTimeline.getWindowCount() ? windowIndex : C.INDEX_UNSET;
      }
      oldTimeline.getWindow(windowIndex, window);
      for (int periodIndex = window.firstPeriodIndex;
          periodIndex <= window.lastPeriodIndex;
          periodIndex++) {
        Object periodUid = oldTimeline.getUidOfPeriod(periodIndex);
        int newPeriodIndex = newTimeline.getIndexOfPeriod(periodUid);
        if (newPeriodIndex != C.INDEX_UNSET) {
          return newTimeline.getPeriod(newPeriodIndex, period).windowIndex;
        }
      }
      return C.INDEX_UNSET;
    }
  }
}
