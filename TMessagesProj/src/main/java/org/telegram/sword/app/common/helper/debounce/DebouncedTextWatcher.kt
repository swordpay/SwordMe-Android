package org.telegram.sword.app.common.helper.debounce

import android.os.Handler
import android.text.Editable
import android.text.TextWatcher

class DebouncedTextWatcher(private val debounceInterval: Long = 500, private val onDebouncedTextChanged: (String) -> Unit) :
    TextWatcher {
    private val handler = Handler()
    private var runnable: Runnable? = null

    override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {

    }

    override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
        runnable?.let { handler.removeCallbacks(it) }
    }

    override fun afterTextChanged(editable: Editable?) {
        runnable = Runnable {
            onDebouncedTextChanged.invoke(editable.toString())
        }
        handler.postDelayed(runnable!!, debounceInterval)
    }
}