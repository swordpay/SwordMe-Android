package org.telegram.sword.app.common.extansion

import android.content.Context
import android.content.SharedPreferences
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.ColorMatrix
import android.graphics.ColorMatrixColorFilter
import android.graphics.drawable.Drawable
import android.preference.PreferenceManager
import android.util.Base64
import androidx.appcompat.widget.AppCompatImageView
import coil.ImageLoader
import coil.decode.SvgDecoder
import coil.request.CachePolicy
import coil.request.ImageRequest
import com.bumptech.glide.GenericTransitionOptions
import com.bumptech.glide.Glide
import com.bumptech.glide.load.DataSource
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.bumptech.glide.load.engine.GlideException
import com.bumptech.glide.load.resource.gif.GifDrawable
import com.bumptech.glide.request.RequestListener
import com.bumptech.glide.request.RequestOptions
import com.bumptech.glide.request.target.CustomTarget
import com.bumptech.glide.request.target.SimpleTarget
import com.bumptech.glide.request.target.Target
import com.bumptech.glide.request.transition.Transition
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.makeramen.roundedimageview.RoundedImageView
import okhttp3.*
import okio.IOException
import org.telegram.messenger.R
import org.telegram.sword.app.common.helper.imagePrefetcher.ImagePrefetch
import org.telegram.sword.domain.common.BaseUrl.CRYPTO_IMAGE_URL
import java.io.ByteArrayOutputStream


fun RoundedImageView.loadImage(
    imageUrl: String?,
    errorImage: Int? = null,
    skipMemoryCache: Boolean = false
) {

    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)

    Glide.with(this)
        .load(imageUrl)
        .thumbnail(0.05f)
        .error(errorImage ?: R.drawable.gray_oval_fon)
        .apply(options)
        .placeholder(R.drawable.gray_oval_fon)
        .into(this)

}
fun RoundedImageView.loadImageWhitAlphaAnimation(
    imageUrl: String?,
    errorImage: Int? = null,
    skipMemoryCache: Boolean = false
) {

    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)

    Glide.with(this)
        .load(imageUrl)
        .error(errorImage ?: R.drawable.gray_oval_fon)
        .apply(options)
        .transition(GenericTransitionOptions.with(R.anim.alpha_animation))
        .into(this)

}

fun RoundedImageView.loadCryptoImage(coin: String) {
    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)

    Glide.with(this)
        .load("$CRYPTO_IMAGE_URL$coin.png")
        .error(R.drawable.crypto_default_icon)
        .apply(options)
        .placeholder(R.drawable.dark_gray_oval)
        .transition(GenericTransitionOptions.with(R.anim.alpha_animation))
        .into(this)

}


fun BottomNavigationView.loadTabIcon(url: String, tabIndex: Int) {
    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)
    Glide.with(this)
        .load(url)
        .thumbnail(0.15f)
        .error(R.drawable.user)
        .apply(RequestOptions.circleCropTransform())
        .apply(options)
        .placeholder(R.drawable.user)
        .into(object : SimpleTarget<Drawable>() {
            override fun onResourceReady(
                resource: Drawable,
                transition: com.bumptech.glide.request.transition.Transition<in Drawable>?
            ) {
                this@loadTabIcon.menu.getItem(tabIndex).icon = resource
            }


        })
}

fun RoundedImageView.loadSvg(imageUrl: String?,loadImageError :((onError:Boolean) -> Unit)? = null) {

    imageUrl?.let {
        if (it.lowercase().endsWith("svg")) {
            val imageLoader = ImageLoader.Builder(this.context)
                .componentRegistry {
                    add(SvgDecoder(this@loadSvg.context))
                }.build()

            val request = ImageRequest.Builder(this.context).apply {
                memoryCachePolicy(CachePolicy.ENABLED)
                listener(onError = { _, _ ->

                    loadImageError?.invoke(true)

                    this@loadSvg.gone()
                },
                    onSuccess = { _, _ ->

                       this@loadSvg.show()
                       loadImageError?.invoke(false)
                    }

                )
                placeholder(R.drawable.gray_oval_fon)
                data(it).decoder(SvgDecoder(this@loadSvg.context))
            }.target(this).build()

            imageLoader.enqueue(request)
        } else {
            val imageLoader = ImageLoader(context)
            val request = ImageRequest.Builder(context).apply {
                memoryCachePolicy(CachePolicy.ENABLED)
                error(R.drawable.image_failure_image)
                placeholder(R.drawable.image_failure_image)
                data("$it")
            }.target(this).build()

            imageLoader.enqueue(request)
        }
    }
}




