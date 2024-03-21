package org.telegram.sword.domain.chat.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable

@Parcelize
data class PayOrRequestResponseModel(

    @SerializedName("data") val data:PayOrReqData,

): BaseResponse(), Serializable, Parcelable
@Parcelize
data class PayOrReqData(

    @SerializedName("room") val room : Rooms?=null,
    @SerializedName("redirectUrl") val redirectUrl : String?=null
) : Parcelable

@Parcelize
data class PayOrRequestResponseModelTele(

    @SerializedName("data") val data:PayOrReqDataTele?=null,

    ): BaseResponse(), Serializable, Parcelable
@Parcelize
data class PayOrReqDataTele(

    @SerializedName("redirectUrl") val redirectUrl : String?=null
) : Parcelable