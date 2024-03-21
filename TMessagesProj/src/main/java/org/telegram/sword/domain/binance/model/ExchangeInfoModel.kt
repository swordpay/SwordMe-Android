package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import java.io.Serializable
@Parcelize
data class ExchangeInfoModel(


    @SerializedName("fromAsset") val fromAsset : String,
    @SerializedName("toAsset") val toAsset : String,
    @SerializedName("fromAssetMinAmount") val fromAssetMinAmount : String,
    @SerializedName("fromAssetMaxAmount") val fromAssetMaxAmount : String,
    @SerializedName("toAssetMinAmount") val toAssetMinAmount : String,
    @SerializedName("toAssetMaxAmount") val toAssetMaxAmount : String

): Serializable, Parcelable
