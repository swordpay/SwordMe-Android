package org.telegram.sword.socket.webSocket.model.response

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class CryptoToCryptoEventModel(

    @SerializedName("stream") val stream : String?=null,
    @SerializedName("data") val data : PercentData?=null

) : Parcelable

@Parcelize
data class PercentData(

    @SerializedName("e") val eventType : String,
    @SerializedName("E") val eventTime : Long,
    @SerializedName("s") val symbol : String,
    @SerializedName("p") val priceChange : Double,
    @SerializedName("P") val priceChangePercent : Double,
    @SerializedName("w") val weightedAveragePercent : Double,
    @SerializedName("x") val firstTrade : Double,
    @SerializedName("c") val lastPrice : String,
    @SerializedName("Q") val lastQuantity : Double,
    @SerializedName("b") val bestBidPrice : Double,
    @SerializedName("B") val bestBidQuantity : Double,
    @SerializedName("a") val bestAskPrice : Double,
    @SerializedName("A") val bestAskQuantity : Double,
    @SerializedName("o") val openPrice : Double,
    @SerializedName("h") val highPrice : Double,
    @SerializedName("l") val lowPrice : Double,
    @SerializedName("v") val totalTradedBaseAssetVolume : Double,
    @SerializedName("q") val totalTradedQuoteAssetVolume : Double,
    @SerializedName("O") val statisticsOpenTime : Long,
    @SerializedName("C") val statisticsCloseTime : Long,
    @SerializedName("F") val firstTradeID : Long,
    @SerializedName("L") val lastTradeId : Long,
    @SerializedName("n") val totalNumberOfTrades : Long

) : Parcelable