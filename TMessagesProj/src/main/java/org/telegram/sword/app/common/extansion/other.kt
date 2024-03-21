package org.telegram.sword.app.common.extansion

import android.app.Activity
import android.content.Context
import android.database.Cursor
import android.net.Uri
import android.provider.MediaStore
import android.view.View
import android.view.inputmethod.InputMethodManager
import androidx.appcompat.widget.AppCompatEditText
import androidx.lifecycle.coroutineScope
import androidx.lifecycle.findViewTreeLifecycleOwner
import kotlinx.coroutines.*
import org.telegram.sword.app.common.AppConst.EMPTY
import java.text.SimpleDateFormat
import java.util.*

fun Uri?.getRealPathFromURI(context: Context): String {
    var path = EMPTY
    val proj = arrayOf(MediaStore.Images.Media.DATA)
    if (context.contentResolver != null) {
        try {
            val cursor: Cursor? =
                this?.let { context.contentResolver.query(it, proj, null, null, null) }
            if (cursor != null) {
                cursor.moveToFirst()
                val idx = cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA)
                path = cursor.getString(idx)
                cursor.close()
            }
        }catch (e:Exception){

        }

    }
    return path
}

fun Context.hideKeyboard(view: View) {
    val inputMethodManager = getSystemService(Activity.INPUT_METHOD_SERVICE) as InputMethodManager
    inputMethodManager.hideSoftInputFromWindow(view.windowToken, 0)
}
fun Context.showKeyboard() {
    val inputMethodManager: InputMethodManager =
        getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    inputMethodManager.toggleSoftInput(0, InputMethodManager.SHOW_FORCED)
}

fun showKeyboard(view: View?) {
    if (view == null) return
    view.requestFocus()
    val imm = view.context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    if (view is AppCompatEditText) imm.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT)
    else imm.toggleSoftInput(
        InputMethodManager.SHOW_FORCED,
        InputMethodManager.HIDE_IMPLICIT_ONLY
    )
}


fun View.delayOnLifecycle(
    durationInMillis: Long,
    dispatcher:CoroutineDispatcher = Dispatchers.Main,
    block:()->Unit
):Job?=findViewTreeLifecycleOwner()?.let { lifecycleOwner ->
    lifecycleOwner.lifecycle.coroutineScope.launch(dispatcher){
        delay(durationInMillis)
        block()
    }
}

fun String.extractUrlsFromString(): List<String> {


    val regex = Regex("""\bhttps?://\S+\b""")


    val matches = regex.findAll(this)


    return matches.map { it.value }.toList()
}

fun List<String>.getFirstCorrectImageUrl():String{

    var firstCorrectImageUrl = EMPTY

    run blocking@{

        this.forEach{

            if (it.lowercase().endsWith(".gif") ||

                it.lowercase().endsWith(".jpg") ||

                it.lowercase().endsWith(".png") ||

                it.lowercase().endsWith(".jpeg")){


                firstCorrectImageUrl = it

                return@blocking

            }

        }

    }

    return  firstCorrectImageUrl

}
fun <T> debounce(
    delayMillis: Long = 1000L,
    scope: CoroutineScope,
    action: (T) -> Unit
): (T) -> Unit {
    var debounceJob: Job? = null
    return { param: T ->
        if (debounceJob == null) {
            debounceJob = scope.launch {
                action(param)
                delay(delayMillis)
                debounceJob = null
            }
        }
    }
}


fun convertLongToDateString(time: Long): String {
    val date = Date(time)
    val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault())
    sdf.timeZone = TimeZone.getDefault()

    return sdf.format(date)
}


 fun <T> List<T>.seconds(): T {
    if (isEmpty())
        throw NoSuchElementException("List is empty.")
    return this[1]
}

