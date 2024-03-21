package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable

data class LatestTradeModel(


    @SerializedName("rooms") val rooms : ArrayList<Rooms>,

): BaseResponse(),Serializable


