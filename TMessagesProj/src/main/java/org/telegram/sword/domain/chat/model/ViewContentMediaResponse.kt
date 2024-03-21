package org.telegram.sword.domain.chat.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
@Parcelize
data class ViewContentMediaResponse(

    @SerializedName("data") val data :ArrayList<ViewContentData> ?=null



): Parcelable

@Parcelize
data class ViewContentData(
    @SerializedName("path") val path :String ?=null,
    @SerializedName("extension") val extension :String ?=null,
    @SerializedName("thumb") val thumb :String ?=null,
    @SerializedName("seconds") val seconds :Long ?=null,
    @SerializedName("title") val title :String ?=null,
    @SerializedName("artist") val artist :String ?=null,
    @SerializedName("downloadable") val downloadable :Boolean ?=null,
    @SerializedName("timer") val timer :Int ?=null,
) : Parcelable
