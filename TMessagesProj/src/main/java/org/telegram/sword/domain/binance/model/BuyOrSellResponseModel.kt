package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
@Parcelize
data class BuyOrSellResponseModel(
    @SerializedName("data")   var data:Transaction


): BaseResponse(), Parcelable

@Parcelize
data class Transaction(

    @SerializedName("redirectUrl")  var redirectUrl:String?=null,
    @SerializedName("success")  var success:Boolean?=null,
    @SerializedName("type")  var type:String?=null

) : Parcelable


