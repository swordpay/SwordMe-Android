package org.telegram.sword.domain.chat.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import java.io.Serializable

data class SendMessageRequestModel(

    @SerializedName("message") val message:String,
    @SerializedName("referenceId") var referenceId:String
)

data class RejectRequestModel(

    @SerializedName("referenceId") val referenceId:String
)
@Parcelize
data class RejectPayRequestModel(

    @SerializedName("peer") val peer : PeerInfo,
    @SerializedName("messageId") val messageId : Long
) : Parcelable, Serializable

@Parcelize
data class AcceptRequest(

    @SerializedName("payId") val payId : String? = null,
    @SerializedName("peer") val peer : PeerInfo,
    @SerializedName("messageId") val messageId : Int? = null,
    @SerializedName("amount") val amount : Double? = null,
    @SerializedName("note") val note : String? = null,
    @SerializedName("currencyType") val currencyType : String? = null,
    @SerializedName("transactionType") val transactionType : String? = null,


) : Parcelable, Serializable

@Parcelize
data class PeerInfo (

    @SerializedName("peerId") val peerId : Long,
    @SerializedName("extraPeerId") val extraPeerId : String,
    @SerializedName("accessHash") val accessHash : String,
    @SerializedName("title") val title : String,
    @SerializedName("type") val type : String
) : Parcelable, Serializable
