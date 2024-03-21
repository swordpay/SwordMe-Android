package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
@Parcelize
data class CryptoBalanceResponseModel(

    @SerializedName("data") val data: Balance,

) : BaseResponse(), Parcelable

@Parcelize
data class Balance(

    @SerializedName("balance") val balance: Double,

) : Parcelable
