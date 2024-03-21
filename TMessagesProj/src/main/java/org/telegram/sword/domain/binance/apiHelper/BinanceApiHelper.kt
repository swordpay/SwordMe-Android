package org.telegram.sword.domain.binance.apiHelper

import org.telegram.sword.domain.binance.apiService.BinanceApiService
import org.telegram.sword.domain.binance.model.BuyOrSellRequestModel
import retrofit2.Retrofit

class BinanceApiHelper(service: Retrofit,binanceService:Retrofit) {

    private val service = service.create(BinanceApiService::class.java)

    private val binanceService = binanceService.create(BinanceApiService::class.java)

    suspend fun getBalance(currency:String) = service.getBalance() //currency = currency

    suspend fun getCryptoAssets(onlyStable:Boolean) = service.getCryptoAssets(onlyStable)

    suspend fun getSingleCoin(coin:String) = service.getSingleCoin(coin = coin)

    suspend fun getExchangeInfo(fromAsset:String) = binanceService.getExchangeInfo(fromAsset = fromAsset)

    suspend fun getTradeInfo(coin:String,type :String) = service.getTradeInfo(coin = coin,type  = type )

    suspend fun buyOrSell(type:String,requestData: BuyOrSellRequestModel) = service.buyOrSell(type = type,requestData = requestData)

    suspend fun getCryptoPriceChangeStatistics(symbols:String) = binanceService.getCryptoPriceChangeStatistics(symbols = symbols)

    suspend fun getChart(symbols:String,interval:String,startTime:String,endTime:String)
    = binanceService.getChart(symbols = symbols,interval =interval,startTime = startTime,endTime = endTime)

}