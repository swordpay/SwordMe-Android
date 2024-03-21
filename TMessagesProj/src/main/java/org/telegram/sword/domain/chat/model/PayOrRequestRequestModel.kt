package org.telegram.sword.domain.chat.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import java.io.Serializable

@Parcelize
data class PayOrRequestRequestModel(


 @SerializedName("transactionType") val transactionType : String?=null,
@SerializedName("peers") val peers : ArrayList<Peers>?=null,
@SerializedName("note") val note : String?=null,
@SerializedName("type") val type : String?=null,
@SerializedName("amountType") val amountType : String?=null,
@SerializedName("F_") val F_ : F_?=null,
@SerializedName("currencyType") val currencyType : String?=null

) : Parcelable, Serializable

@Parcelize
data class Peers (
    @SerializedName("peerId") val peerId : Long?=null,
    @SerializedName("extraPeerId") val extraPeerId : Long?=null,
    @SerializedName("accessHash") val accessHash : String?=null,
    @SerializedName("title") val title : String?=null,
    @SerializedName("type") val type : String?=null,
    @SerializedName("amount") val amount : Double?=null

) : Parcelable, Serializable

@Parcelize
data class F_ (

    @SerializedName("timer") val timer : Long?=null,
    @SerializedName("maxViews") val maxViews : Long?=null,
    @SerializedName("path") val path : String?=null

) : Parcelable, Serializable
