package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName
import org.telegram.sword.domain.common.BaseListResponse
import java.io.Serializable

data class UsersResponseModel(

    @SerializedName("users") val users: ArrayList<Users>

): BaseListResponse(), Serializable
