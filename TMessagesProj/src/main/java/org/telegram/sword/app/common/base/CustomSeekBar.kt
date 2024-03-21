package org.telegram.sword.app.common.base

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Rect
import android.util.AttributeSet
import android.view.MotionEvent

class CustomSeekBar(context: Context, attrs: AttributeSet?) : androidx.appcompat.widget.AppCompatSeekBar(context, attrs) {

    private val extraTouchArea = 30

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val rect = Rect()
        getHitRect(rect)
        rect.top -= extraTouchArea
        rect.bottom += extraTouchArea
        rect.left -= extraTouchArea
        rect.right += extraTouchArea

        val x = event.x.toInt()
        val y = event.y.toInt()

        return if (rect.contains(x, y)) {
            super.onTouchEvent(event)
        } else {
            false
        }
    }
}