package org.telegram.sword.app.common.base

import com.google.gson.annotations.SerializedName
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.domain.chat.model.Rooms
import java.io.Serializable

data class RedirectOrPushDataModel(
    @SerializedName("key")
    var key: String = EMPTY,

    @SerializedName("data")
    var data: RedirectOrPushData?=null,
    var room: Rooms?=null
): Serializable

data class RedirectOrPushData(
    @SerializedName("status")
    var status:String = EMPTY,
    var message:String = EMPTY,
    var id:String = EMPTY,

    ): Serializable