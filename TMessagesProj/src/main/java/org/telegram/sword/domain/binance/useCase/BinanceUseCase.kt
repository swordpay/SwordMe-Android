package org.telegram.sword.domain.binance.useCase

import android.content.Context
import org.telegram.sword.domain.binance.apiHelper.BinanceApiHelper
import org.telegram.sword.domain.binance.model.BuyOrSellRequestModel
import org.telegram.sword.domain.common.BaseDataSource

class BinanceUseCase(private val apiHelper: BinanceApiHelper, context: Context): BaseDataSource(context = context) {

    suspend fun getBalance(currency:String) =  safeApiCall { apiHelper.getBalance(currency = currency) }

    suspend fun getCryptoAssets(onlyStable:Boolean) =  safeApiCall { apiHelper.getCryptoAssets(onlyStable) }

    suspend fun getSingleCoin(coin:String) =  safeApiCall { apiHelper.getSingleCoin(coin =coin) }

    suspend fun getExchangeInfo(fromAsset:String) =  safeApiCall { apiHelper.getExchangeInfo(fromAsset =fromAsset) }

    suspend fun getTradeInfo(coin:String,type :String) =  safeApiCall { apiHelper.getTradeInfo(coin = coin,type  = type) }

    suspend fun buyOrSell(type:String,requestData: BuyOrSellRequestModel) =  safeApiCall { apiHelper.buyOrSell(type =type,requestData = requestData) }

    suspend fun getCryptoPriceChangeStatistics(symbols:String) =  safeApiCall { apiHelper.getCryptoPriceChangeStatistics(symbols = symbols) }

    suspend fun getChart(symbols:String,interval:String,startTime:String,endTime:String) =  safeApiCall { apiHelper.getChart(symbols = symbols,interval =interval,startTime = startTime,endTime = endTime) }


}