fun AppCompatImageView.loadImage(imageUrl: String?, errorImage: Int? = null) {
    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)
    Glide.with(this)
        .load(imageUrl)
        .thumbnail(0.15f)
        .error(errorImage)
        .apply(options)
        .into(this)

}

fun AppCompatImageView.loadImageWhitAlphaAnimation(imageUrl: String?, errorImage: Int? = null) {
    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)
    Glide.with(this)
        .load(imageUrl)
        .error(errorImage)
        .apply(options)
        .transition(GenericTransitionOptions.with(R.anim.alpha_animation))
        .into(this)

}
fun AppCompatImageView.setColorToBlackWhite() {
    val matrix = ColorMatrix()
    matrix.setSaturation(0f)
    val filter = ColorMatrixColorFilter(matrix)
    this.colorFilter = filter
}

fun AppCompatImageView.setBlackWhiteToColor() {
    val matrix = ColorMatrix()
    matrix.setSaturation(1f)
    val filter = ColorMatrixColorFilter(matrix)
    this.colorFilter = filter

}

fun RoundedImageView.setColorToBlackWhite() {
    val matrix = ColorMatrix()
    matrix.setSaturation(0f)
    val filter = ColorMatrixColorFilter(matrix)
    this.colorFilter = filter
}

fun RoundedImageView.setBlackWhiteToColor() {
    val matrix = ColorMatrix()
    matrix.setSaturation(1f)
    val filter = ColorMatrixColorFilter(matrix)
    this.colorFilter = filter

}

fun AppCompatImageView.loadImageAndSaveLocalDb(
    imageUrl: String?,
    errorImage: Int? = null,
    fileId: String,
    loadImageCallBack: ((isSuccessLoad: Boolean) -> Unit)? = null
) {
    val options = RequestOptions().diskCacheStrategy(DiskCacheStrategy.ALL)
    val imageView: AppCompatImageView = this;
    Glide.with(this)
        .asBitmap()
        .load(imageUrl)
        .thumbnail(0.15f)
        .error(R.drawable.placeholder_image)
        .apply(options)
        .timeout(120000)
        .placeholder(R.drawable.placeholder_image)
        .into(object : CustomTarget<Bitmap?>() {

            override fun onLoadCleared(placeholder: Drawable?) {}

            override fun onLoadFailed(errorDrawable: Drawable?) {
                loadImageCallBack?.invoke(false)
            }

            override fun onResourceReady(resource: Bitmap, transition: Transition<in Bitmap?>?) {
                val baos = ByteArrayOutputStream()
                resource.compress(Bitmap.CompressFormat.JPEG, 100, baos)
                val b = baos.toByteArray()
                val encodedImage = Base64.encodeToString(b, Base64.DEFAULT)
                val shre = PreferenceManager.getDefaultSharedPreferences(context)
                val edit = shre.edit()
                edit.putString(fileId, encodedImage)
                edit.commit()

                imageView.setImageBitmap(resource)

                loadImageCallBack?.invoke(true)
            }
        })
}

fun AppCompatImageView.loadSvg(imageUrl: String?,loadImageCallBack: ((isSuccessLoad: Boolean) -> Unit)? = null) {
//    GlideToVectorYou
//        .init()
//        ?.with(this.context)
//        ?.withListener(object : GlideToVectorYouListener {
//            override fun onLoadFailed() {
//                loadImageCallBack?.invoke(false)
//            }
//
//            override fun onResourceReady() {
//                loadImageCallBack?.invoke(true)
//            }
//
//        })
//        ?.load(Uri.parse(imageUrl), this)
}

fun AppCompatImageView.loadGif(imageUrl: String?,loadImageCallBack: ((isSuccessLoad: Boolean) -> Unit)? = null) {
    Glide.with(this)
        .asGif()
        .load(imageUrl)
        .listener(
            object : RequestListener<GifDrawable> {
                override fun onLoadFailed(
                    e: GlideException?,
                    model: Any?,
                    target: Target<GifDrawable>?,
                    isFirstResource: Boolean
                ): Boolean {

                    loadImageCallBack?.invoke(false)

                    return false
                }

                override fun onResourceReady(
                    resource: GifDrawable?,
                    model: Any?,
                    target: Target<GifDrawable>?,
                    dataSource: DataSource?,
                    isFirstResource: Boolean
                ): Boolean {

                    loadImageCallBack?.invoke(true)
                    return false
                }


            },
        )
        .into(this)
}



