package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable

@Parcelize
data class SuccessResponseModel(

    @SerializedName("data")
    val data:IsSuccess,

    ):BaseResponse() , Parcelable, Serializable

@Parcelize
data class IsSuccess(

    @SerializedName("success")
    val success:Boolean
) : Parcelable,Serializable