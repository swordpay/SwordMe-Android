package org.telegram.sword.domain.chat.model

import com.google.gson.annotations.SerializedName
import org.telegram.sword.domain.common.BaseListResponse
import java.io.Serializable
import java.util.*

data class LiveChatResponseModel(

    @SerializedName("messages") val messages : LinkedList<ChatMessage>

): BaseListResponse(), Serializable
