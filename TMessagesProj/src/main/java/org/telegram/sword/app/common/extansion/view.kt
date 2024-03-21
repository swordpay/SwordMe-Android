package org.telegram.sword.app.common.extansion

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Dialog
import android.content.ContentValues
import android.content.Context
import android.content.Intent
import android.graphics.*
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.Window
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.Toast
import androidx.appcompat.widget.AppCompatEditText
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.core.content.ContextCompat
import androidx.core.content.FileProvider
import androidx.core.graphics.drawable.toBitmap
import androidx.core.view.drawToBitmap
import androidx.fragment.app.Fragment
import androidx.print.PrintHelper
import org.telegram.messenger.R
import org.telegram.sword.app.common.base.appResources
import java.io.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream


fun View.hide(){
    this.visibility= View.INVISIBLE
}
fun View.show(){
    this.visibility= View.VISIBLE
}
fun View.gone(){
    this.visibility= View.GONE
}

fun View.show(isShow:Boolean){

    if (isShow)  this.visibility = View.VISIBLE else this.visibility= View.GONE

}


fun AppCompatTextView.enableGradient(){
    val width = paint.measureText(this.text.toString())
    val textShader: Shader = LinearGradient(0f, 0f, width, this.textSize, intArrayOf(
        Color.parseColor("#041B7A"),
        Color.parseColor("#041B7A"),
        Color.parseColor("#041B7A"),
        Color.parseColor("#041B7A"),
        Color.parseColor("#041B7A")
    ), null, Shader.TileMode.REPEAT)
    this.paint.shader = textShader
}

fun Activity.loading(): Dialog {
    val dialog = Dialog(this)
    dialog.run {
        requestWindowFeature(Window.FEATURE_NO_TITLE)
        window?.setBackgroundDrawableResource(android.R.color.transparent)
        setCancelable(false)
        setContentView(R.layout.loading_view)
    }
    return dialog
}
fun Fragment.loading(): Dialog {
    val dialog = Dialog(requireContext())
    dialog.run {
        requestWindowFeature(Window.FEATURE_NO_TITLE)
        window?.setBackgroundDrawableResource(android.R.color.transparent)
        setCancelable(false)
        setContentView(R.layout.loading_view)
    }
    return dialog
}

@SuppressLint("ClickableViewAccessibility")
fun AppCompatEditText.enableVerticalScroll(){
    this.setOnTouchListener { v, event ->
        v.parent.requestDisallowInterceptTouchEvent(true)
        when (event.action and MotionEvent.ACTION_MASK) {
            MotionEvent.ACTION_UP ->
                v.parent.requestDisallowInterceptTouchEvent(false)
        }
        false
    }
}

fun AppCompatImageView.setHintColor(context: Context, color:Int){
    this.setColorFilter(ContextCompat.getColor(context, color), android.graphics.PorterDuff.Mode.SRC_IN)

}

fun Activity.disableDoubleClick(button:View){

    button.isEnabled = false

    window.decorView.rootView.delayOnLifecycle(1000L) {

      button.isEnabled = true
    }
}


 fun AppCompatImageView.sharedImage(context: Context) {

     try {
         val intent = Intent(Intent.ACTION_SEND).setType("image/*")

         val bitmap = this.drawable.toBitmap()

         val bytes = ByteArrayOutputStream()

         bitmap.compress(Bitmap.CompressFormat.JPEG, 100, bytes)

         val path = MediaStore.Images.Media.insertImage(context.contentResolver, bitmap, "QrCode", null)

         val uri = Uri.parse(path)

         intent.putExtra(Intent.EXTRA_STREAM, uri)

             context.startActivity(Intent.createChooser(intent, "Share Qr using"))

     }catch (e:Exception){Unit}


}

fun LinearLayoutCompat.sharedQR(context: Context,isViaEmail:Boolean = false) {

     try {

         val intent = Intent(Intent.ACTION_SEND).setType("image/*")

         val view = this

         val bitmap = Bitmap.createBitmap(view.width, view.rootView.height, Bitmap.Config.ARGB_8888)

         val sharedBitmap =  view.drawToBitmap(bitmap.config)

         shareImage(sharedBitmap,context,isViaEmail = isViaEmail)

//
//         val uri = Uri.parse(path)
//
//         intent.putExtra(Intent.EXTRA_STREAM, uri)
//
//             context.startActivity(Intent.createChooser(intent, "Share Qr using"))

     }catch (e:Exception){ Unit }


}


fun LinearLayoutCompat.saveImage(context: Context,loading:Dialog) {

     try {

         val view = this

         val bitmap = Bitmap.createBitmap(view.width, view.rootView.height, Bitmap.Config.ARGB_8888)

         val sharedBitmap =  view.drawToBitmap(bitmap.config)

         saveBitmapImage(sharedBitmap,context,loading = loading)


     }catch (_:Exception){

         loading.dismiss()
     }


}

