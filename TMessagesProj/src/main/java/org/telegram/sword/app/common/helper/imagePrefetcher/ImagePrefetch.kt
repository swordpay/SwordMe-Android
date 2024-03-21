package org.telegram.sword.app.common.helper.imagePrefetcher

import android.content.Context
import com.bumptech.glide.Glide
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.bumptech.glide.request.FutureTarget

class ImagePrefetch(private val context: Context) {
    fun prefetchImage(url: String) {
        val futureTarget: FutureTarget<*> = Glide.with(context)
            .downloadOnly()
            .load(url)
            .diskCacheStrategy(DiskCacheStrategy.ALL)
            .submit()
    }
}






