package org.telegram.sword.socket.webSocket.helper

import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoin
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.socket.webSocket.const_.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.model.request.BinanceSocketSendModel
import org.telegram.sword.socket.webSocket.model.request.toJsonString
import org.telegram.sword.socket.webSocket.viewModel.SocketViewModel

fun SocketViewModel.chart(method:String, coin:String, interval:String) = BinanceSocketSendModel(params = arrayListOf("${coin.lowercase()}${mainCoin.lowercase()}@kline_$interval"),method = method, id = 1).toJsonString()

fun SocketViewModel.coiPriceChangeStatistics(params:ArrayList<String>, method:String) = BinanceSocketSendModel(params = params,method = method, id = 1).toJsonString()

fun SocketViewModel.mainCoinToEuro(method:String) = BinanceSocketSendModel(params = arrayListOf("${ACCOUNT_CURRENCY().uppercase()}${mainCoin.lowercase()}@ticker"),method = method, id = 1).toJsonString()

fun SocketViewModel.coinToCoin(from:String,to:String,method:String) = BinanceSocketSendModel(params = arrayListOf("${from.lowercase()}${to.lowercase()}@ticker"),method = method, id = 1).toJsonString()



fun createPriceChangePercentParam(coinList: ArrayList<CryptoModel>):ArrayList<String>{

    val params:ArrayList<String> = ArrayList()

    coinList.forEach{

        if (it.coin.isNotEmpty()){

            params.add("${it.coin.lowercase()}${it.mainCoin.lowercase()}@ticker")
        }
    }

    return  params
}