private fun saveBitmapImage(bitmap: Bitmap,context: Context,loading:Dialog) {
    val timestamp = System.currentTimeMillis()

    val values = ContentValues()
    values.put(MediaStore.Images.Media.MIME_TYPE, "image/png")
    values.put(MediaStore.Images.Media.DATE_ADDED, timestamp)
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
        values.put(MediaStore.Images.Media.DATE_TAKEN, timestamp)
        values.put(MediaStore.Images.Media.RELATIVE_PATH, "Pictures/" + appResources?.getString(R.string.App_Name))
        values.put(MediaStore.Images.Media.IS_PENDING, true)
        val uri = context.contentResolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values)
        if (uri != null) {
            try {
                val outputStream = context.contentResolver.openOutputStream(uri)
                if (outputStream != null) {
                    try {
                        bitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream)

                        outputStream.close()

                    } catch (e: Exception) {

                    }
                }
                values.put(MediaStore.Images.Media.IS_PENDING, false)
                context.contentResolver.update(uri, values, null, null)

                Toast.makeText(context, appResources?.getString(R.string.imageSaveSuccessfully), Toast.LENGTH_LONG).also { t->
                    t.setGravity(Gravity.TOP or Gravity.LEFT , 0, 0)
                    t.show()

                }


            } catch (e: Exception) {

            }
        }
    } else {
        val imageFileFolder = File(Environment.getExternalStorageDirectory().toString() + '/' + appResources?.getString(R.string.App_Name))
        if (!imageFileFolder.exists()) {
            imageFileFolder.mkdirs()
        }
        val mImageName = "$timestamp.png"
        val imageFile = File(imageFileFolder, mImageName)
        try {
            val outputStream: OutputStream = FileOutputStream(imageFile)
            try {
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream)
                outputStream.close()
            } catch (e: Exception) {

            }
            values.put(MediaStore.Images.Media.DATA, imageFile.absolutePath)
            context.contentResolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values)

            Toast.makeText(context, appResources?.getString(R.string.imageSaveSuccessfully), Toast.LENGTH_LONG).also { t->
                t.setGravity(Gravity.TOP or Gravity.LEFT, 0, 0)
                t.show()

            }
        } catch (e: Exception) {

        }
    }

}




private fun shareImage(bitmap: Bitmap,context: Context,isViaEmail:Boolean = false) {

    try {
        val cachePath = File(context.cacheDir, "images")

        cachePath.mkdirs()

        val stream = FileOutputStream("$cachePath/image.png")

        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, stream)

        stream.close()

    } catch (e: IOException) {

        e.printStackTrace()
    }

    val imagePath = File(context.cacheDir, "images")

    val newFile = File(imagePath, "image.png")

    val contentUri = FileProvider.getUriForFile(context, "com.swordpay.me.provider", newFile)

    if (contentUri != null) {

        val shareIntent = Intent()

        shareIntent.action = Intent.ACTION_SEND

        shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)

        shareIntent.setDataAndType(contentUri, context.contentResolver.getType(contentUri))

        shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri)

        shareIntent.type = "image/png"

        if (isViaEmail){

          shareIntent.setPackage("com.google.android.gm")
        }

        context.startActivity(Intent.createChooser(shareIntent, "Choose an app"))
    }
}

fun LinearLayoutCompat.printQr(context: Context) {

     try {

         val printHelper = PrintHelper(context)

         printHelper.scaleMode = PrintHelper.SCALE_MODE_FIT

         Intent(Intent.ACTION_SEND).type = "image/*"

         val view = this

         val bitmap = Bitmap.createBitmap(view.width, view.rootView.height, Bitmap.Config.ARGB_8888)

         val printBitmap =  view.drawToBitmap(bitmap.config)

         printHelper.printBitmap("My Qr", printBitmap)

     }catch (e:Exception){Unit}


}

@SuppressLint("SetJavaScriptEnabled")
fun WebView.loadWebView(url:String){

    settings.loadsImagesAutomatically = true

    settings.javaScriptEnabled = true

    scrollBarStyle = View.SCROLLBARS_INSIDE_OVERLAY

    webViewClient = WebViewClient()

    settings.useWideViewPort = true

    settings.loadWithOverviewMode = true

    settings.setSupportZoom(true)

    settings.builtInZoomControls = true

    settings.displayZoomControls = false

    loadUrl(url)
}
@SuppressLint("SetJavaScriptEnabled")
fun WebView.loadDocWebView(filePath:String){

    settings.loadsImagesAutomatically = true

    settings.javaScriptEnabled = true

    scrollBarStyle = View.SCROLLBARS_INSIDE_OVERLAY

    webViewClient = WebViewClient()

    settings.useWideViewPort = true

    settings.loadWithOverviewMode = true

    settings.setSupportZoom(true)

    settings.builtInZoomControls = true

    settings.displayZoomControls = false

    loadUrl(filePath)
}



private fun convertXlsxToHtml(filePath: String): String {
    val stringBuilder = StringBuilder()
    try {
        val fis = FileInputStream(File(filePath))
        val zis = ZipInputStream(fis)
        var entry: ZipEntry?

        while (zis.nextEntry.also { entry = it } != null) {
            if (entry?.name?.endsWith(".xml") == true) {
                val outputStream = ByteArrayOutputStream()
                val buffer = ByteArray(1024)
                var len: Int
                while (zis.read(buffer).also { len = it } > 0) {
                    outputStream.write(buffer, 0, len)
                }
                val xmlContent = outputStream.toString("UTF-8")
                stringBuilder.append(xmlContent)
                outputStream.close()
            }
        }
        zis.close()
        fis.close()
    } catch (e: Exception) {
        e.printStackTrace()
    }
    return stringBuilder.toString()
}





