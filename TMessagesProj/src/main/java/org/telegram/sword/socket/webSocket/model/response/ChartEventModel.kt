package org.telegram.sword.socket.webSocket.model.response

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class ChartEventModel (

    @SerializedName("stream") val stream : String,
    @SerializedName("data") val data : Chart

 ) : Parcelable

@Parcelize
data class Chart (

    @SerializedName("e") val eventType : String,
    @SerializedName("E") val eventTime : Long,
    @SerializedName("s") val symbol: String,
    @SerializedName("k") val chartInfo : ChartInfo
) : Parcelable

@Parcelize
data class ChartInfo (

    @SerializedName("t") val klineStartTime : Long,
    @SerializedName("T") val klineCloseTime : Long,
    @SerializedName("s") val symbol : String,
    @SerializedName("i") val interval : String,
    @SerializedName("f") val firstTradeID : Long,
    @SerializedName("L") val lastTradeID : Long,
    @SerializedName("o") val openPrice : Double,
    @SerializedName("c") val closePrice : Double,
    @SerializedName("h") val highPrice : Double,
    @SerializedName("l") val lowPrice : Double,
    @SerializedName("v") val baseAssetVolume : Double,
    @SerializedName("n") val numberOfTrades : Int,
    @SerializedName("x") val isThisKlineClosed : Boolean,
    @SerializedName("q") val quoteAssetVolume : Double,
    @SerializedName("V") val takerBuyBaseAssetVolume : Double,
    @SerializedName("Q") val takerBuyQuoteAssetVolume : Double,
    @SerializedName("B") val ignore : Long
) : Parcelable