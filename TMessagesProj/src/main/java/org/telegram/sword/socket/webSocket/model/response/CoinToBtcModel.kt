package org.telegram.sword.socket.webSocket.model.response

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class CoinToBtcModel(

    @SerializedName("stream") val stream : String?=null,
    @SerializedName("data") val data : Data?=null

) : Parcelable
@Parcelize
data class Data (

    @SerializedName("e") val eventType : String,
    @SerializedName("E") val eventTime : Long,
    @SerializedName("s") val symbol : String,
    @SerializedName("a") val aggregateTradeID : Long,
    @SerializedName("p") val price : String,
    @SerializedName("q") val quantity : Double,
    @SerializedName("f") val firstTradeID : Long,
    @SerializedName("l") val lastTradeID : Long,
    @SerializedName("T") val tradeID : Long,
    @SerializedName("m") val isTheBuyerTheMarketMaker : Boolean,
    @SerializedName("M") val ignore : Boolean
) : Parcelable
