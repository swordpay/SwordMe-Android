package org.telegram.sword.domain.chat.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class FederationTokenResponseModel(

    @SerializedName("data") val data : FederationData?=null
) : Parcelable
@Parcelize
data class FederationData (

    @SerializedName("accessKeyId") val accessKey : String?=null,
    @SerializedName("secretAccessKey") val secretKey : String?=null,
    @SerializedName("sessionToken") val sessionToken : String?=null,
    @SerializedName("expiration") val expiration : String?=null,
    @SerializedName("bucket") val bucket : String?=null,
    @SerializedName("customerFolder") val customerFolder : String?=null
) : Parcelable