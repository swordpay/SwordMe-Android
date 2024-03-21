package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName
import org.telegram.sword.app.common.AppConst.EMPTY

data class AcceptRequestModel(


    var transactionType:String,
    var amount:Double,
    var referenceId:String,
    var note:String = EMPTY,
    var currencyType:String

)

data class AcceptRequestModelTele(


    @SerializedName("transactionType") val transactionType : String,
    @SerializedName("amount") val amount : Double?=null,
    @SerializedName("note") val note : String,
    @SerializedName("peer") val peer : PeerInfo,
    @SerializedName("messageId") val messageId : Int,
    @SerializedName("currencyType") val currencyType : String

)

