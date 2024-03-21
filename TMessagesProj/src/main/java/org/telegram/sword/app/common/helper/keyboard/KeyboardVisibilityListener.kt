package org.telegram.sword.app.common.helper.keyboard

import android.app.Activity
import android.graphics.Rect
import android.view.View
import android.view.ViewTreeObserver

class KeyboardVisibilityListener(private val activity: Activity, private val onKeyboardVisibilityChanged: (Boolean) -> Unit) {

    private val rootView: View = activity.findViewById(android.R.id.content)
    private var isKeyboardVisible: Boolean = false

    init {
        rootView.viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
            private val r = Rect()

            override fun onGlobalLayout() {
                rootView.getWindowVisibleDisplayFrame(r)
                val screenHeight = rootView.rootView.height
                val keyboardHeight = screenHeight - r.bottom

                if (keyboardHeight > screenHeight * 0.15) {
                    if (!isKeyboardVisible) {
                        isKeyboardVisible = true
                        onKeyboardVisibilityChanged(true)
                    }
                } else {
                    if (isKeyboardVisible) {
                        isKeyboardVisible = false
                        onKeyboardVisibilityChanged(false)
                    }
                }
            }
        })
    }

    fun removeListener() {
        rootView.viewTreeObserver.removeOnGlobalLayoutListener(null)
    }
}