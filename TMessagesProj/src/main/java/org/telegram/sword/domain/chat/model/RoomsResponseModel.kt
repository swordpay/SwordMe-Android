package org.telegram.sword.domain.chat.model

import android.graphics.drawable.Drawable
import com.google.gson.annotations.SerializedName
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.ForChatAdapter.FRIENDS_ACCEPT
import org.telegram.sword.app.common.AppConst.ForChatAdapter.FRIENDS_MESSAGE
import org.telegram.sword.app.common.AppConst.ForChatAdapter.FRIENDS_PAY
import org.telegram.sword.app.common.AppConst.ForChatAdapter.FRIENDS_REJECT
import org.telegram.sword.app.common.AppConst.ForChatAdapter.FRIENDS_REQUEST
import org.telegram.sword.app.common.AppConst.ForChatAdapter.MY_ACCEPT
import org.telegram.sword.app.common.AppConst.ForChatAdapter.MY_MESSAGE
import org.telegram.sword.app.common.AppConst.ForChatAdapter.MY_PAY
import org.telegram.sword.app.common.AppConst.ForChatAdapter.MY_REJECT
import org.telegram.sword.app.common.AppConst.ForChatAdapter.MY_REQUEST
import org.telegram.sword.app.common.AppConst.Number.INVALID_NUMBER
import org.telegram.sword.app.common.AppConst.SPACE
import org.telegram.sword.domain.common.BaseListResponse
import org.telegram.tgnet.TLRPC
import org.telegram.ui.Components.AvatarDrawable
import org.telegram.ui.Components.BackupImageView
import java.io.Serializable

data class RoomsResponseModel(

    @SerializedName("rooms") val rooms: ArrayList<Rooms>

) : BaseListResponse(), Serializable

data class Rooms(

    @SerializedName("id") val id: Int = INVALID_NUMBER,
    @SerializedName("createdAt") val createdAt: String = EMPTY,
    @SerializedName("name") val name: String? = null,
    @SerializedName("users") var users: ArrayList<Users> = ArrayList(),
    @SerializedName("lastMessage") val lastMessage: ChatMessage? = null,
    @SerializedName("channelImage") val channelImage: String? = null

) : Serializable


fun  Rooms.correctAvatar():String{

    var avatar = EMPTY

    if (this.channelImage.isNullOrEmpty()){

        if (this.users.size>2){

            avatar = EMPTY

        }else{

            run blocking@{

                this.users.forEach { user ->

//
//                        avatar = user.avatar?: EMPTY
//
//                        return@blocking
//
//                    }

                }
            }
        }

    }else{

        avatar = this.channelImage?: EMPTY
    }

    return avatar
}


fun  Rooms.senderAvatar() = this.lastMessage?.sender?.avatar


fun  Rooms.correctPaymentCreator() = if (this.lastMessage?.payment?.creator?.username == "getUser()?.user?.username") "your" else
    this.lastMessage?.payment?.creator?.firstName+ SPACE +this.lastMessage?.payment?.creator?.lastName


fun Rooms.correctChannelName() = this.name

?: if (this.users.first().username != "getUser()?.user?.username")(this.users.first().firstName + SPACE + this.users.first().lastName)

else (this.users.last().firstName + SPACE + this.users.last().lastName)

fun Rooms.correctUserName() = if (this.users.size==2){

    if (this.users.first().username != "getUser()?.user?.username")("@"+this.users.first().username)

    else ("@"+this.users.last().username)

}else{
    EMPTY
}


data class ChatMessage(
    var type:Int = -10,
    @SerializedName("status") var status: String? = EMPTY,
    @SerializedName("isDeleted") var isDeleted: Boolean = false, //request is deleted
    @SerializedName("id") val id: Int,
    @SerializedName("isShowDate") val isShowDate: Boolean = true,
    @SerializedName("createdAt") val createdAt: String,
    @SerializedName("message") val message: String,
    @SerializedName("referenceId") val referenceId: String? = null,
    @SerializedName("sender") val sender: Sender,
    @SerializedName("payment") val payment: Payment? = null,
    @SerializedName("paymentTransaction") val paymentTransaction: PaymentTransaction? = null
) : Serializable {

    private val isMyMessage: Boolean get() = sender.username == "getUser()?.user?.username"

    val messageType: Int
        get() = if (payment != null) {

            if (isMyMessage) {

                if (payment.type == "request" && paymentTransaction == null) {

                    MY_REQUEST

                } else if (payment.type == "pay" && paymentTransaction != null) {

                    MY_PAY

                } else {

                    if (paymentTransaction != null && payment.type == "request") {

                        if (paymentTransaction.status == "rejected") {

                            MY_REJECT

                        } else {

                            MY_ACCEPT
                        }


                    } else {

                        INVALID_NUMBER
                    }

                }


            } else {

                if (payment.type == "request" && paymentTransaction == null) {

                    FRIENDS_REQUEST

                } else if (payment.type == "pay" && paymentTransaction != null) {

                    FRIENDS_PAY
                } else {

                    if (paymentTransaction != null && payment.type == "request") {

                        if (paymentTransaction.status == "rejected") {

                            FRIENDS_REJECT

                        } else {

                            FRIENDS_ACCEPT
                        }

                    } else {

                        INVALID_NUMBER
                    }

                }

            }

        } else {

            if (isMyMessage) {
                MY_MESSAGE
            } else {
                FRIENDS_MESSAGE
            }
        }

}

data class Payment(

    @SerializedName("id") val id: Int,
    @SerializedName("createdAt") val createdAt: String,
    @SerializedName("updatedAt") val updatedAt: String,
    @SerializedName("amount") val amount: String,
    @SerializedName("currencyType") val currencyType: String,
    @SerializedName("transactionType") val transactionType: String,
    @SerializedName("type") var type: String,
    @SerializedName("users") val users: Users? = null,
    @SerializedName("creator") val creator: Creator? = null,


    ) : Serializable

data class Creator(
    @SerializedName("username") val username: String? = null,
    @SerializedName("firstName") val firstName: String,
    @SerializedName("lastName") val lastName: String,
    @SerializedName("id") val id: Int,
    @SerializedName("avatar") val avatar: String? = null


) : Serializable


data class PaymentTransaction(

    @SerializedName("status") val status: String,// reject or pay

    @SerializedName("note") val note: String? = null,

    @SerializedName("currency") val currency: String? = null,

    @SerializedName("amount") val amount: Double? = 0.0,


    ) : Serializable


data class Sender(

    @SerializedName("firstName") val firstName: String,
    @SerializedName("lastName") val lastName: String,
    @SerializedName("username") val username: String,
    @SerializedName("avatar") val avatar: String? = null
) : Serializable


data class Users(
    var teleUser:TLRPC.User?=null,
    var imageView: BackupImageView?=null,
    var avatarDrawable:AvatarDrawable?=null,
    var accessHash: String?=null,
    var isChecked: Boolean = false,
    var headerName: String? = EMPTY,
    var isSelectedChip: Boolean = false,
    @SerializedName("id") val id: Long = 0,
    @SerializedName("status") var status: String? = EMPTY,
    @SerializedName("firstName") val firstName: String,
    @SerializedName("lastName") val lastName: String,
    @SerializedName("username") val username: String,
    @SerializedName("avatar") val avatar: String? = null,
    @SerializedName("avatarDir") val avatarDir: Drawable? = null
) : Serializable