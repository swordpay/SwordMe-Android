package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
@Parcelize
data class TradeInfoResponseModel(

    @SerializedName("info") val info : TradeInfo

): BaseResponse(), Parcelable

@Parcelize
data class TradeInfo (

    @SerializedName("coin") val coin : Coin,
    @SerializedName("currency") val currency : Currency
) : Parcelable

@Parcelize
data class Coin (

    @SerializedName("min") val min : String,
    @SerializedName("max") val max : String,
    @SerializedName("precision") val precision : Int?=null
) : Parcelable

@Parcelize
data class Currency (

    @SerializedName("min") val min : String,
    @SerializedName("max") val max : String,
    @SerializedName("precision") val precision : Int?=null
) : Parcelable