fun AppCompatImageView.getImageFromDb(
    imageUrl: String?,
    fileId: String,
    loadImageCallBack: ((isSuccessLoad: Boolean) -> Unit)? = null
) {

    val shrew = PreferenceManager.getDefaultSharedPreferences(context)
    val previouslyEncodedImage = shrew.getString(fileId, "")

    if (!previouslyEncodedImage.equals("", ignoreCase = true)) {
        val bu = Base64.decode(previouslyEncodedImage, Base64.DEFAULT)
        val bitmap = BitmapFactory.decodeByteArray(bu, 0, bu.size)
        this.setImageBitmap(bitmap)
        loadImageCallBack?.invoke(true)
    } else {

        this.loadImageAndSaveLocalDb(
            imageUrl = imageUrl,
            fileId = fileId,
            loadImageCallBack = loadImageCallBack
        )

    }

}


fun getFdfFromDb(
    pdfUrl: String,
    fileId: String,
    context: Context,
    loadPdfCallBack: ((pdf: ByteArray, isSuccessLoad: Boolean) -> Unit)? = null
) {

    var pdfBytes: ByteArray = getPdfFromLocalDb(fileId, context)

    if (pdfBytes.isNotEmpty()) {

        loadPdfCallBack?.invoke(pdfBytes, true)

    } else {

        val request = Request.Builder()
            .url(pdfUrl)
            .build()
        val client = OkHttpClient.Builder()
            .build()


        client.newCall(request).enqueue(object : Callback {

            override fun onResponse(call: Call, response: Response) {
                val pdfData = response.body.bytes()
                if (pdfData.isNotEmpty()) {
                    val encodedImage = Base64.encodeToString(pdfData, Base64.DEFAULT)

                    val shre = PreferenceManager.getDefaultSharedPreferences(context)
                    val edit: SharedPreferences.Editor = shre.edit()
                    edit.putString(fileId, encodedImage)
                    edit.commit()
                    pdfBytes = getPdfFromLocalDb(fileId, context)

                    if (pdfBytes.isNotEmpty()) {
                        loadPdfCallBack?.invoke(pdfBytes, true)

                    } else {
                        loadPdfCallBack?.invoke(pdfBytes, false)
                    }
                }
            }

            override fun onFailure(call: Call, e: IOException) {

                loadPdfCallBack?.invoke(pdfBytes, false)
            }
        })

    }

}

fun getPdfFromLocalDb(fileId: String, context: Context): ByteArray {

    val shre = PreferenceManager.getDefaultSharedPreferences(context)
    val previouslyEncodedImage = shre.getString(fileId, "")

    return if (!previouslyEncodedImage.equals("", ignoreCase = true)) {

        Base64.decode(previouslyEncodedImage, Base64.DEFAULT)

    } else {

        byteArrayOf()
    }

}

fun getVideoFromDb(
    videoUrl: String,
    fileId: String,
    context: Context,
    loadVideoCallBack: ((video: ByteArray, isSuccessLoad: Boolean) -> Unit)? = null
) {

    var videoBytes: ByteArray = getVideoFromLocalDb(fileId, context)

    if (videoBytes.isNotEmpty()) {

        loadVideoCallBack?.invoke(videoBytes, true)

    } else {

        val request = Request.Builder()
            .url(videoUrl)
            .build()
        val client = OkHttpClient.Builder()
            .build()


        client.newCall(request).enqueue(object : Callback {

            override fun onResponse(call: Call, response: Response) {
                val pdfData = response.body.bytes()
                if (pdfData.isNotEmpty()) {
                    val encodedVideo = Base64.encodeToString(pdfData, Base64.DEFAULT)

                    val shre = PreferenceManager.getDefaultSharedPreferences(context)
                    val edit: SharedPreferences.Editor = shre.edit()
                    edit.putString(fileId, encodedVideo)
                    edit.commit()
                    videoBytes = getVideoFromLocalDb(fileId, context)


                    if (videoBytes.isNotEmpty()) {
                        loadVideoCallBack?.invoke(videoBytes, true)

                    } else {
                        loadVideoCallBack?.invoke(videoBytes, false)
                    }
                }
            }

            override fun onFailure(call: Call, e: IOException) {

            }
        })

    }
}

fun getVideoFromLocalDb(fileId: String, context: Context): ByteArray {

    val shre = PreferenceManager.getDefaultSharedPreferences(context)
    val previouslyEncodedVideo = shre.getString(fileId, "")

    return if (!previouslyEncodedVideo.equals("", ignoreCase = true)) {

        Base64.decode(previouslyEncodedVideo, Base64.DEFAULT)

    } else {

        byteArrayOf()
    }

}


fun Context.prefetchImages(images: ArrayList<String>) {

    val imagePrefetch = ImagePrefetch(this)

    images.forEach {

        imagePrefetch.prefetchImage(it)
    }

}

