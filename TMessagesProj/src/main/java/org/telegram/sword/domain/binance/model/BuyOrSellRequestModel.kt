package org.telegram.sword.domain.binance.model


import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class BuyOrSellRequestModel(

    var coin:String,
    var network:Network?=null,
    var ramp:String?=null,
    var coinId:String?=null,
    var paymentMethod:String?=null,
    var quoteId:String?=null,
    var amount:Double,
    var currency:String?=null,
    var amountType:String

) : Parcelable

@Parcelize
data class Network(

    var source:String?=null,
    var ramp:String?=null,

) : Parcelable
