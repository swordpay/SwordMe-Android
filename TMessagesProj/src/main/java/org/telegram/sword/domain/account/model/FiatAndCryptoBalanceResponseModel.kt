package org.telegram.sword.domain.account.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.BalanceType.ALL
import org.telegram.sword.app.common.AppConst.BalanceType.CRYPTO
import org.telegram.sword.app.common.AppConst.BalanceType.FIAT

@Parcelize
data class FiatAndCryptoBalanceResponseModel(

    @SerializedName("data") val data : Data
):Parcelable
@Parcelize
data class Data (
    @SerializedName("crypto") val crypto : ArrayList<Crypto>,
    @SerializedName("fiat") val fiat : Fiat?=null,
    @SerializedName("mainCoin") val mainCoin : String?=null,
    @SerializedName("redirectUrl") val redirectUrl : String?=null
):Parcelable{

    val balanceType:String get() =

        if (!crypto.isNullOrEmpty() && fiat!=null){

            ALL

        }else if (!crypto.isNullOrEmpty()){

            CRYPTO

        }else{

            FIAT
        }

}

@Parcelize
data class Crypto (

    @SerializedName("balance") val balance : Double,
    @SerializedName("locked") val locked : Double,
    @SerializedName("freeze") val freeze : Double,
    @SerializedName("coin") val coin : String
):Parcelable
@Parcelize
data class Fiat (

    @SerializedName("balance") val balance : Double,
    @SerializedName("currency") val currency : String
):Parcelable
