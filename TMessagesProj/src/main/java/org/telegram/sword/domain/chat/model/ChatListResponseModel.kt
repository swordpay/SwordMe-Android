package org.telegram.sword.domain.chat.model

import android.graphics.Color
import org.telegram.sword.app.common.AppConst.EMPTY

data class ChatListResponseModel(

    var avatar:String = EMPTY,
    var name:String = EMPTY,
    var lastMessage:String = EMPTY,
    var channelStatus:String = EMPTY,
    var chatDate:String = EMPTY,
    var newMessage:Boolean = false,
    var color:Int = if (avatar.isEmpty()) (Math.random() * 16777215).toInt() or (0xFF shl 24) else Color.TRANSPARENT,

    )

object ChatList{
    private var chatList:ArrayList<ChatListResponseModel> = ArrayList()
    fun generateChatList():ArrayList<ChatListResponseModel>{
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Tiko",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
            newMessage = true,
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Arman",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Private Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/1e/ce/6c/1ece6c1b9c45f3fc2135fbb5a00da4f3.jpg",
            name = "Ando",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gogo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Anna",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gago",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

            ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Varo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Tiko",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
            newMessage = true,
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Arman",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Private Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/1e/ce/6c/1ece6c1b9c45f3fc2135fbb5a00da4f3.jpg",
            name = "Ando",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gogo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

            ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Anna",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gago",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

            ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Varo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Tiko",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
            newMessage = true,
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Arman",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Private Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/1e/ce/6c/1ece6c1b9c45f3fc2135fbb5a00da4f3.jpg",
            name = "Ando",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gogo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

            ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Anna",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Mayis",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "Public Channel",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Anushavan",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        chatList.add(ChatListResponseModel(
            avatar = EMPTY,
            name = "Gago",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22"
        ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/3c/1f/45/3c1f45ad4f30bdfffee5103c6670740e.jpg",
            name = "Vaxo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",

            ))
        chatList.add(ChatListResponseModel(
            avatar = "https://i.pinimg.com/564x/39/da/ae/39daaeea1496643f9c8137b0dc8ee616.jpg",
            name = "Varo",
            lastMessage = "Bla Bla Bla Bla Bla Bla Bla",
            channelStatus = "",
            chatDate = "12/22",
        ))
        return chatList
    }
}