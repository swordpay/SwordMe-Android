package org.telegram.sword.app.common.helper.ui

import android.content.Context
import android.widget.FrameLayout
import org.telegram.messenger.AndroidUtilities
import org.telegram.messenger.MessagesController
import org.telegram.messenger.UserConfig
import org.telegram.messenger.UserObject
import org.telegram.sword.app.common.extansion.show
import org.telegram.tgnet.TLRPC
import org.telegram.ui.Components.AvatarDrawable
import org.telegram.ui.Components.BackupImageView


fun loadTeleAvatar(context:Context,peerId:Long,isChannel:Boolean = false,frame:FrameLayout, radius:Float) {



    frame.show()


    var inputPeer: TLRPC.InputPeer =
        MessagesController.getInstance(UserConfig.selectedAccount).getInputPeer(peerId)

    var currentChat: TLRPC.Chat? = null

    if (isChannel) {




         currentChat = getTeleChatInId(peerId)



        if (currentChat != null) {

            inputPeer = MessagesController.getInputPeer(currentChat)
        }
    }


    val avatarImageView: BackupImageView

    val avatarDrawable: AvatarDrawable

    try {

        if (inputPeer != null) {

            if (inputPeer.type == "inputPeerUser") {


                    avatarImageView = BackupImageView(context)

                    avatarDrawable = AvatarDrawable()

                    frame.addView(avatarImageView)

                    avatarImageView.contentDescription = "Avatar"

                    avatarImageView.setRoundRadius(AndroidUtilities.dp(radius))

                    val user = getTeleUserInId(peerId)

                    if (user!=null){
                        setTeleUserAvatar(
                            avatarImageView,
                            avatarDrawable,
                            user as TLRPC.User,
                            false
                        )

                    }




            } else if (inputPeer?.type == "inputPeerChat") {

                avatarImageView = BackupImageView(context)

                avatarDrawable = AvatarDrawable()

                frame.addView(avatarImageView)

                avatarImageView.contentDescription = "Avatar"

                avatarImageView.setRoundRadius(AndroidUtilities.dp(radius))

                    val chat = getTeleChatInId(peerId)

                    setTeleChatAvatar(
                        chat,
                        avatarImageView,
                        avatarDrawable,
                        radius
                    )


            } else if (inputPeer?.type == "inputPeerChannel") {


                avatarImageView = BackupImageView(context)

                avatarDrawable = AvatarDrawable(currentChat as TLRPC.Chat)

                frame.addView(avatarImageView)

                avatarImageView.contentDescription = "Avatar"

                avatarImageView.setRoundRadius(AndroidUtilities.dp(radius))


                if (inputPeer.channel_id != null) {

                    val chat = getTeleChatInId(inputPeer.channel_id!!)

                    setTeleChatAvatar(

                        chat,

                        avatarImageView,

                        avatarDrawable,
                        radius
                    )

                }

            }

        }
    } catch (e: Exception) {


    }

}


 fun getTeleUserInId(id: Long): TLRPC.User? {

    val currentAccount = UserConfig.selectedAccount

    val messagesController = MessagesController.getInstance(currentAccount)

    return messagesController.getUser(id)

}

 fun getTeleChatInId(id: Long): TLRPC.Chat {

    val currentAccount = UserConfig.selectedAccount

    val messagesController = MessagesController.getInstance(currentAccount)

    return messagesController.getChat(id)
}


fun setTeleUserAvatar(
    avatarImageView: BackupImageView?,
    avatarDrawable: AvatarDrawable,
    user: TLRPC.User?,
    showSelf: Boolean
) {
    avatarDrawable.setInfo(user as TLRPC.User)
    if (UserObject.isReplyUser(user as TLRPC.User)) {
        avatarDrawable.avatarType = AvatarDrawable.AVATAR_TYPE_REPLIES
        avatarDrawable.setScaleSize(.8f)
        avatarImageView?.setImage(null, null, avatarDrawable, user)
    } else if (UserObject.isUserSelf(user as TLRPC.User) && !showSelf) {
        avatarDrawable.avatarType = AvatarDrawable.AVATAR_TYPE_SAVED
        avatarDrawable.setScaleSize(.8f)
        avatarImageView?.setImage(null, null, avatarDrawable, user)
    } else {
        avatarDrawable.setScaleSize(1f)
        avatarImageView?.setForUserOrChat(user, avatarDrawable)
    }
}

fun setTeleChatAvatar(
    chat: TLRPC.Chat?,
    avatarImageView: BackupImageView?,
    avatarDrawable: AvatarDrawable,
    radius:Float
) {




    avatarDrawable.setInfo(chat)
    if (avatarImageView != null) {
        avatarImageView.setForUserOrChat(chat, avatarDrawable)
        avatarImageView.setRoundRadius(
            if (chat != null && chat.forum) AndroidUtilities.dp(radius) else AndroidUtilities.dp(
                radius
            )
        )
    }
}
