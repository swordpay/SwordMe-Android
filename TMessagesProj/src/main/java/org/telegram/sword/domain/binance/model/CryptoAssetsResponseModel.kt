package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable
@Parcelize
data class CryptoAssetsResponseModel(

    @SerializedName("data") val data : Data

): BaseResponse(),Serializable, Parcelable

@Parcelize
data class Data (

    @SerializedName("self") val self : ArrayList<Self>,
    @SerializedName("main") val mainCoin : String,
    @SerializedName("top") val top : ArrayList<Top>,
    @SerializedName("available") val available : ArrayList<Available>
): Serializable, Parcelable

@Parcelize
data class Self (

    @SerializedName("ramp") val ramp : String?=null,
    @SerializedName("name") val name : String,
    @SerializedName("coin") val coin : String,
    @SerializedName("balance") val balance : String,
    @SerializedName("withdrawInfo") val WithdrawInfo : WithdrawInfo?=null,
    @SerializedName("depositInfo") val depositInfo:DepositInfo? = null,
    @SerializedName("networks") val networks:ArrayList<CryptoNetworkModel>,

): Serializable, Parcelable

@Parcelize
data class CryptoNetworkModel (

    @SerializedName("name") val name : String,
    @SerializedName("withdrawInfo") val WithdrawInfo : WithdrawInfo?=null,
    @SerializedName("depositInfo") val depositInfo:DepositInfo? = null,
    @SerializedName("rampInfo") val rampInfo:RampInfo? = null,
    @SerializedName("network") val network:String? = null,

): Serializable, Parcelable

@Parcelize
data class RampInfo (

    @SerializedName("id") val id : String?=null,
    @SerializedName("network") val network : String?=null,
    @SerializedName("name") val name:String?=null,
    @SerializedName("sellEnable") val sellEnable:Boolean?=null,

): Serializable, Parcelable


@Parcelize
data class Top (
    @SerializedName("ramp") val ramp : String?=null,
    @SerializedName("networks") val networks:ArrayList<CryptoNetworkModel>,
    @SerializedName("name") val name : String,
    @SerializedName("coin") val coin : String,
    @SerializedName("withdrawInfo") val WithdrawInfo : WithdrawInfo,
    @SerializedName("depositInfo") val depositInfo:DepositInfo? = null,
): Serializable, Parcelable

@Parcelize
data class Available (
    @SerializedName("ramp") val ramp : String?=null,
    @SerializedName("networks") val networks:ArrayList<CryptoNetworkModel>,
    @SerializedName("name") val name : String,
    @SerializedName("coin") val coin : String,
    @SerializedName("withdrawInfo") val WithdrawInfo : WithdrawInfo,
    @SerializedName("depositInfo") val depositInfo:DepositInfo? = null,
): Serializable, Parcelable

@Parcelize
data class DepositInfo(

    @SerializedName("depositEnable") val depositEnable : Boolean?=null,
    @SerializedName("disableReason") val disableReason : String?=null,

): Serializable, Parcelable

@Parcelize
data class WithdrawInfo (

    @SerializedName("min") val min : Double,
    @SerializedName("max") val max : Double,
    @SerializedName("fee") val fee : Double,
    @SerializedName("precision") val precision : Int?=null,
    @SerializedName("withdrawEnable") val withdrawEnable : Boolean,
    @SerializedName("memoRequired") val memoRequired : Boolean,
    @SerializedName("addressRegex") val addressRegex : String,
    @SerializedName("disableReason") val disableReason : String?=null,
    @SerializedName("memoRegex") val memoRegex : String?=null

): Serializable, Parcelable

@Parcelize
data class CryptoModel (

    @SerializedName("ramp") val ramp : String?=null,
    @SerializedName("rampInfo") val rampInfo : RampInfo?=null,
    @SerializedName("self") val self : Self?=null,
    @SerializedName("isTop") val isTop : Boolean = false,
    @SerializedName("networks") val networks:ArrayList<CryptoNetworkModel> = ArrayList(),
    @SerializedName("min") val min : Double = 1.0,
    @SerializedName("max") val max : Double = 5.0,
    @SerializedName("name") val name : String = EMPTY,
    @SerializedName("coin") val coin : String = EMPTY,
    @SerializedName("balance") var balance : String? = null,
    @SerializedName("mainCoin") val mainCoin : String = EMPTY,
    @SerializedName("withdrawInfo") var withdrawInfo : WithdrawInfo?=null,
    @SerializedName("depositInfo") var depositInfo : DepositInfo?=null,
    @SerializedName("sectionHeader") val sectionHeader : String = EMPTY,
    @SerializedName("percent") var percent : Double = 0.0,
    @SerializedName("balanceInFiat") var balanceInFiat : String = "0.0",
    @SerializedName("exchangeInfo") var exchangeInfo : ExchangeInfoModel?=null,
    @SerializedName("selectedNetwork") var selectedNetwork : CryptoNetworkModel?=null,
    @SerializedName("isDeposit") var isDeposit : Boolean = false


    ): Serializable, Parcelable

@Parcelize
data class CryptoToBTC(

    @SerializedName("symbol") val symbol : String,
    @SerializedName("price") var price : Double,
    @SerializedName("priceChangePercent") var priceChangePercent : Double? = 0.0,


    ) : Parcelable

@Parcelize
data class CryptoCryptoModel(
    @SerializedName("symbol") val symbol : String,
    @SerializedName("priceChange") val priceChange : Double,
    @SerializedName("priceChangePercent") val priceChangePercent : Double,
    @SerializedName("weightedAvgPrice") val weightedAvgPrice : Double,
    @SerializedName("prevClosePrice") val prevClosePrice : Double,
    @SerializedName("lastPrice") var lastPrice : Double = 0.0,
    @SerializedName("lastQty") val lastQty : Double,
    @SerializedName("bidPrice") val bidPrice : Double,
    @SerializedName("bidQty") val bidQty : Double,
    @SerializedName("askPrice") val askPrice : Double,
    @SerializedName("askQty") val askQty : Double,
    @SerializedName("openPrice") val openPrice : Double,
    @SerializedName("highPrice") val highPrice : Double,
    @SerializedName("lowPrice") val lowPrice : Double,
    @SerializedName("volume") val volume : Double,
    @SerializedName("quoteVolume") val quoteVolume : Double,
    @SerializedName("openTime") val openTime : Long,
    @SerializedName("closeTime") val closeTime : Long,
    @SerializedName("firstId") val firstId : Long,
    @SerializedName("lastId") val lastId : Long,
    @SerializedName("count") val count : Long

) : Parcelable{

    fun correctPrice() = if (lastPrice>0) lastPrice else prevClosePrice

}
