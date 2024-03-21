package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName

data class PaymentsRequestModel(

    @SerializedName("transactionType") val transactionType : String,
    @SerializedName("amount") val amount : Double,
    @SerializedName("note") val note : String,
    @SerializedName("type") val type : String,
    @SerializedName("roomId") val roomId : Int?=null,
    @SerializedName("users") val users : ArrayList<Int>?=null,
    @SerializedName("currencyType") val currencyType : String,
    @SerializedName("referenceId") val referenceId:String,
    @SerializedName("amountType") val amountType:String?=null
)