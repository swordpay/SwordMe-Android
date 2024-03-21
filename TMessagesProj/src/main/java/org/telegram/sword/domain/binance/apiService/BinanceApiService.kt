package org.telegram.sword.domain.binance.apiService

import org.telegram.sword.domain.binance.model.*
import org.telegram.sword.domain.common.Version
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import retrofit2.Response
import retrofit2.http.*

interface BinanceApiService {

    @GET("${Version.V_1}/crypto/balance")
    suspend fun getBalance(): Response<CryptoBalanceResponseModel>  //@Query("currency")currency:String

    @GET("${Version.V_1}/crypto/assets")
    suspend fun getCryptoAssets(@Query("onlyStable" )onlyStable:Boolean): Response<CryptoAssetsResponseModel>

    @GET("${Version.V_1}/crypto/assets/{coin}")
    suspend fun getSingleCoin(@Path("coin") coin:String): Response<SingleCoinResponseModel>

    @GET("/sapi/v1/convert/exchangeInfo")
    suspend fun getExchangeInfo(@Query("fromAsset" )fromAsset:String): Response<ArrayList<ExchangeInfoModel>>

    @GET("/api/v3/ticker/24hr")
    suspend fun getCryptoPriceChangeStatistics(@Query("symbols" )symbols:String ): Response<ArrayList<CryptoCryptoModel>>

    @GET("${Version.V_1}/crypto/trade/info")
    suspend fun getTradeInfo(@Query("type")type :String,@Query("coin")coin :String,@Query("currency")currency:String = ACCOUNT_CURRENCY ): Response<TradeInfoResponseModel>

    @POST("${Version.V_1}/crypto/{type}")
    suspend fun buyOrSell(@Path("type") type:String, @Body requestData:BuyOrSellRequestModel): Response<BuyOrSellResponseModel>

    @GET("/api/v3/klines")
    suspend fun getChart(@Query("symbol" )symbols:String ,
                         @Query("interval")interval :String,
                         @Query("startTime")startTime :String,
                         @Query("endTime")endTime :String): Response<ArrayList<ArrayList<String>>>



}