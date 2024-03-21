package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName

data class SendMessageResponseModel (

@SerializedName("data")
val data: SendMessageData,

)

data class SendMessageData(

    @SerializedName("referenceId")
    val referenceId: String,

    @SerializedName("id")
    val id: Int,


)
