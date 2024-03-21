package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName

data class RoomUsersResponseModel(

    @SerializedName("users") val users: ArrayList<Users>

)
