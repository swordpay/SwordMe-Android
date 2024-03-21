package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class CoinUpdateStatusCheckModel(

    @SerializedName("transaction")
    val transaction:Status?=null

) : Parcelable

@Parcelize
data class Status(

    @SerializedName("status")
    val status:String?=null ,

    @SerializedName("coin")
    val coin:String
) : Parcelable

