package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable

data class CreateTradeResponseModel (

    @SerializedName("data") val data:Rooms


 ): BaseResponse(), Serializable

