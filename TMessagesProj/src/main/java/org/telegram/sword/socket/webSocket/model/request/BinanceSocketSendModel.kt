package org.telegram.sword.socket.webSocket.model.request

import com.google.gson.Gson

const val SUBSCRIBE = "SUBSCRIBE"

const val UNSUBSCRIBE = "UNSUBSCRIBE"

data class BinanceSocketSendModel(

 var method:String = SUBSCRIBE,

 var params:ArrayList<String>,

 var id:Int,

)

fun BinanceSocketSendModel.toJsonString():String{

 val gson = Gson()

 return gson.toJson(this)
}



