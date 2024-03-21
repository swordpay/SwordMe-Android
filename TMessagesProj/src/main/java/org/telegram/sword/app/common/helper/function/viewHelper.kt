package org.telegram.sword.app.common.helper.function

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Dialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.net.Uri
import android.view.View
import android.view.Window
import android.view.inputmethod.InputMethodManager
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.core.view.WindowInsetsCompat
import com.makeramen.roundedimageview.RoundedImageView
//import kotlinx.android.synthetic.main.dialog_message.*
import okhttp3.MediaType.Companion.toMediaTypeOrNull
import okhttp3.RequestBody
import okhttp3.RequestBody.Companion.toRequestBody
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.ACCESS_TOKEN
import org.telegram.sword.app.common.AppConst.Key.IF_GO_TO_SAVED_MESSAGE
import org.telegram.sword.app.common.AppConst.Key.IMMORTAL_TOKEN
import org.telegram.sword.app.common.AppConst.Key.PHONE_NUMBER
import org.telegram.sword.app.common.AppConst.Key.REFRESH_TOKEN
import org.telegram.sword.app.common.AppConst.SPACE
import org.telegram.sword.app.common.extansion.*


fun enableBorderButton(button: LinearLayoutCompat, enable: Boolean) {
    button.run {
        if (enable) {
            setBackgroundResource(R.drawable.border_btn)
        } else {
            setBackgroundResource(R.drawable.inactive_border_btn)
        }
        isEnabled = enable
    }
}

fun space(count:Int):String{
    var space = SPACE

    for (i in 0 until count){
        space += SPACE
    }
    return space
}

fun createPartFromString(stringData: String): RequestBody {
    return stringData.toRequestBody("text/plain".toMediaTypeOrNull())
}

fun updateAccessToken(token:String,context: Context){
    context.putShared(key = ACCESS_TOKEN, value = token)
}
fun updateRefreshToken(token:String,context: Context){
    context.putShared(key = REFRESH_TOKEN,value = token)
}

fun updatePhoneNumber(phone:String,context: Context){
    context.putShared(key = PHONE_NUMBER,value = phone)
}

fun goToSavedMessage(isGoToSavedMessage:Boolean,context: Context){
    context.putSharedBul(key = IF_GO_TO_SAVED_MESSAGE,value = isGoToSavedMessage)
}

fun ifGoToSavedMessage(context: Context) = context.getSharedBul(key = IF_GO_TO_SAVED_MESSAGE)

fun getAccessToken(context: Context) = context.getShared(key = ACCESS_TOKEN)

fun getRefreshToken(context: Context)= context.getShared(key = REFRESH_TOKEN)

fun getPhoneNumber(context: Context)= context.getShared(key = PHONE_NUMBER)


fun updateImmortalToken(token:String,context: Context){
    context.putShared(key = IMMORTAL_TOKEN,value = token)
}

 fun isConnectedToInternet(context: Context): Boolean {

    val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

    val network = connectivityManager.activeNetwork ?: return false

    val activeNetwork = connectivityManager.getNetworkCapabilities(network) ?: return false

    return when {

        activeNetwork.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) -> true

        activeNetwork.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR) -> true

        else -> false
    }
}

var noInetDialogIsShow = false

@SuppressLint("WifiManagerLeak")
fun showNoInetDialog(context: Activity, function: (() -> Unit)? = null) {

    if (!noInetDialogIsShow){

        val dialog = Dialog(context)
        context.runOnUiThread {
            dialog.run {
                requestWindowFeature(Window.FEATURE_NO_TITLE)
                window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
                setCancelable(false)
                setContentView(R.layout.dialog_message)
                val btn = findViewById<AppCompatTextView>(R.id.dialogBtn)
                val dialogTitle = findViewById<TextView>(R.id.dialogTitle)
                val dialogDesc = findViewById<TextView>(R.id.dialogDesc)
                val dialogStatusIcon = findViewById<RoundedImageView>(R.id.messageDialogStatusIcon)

                btn.text = context.getString(R.string.tryAgain)
                dialogTitle.text = context.getString(R.string.noInet)
                dialogDesc.text = context.getString(R.string.pleaseCheckInet)
                dialogStatusIcon.setImageResource(R.drawable.wrong_dialog_icon)
                btn.setOnClickListener {

                    if (isConnectedToInternet(context)) {

                        noInetDialogIsShow = false

                        dialog.dismiss()

                        function?.invoke()
                    }
                }
                show()

                noInetDialogIsShow = true
            }
        }
    }


}

fun copyText(context: Activity,copyText:String,title:String = EMPTY){

    val clipboardManager = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager

    val clipData = ClipData.newPlainText("label", copyText)

    clipboardManager.setPrimaryClip(clipData)

    context.vibrate(duration = 80)

    if(title.isNotEmpty()){

        Toast.makeText(context, "$title copied", Toast.LENGTH_SHORT).show()
    }


}

fun addCardNumSpace(number:String):String{

    var cardNumber = EMPTY

    cardNumber =  number.replace(number[3].toString(),(number[3]+ SPACE))
    cardNumber =  number.replace(number[7].toString(),(number[7]+ SPACE))
    cardNumber =  number.replace(number[11].toString(),(number[11]+ SPACE))

    return cardNumber

}

fun showKeyboard(context: Context) {
    val inputMethodManager: InputMethodManager =
        context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    inputMethodManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0)
}

fun closeKeyboard(context: Context) {
    val inputMethodManager: InputMethodManager =
        context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    inputMethodManager.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0)
}

fun View.keyboardIsVisible() =
    WindowInsetsCompat
        .toWindowInsetsCompat(this.rootWindowInsets)
        .isVisible(WindowInsetsCompat.Type.ime())


fun getQueryValue(url: String, key:String):String = Uri.parse(url).getQueryParameter(key)?: EMPTY