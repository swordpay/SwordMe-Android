package org.telegram.ui.Cells;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.View;

import androidx.core.graphics.ColorUtils;

import org.telegram.messenger.AndroidUtilities;
import org.telegram.ui.ActionBar.Theme;


public class SmollEmptySell extends View {

    private boolean forceDarkTheme;
    private Paint paint = new Paint();
    private Theme.ResourcesProvider resourcesProvider;

    public SmollEmptySell(Context context) {
        this(context, null);
    }

    public SmollEmptySell(Context context, Theme.ResourcesProvider resourcesProvider) {
        super(context);
        this.resourcesProvider = resourcesProvider;
        setPadding(0, AndroidUtilities.dp(10), 0, AndroidUtilities.dp(10));
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(MeasureSpec.getSize(widthMeasureSpec), getPaddingTop() + getPaddingBottom() + 1);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (forceDarkTheme) {
            paint.setColor(ColorUtils.blendARGB(Color.TRANSPARENT, Theme.getColor(Theme.key_voipgroup_dialogBackground, resourcesProvider),  0.2f));
        } else {
            paint.setColor(Theme.getColor(Theme.key_divider, resourcesProvider));
        }

        canvas.drawLine(getPaddingLeft(), getPaddingTop(), getWidth() - getPaddingRight(), getPaddingTop(), paint);
    }

    public void setForceDarkTheme(boolean forceDarkTheme) {
        this.forceDarkTheme = forceDarkTheme;
    }
}
