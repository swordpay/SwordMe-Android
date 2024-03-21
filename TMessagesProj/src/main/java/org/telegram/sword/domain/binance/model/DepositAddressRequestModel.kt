package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize

@Parcelize
data class DepositAddressRequestModel(

    @SerializedName("data") val data : DepositAddressData
) : Parcelable

@Parcelize
data class DepositAddressData (

    @SerializedName("address") val address : String,
    @SerializedName("qrCode") val qrCode : String,
    @SerializedName("memo") val memo : String?=null
) : Parcelable