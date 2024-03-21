package org.telegram.ui.Components;

import android.content.Context;
import android.graphics.PointF;
import android.view.View;
import android.view.animation.Interpolator;

import androidx.recyclerview.widget.LinearSmoothScroller;
import androidx.recyclerview.widget.RecyclerView;

import org.telegram.messenger.AndroidUtilities;

public class SmoothScroller extends LinearSmoothScroller {

    private Interpolator interpolator = CubicBezierInterpolator.DEFAULT;

    public SmoothScroller(Context context) {
        super(context);
    }

    public SmoothScroller(Context context, Interpolator customInterpolator) {
        super(context);
        this.interpolator = customInterpolator;
    }

    protected void onEnd() {

    }

    @Override
    protected void updateActionForInterimTarget(Action action) {

        PointF scrollVector = computeScrollVectorForPosition(getTargetPosition());
        if (scrollVector == null || (scrollVector.x == 0 && scrollVector.y == 0)) {
            final int target = getTargetPosition();
            action.jumpTo(target);
            stop();
            return;
        }
        normalize(scrollVector);
        mTargetVector = scrollVector;

        mInterimTargetDx = (int) (TARGET_SEEK_SCROLL_DISTANCE_PX * scrollVector.x);
        mInterimTargetDy = (int) (TARGET_SEEK_SCROLL_DISTANCE_PX * scrollVector.y);
        final int time = calculateTimeForScrolling(TARGET_SEEK_SCROLL_DISTANCE_PX);



        action.update((int) (mInterimTargetDx * TARGET_SEEK_EXTRA_SCROLL_RATIO),
                (int) (mInterimTargetDy * TARGET_SEEK_EXTRA_SCROLL_RATIO),
                (int) (time * TARGET_SEEK_EXTRA_SCROLL_RATIO), interpolator);
    }

    @Override
    protected void onTargetFound(View targetView, RecyclerView.State state, Action action) {
        final int dx = calculateDxToMakeVisible(targetView, getHorizontalSnapPreference());
        final int dy = calculateDyToMakeVisible(targetView, getVerticalSnapPreference());
        final int distance = (int) Math.sqrt(dx * dx + dy * dy);
        final int time = calculateTimeForDeceleration(distance);
        if (time > 0) {
            action.update(-dx, -dy, time, interpolator);
        }
        AndroidUtilities.runOnUIThread(this::onEnd, Math.max(0, time));
    }

    protected int calculateTimeForDeceleration(int dx) {
        return Math.min(super.calculateTimeForDeceleration(dx), 500);
    }
    protected int calculateTimeForScrolling(int dx) {
        return Math.min(super.calculateTimeForScrolling(dx), 150);
    }
}
