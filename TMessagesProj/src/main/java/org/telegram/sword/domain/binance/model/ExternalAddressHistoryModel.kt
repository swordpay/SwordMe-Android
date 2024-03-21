package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Number.INVALID_NUMBER
@Parcelize
data class ExternalAddressHistoryModel (

    @SerializedName("addresses") val addresses:ArrayList<Addresses>) : Parcelable

@Parcelize
data class Addresses(

    @SerializedName("id") val id:Int = INVALID_NUMBER,
    @SerializedName("address") val address:String = EMPTY,
    @SerializedName("addressName") val addressName:String = EMPTY,
    @SerializedName("network") val network:String = EMPTY,
    @SerializedName("coin") val coin:String = EMPTY,
    @SerializedName("createdAt") val createdAt:String = EMPTY,
    @SerializedName("updatedAt") val updatedAt:String = EMPTY,
    @SerializedName("isChecked") var isChecked:Boolean = false,
    @SerializedName("isAddNewAddress") val isAddNewAddress:Boolean = false

) : Parcelable