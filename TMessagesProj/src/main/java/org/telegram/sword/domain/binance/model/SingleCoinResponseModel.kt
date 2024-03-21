package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable
@Parcelize
data class SingleCoinResponseModel(

    @SerializedName("data") val data : SingleCoinData

): BaseResponse(), Serializable, Parcelable

@Parcelize
data class SingleCoinData (

    @SerializedName("balance") val balance : Double,
    @SerializedName("locked") val locked : Int,
    @SerializedName("freeze") val freeze : Int,

): Serializable, Parcelable