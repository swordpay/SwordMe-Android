package org.telegram.sword.socket.webSocket.const_

import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.CURRENCY_SYMBOL

internal object CryptoSymbol{

    var ACCOUNT_CURRENCY = "EUR"

    var CURRENCY_SYMBOL = "€"

}

fun CURRENCY_IS_EUR() = ACCOUNT_CURRENCY =="EUR"

fun CURRENCY_IS_USD() = ACCOUNT_CURRENCY =="USD"


fun setCurrentCurrency(currency:String = "EUR"){

    CURRENCY_SYMBOL  = when(currency){

        "EUR" -> "€"

        "USD" -> "$"

         else -> "€"

    }

    ACCOUNT_CURRENCY = currency

}

public fun ACCOUNT_CURRENCY() = ACCOUNT_CURRENCY

internal object BinanceStreams{

    const val PRICE_AND_PERCENT_EVENT = "@ticker"

    const val CHART_EVENT = "@kline_"


}