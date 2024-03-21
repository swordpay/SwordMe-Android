package org.telegram.sword.socket.socketIo

import android.content.Context
import io.socket.client.IO
import io.socket.client.Socket
import org.telegram.sword.app.common.AppConst.Key.IMMORTAL_TOKEN
import org.telegram.sword.app.common.extansion.getShared
import org.telegram.sword.domain.common.BaseUrl.SOCKET_IO_BASE_URL

object SocketHandler {

    lateinit var mSocket: Socket


    @Synchronized
    fun setSocket(context:Context) {
        try {

            val immortalToken =   context.getShared(key = IMMORTAL_TOKEN)

            val options = IO.Options().apply {

                transports = arrayOf("websocket")

                auth = mapOf("token" to immortalToken)

                timeout = Int.MAX_VALUE.toLong()

            }


            try {

                mSocket.disconnect()

            }catch (e:Exception){}

            mSocket = IO.socket("${SOCKET_IO_BASE_URL}chat",options)


            connectChatSocket()

        } catch (e: Exception) {


        }
    }

    @Synchronized
    fun getSocket(): Socket {

        return mSocket
    }

    @Synchronized
    fun connectChatSocket() {

        try {

            mSocket.connect()

        }catch (e:Exception){

        }

    }

    @Synchronized
    fun disconnectChatSocket() {
        try {

            mSocket.disconnect()

        }catch (e:Exception){}

    }
}


