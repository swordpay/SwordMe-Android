

package org.telegram.customqrgenerator.style


import android.graphics.Canvas
import android.graphics.ColorFilter
import android.graphics.PixelFormat
import android.graphics.drawable.Drawable

//
//    fun get(context: Context) : Drawable
//
//
//    object Empty : DrawableSource {
//        override fun get(context: Context): Drawable = EmptyDrawable
//    }
//
//    /**
//     * Load image from resources.
//     * */
//
//    data class Resource(@DrawableRes val id : Int) : DrawableSource {
//        override fun get(context: Context): Drawable =
//            requireNotNull(ContextCompat.getDrawable(context, id))
//    }
//
//    /**
//     * Load image from file system. App must have permission to read this file
//     * */
//
//    @Suppress("deprecation")
//    data class File(val uri : String) : DrawableSource {
//
//        override fun get(context: Context): Drawable =
//            if (Build.VERSION.SDK_INT < 28)
//                MediaStore.Images.Media
//                    .getBitmap(context.contentResolver, uri.toUri())
//                    .copy(Bitmap.Config.ARGB_8888, false)
//                    .toDrawable(context.resources)
//            else ImageDecoder
//                .decodeBitmap(ImageDecoder.createSource(context.contentResolver, uri.toUri()))
//                .copy(Bitmap.Config.ARGB_8888, false)
//                .toDrawable(context.resources)
//    }
//
//    class Custom(val drawable: Drawable) : DrawableSource {
//        override fun get(context: Context): Drawable = drawable
//    }
//}

internal object EmptyDrawable : Drawable() {
    override fun draw(canvas: Canvas) = Unit
    override fun setAlpha(alpha: Int)  = Unit
    override fun setColorFilter(colorFilter: ColorFilter?) = Unit
    override fun getOpacity(): Int = PixelFormat.TRANSPARENT
}