package org.telegram.sword.socket.webSocket.viewModel

import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.WebSocket
import okhttp3.logging.HttpLoggingInterceptor
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent
import org.telegram.sword.domain.common.BaseUrl.BINANCE_STREAM_URL
import org.telegram.sword.socket.webSocket.listener.BinanceWebSocketListener

var webSocket: WebSocket? = null

var client:OkHttpClient? = null

var socketIsConnected = false

open class SocketViewModel:ViewModel() {

    private lateinit var webSocketListener: BinanceWebSocketListener



    private var resp = SingleLiveEvent<String>()

    val socketResponse: LiveData<String> get() = resp

    private var isConnect = SingleLiveEvent<Boolean>()

    val isConnectedSocket: LiveData<Boolean> get() = isConnect

     fun sharedSocketData(response:String){

         viewModelScope.launch {

             resp.value = response
         }

    }

    fun connectSocket(viewModel: SocketViewModel){

        webSocketListener = BinanceWebSocketListener(viewModel = viewModel)

        if (client == null){

            client = OkHttpClient.Builder()

                .addInterceptor(HttpLoggingInterceptor().apply {
                    level = HttpLoggingInterceptor.Level.BASIC
                })
                .build()


        }
        val request = Request.Builder()
            .url(BINANCE_STREAM_URL)
            .build()

        client?.let { cl->


            webSocket = cl.newWebSocket(request, webSocketListener)

        }


    }

     fun isConnectedSocket(){

         socketIsConnected  = true

         isConnect.postValue(true)

    }

    fun socketListenTo(event: String){

       webSocket?.send(event)

    }
    fun socketListenTo(event: String,wSocket:WebSocket?){

        wSocket?.send(event)

    }


    fun stopSocket(){


    }

}