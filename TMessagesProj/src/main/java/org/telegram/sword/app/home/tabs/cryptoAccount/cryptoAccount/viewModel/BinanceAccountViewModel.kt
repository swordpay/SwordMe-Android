package org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel

import android.annotation.SuppressLint
import android.os.CountDownTimer
import androidx.lifecycle.LiveData
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.telegram.messenger.ApplicationLoader
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent
import org.telegram.sword.app.common.helper.function.isConnectedToInternet
import org.telegram.sword.domain.binance.model.*
import org.telegram.sword.domain.binance.useCase.BinanceUseCase
import org.telegram.sword.domain.common.BaseResource
import org.telegram.sword.domain.common.Status
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol
import org.telegram.sword.socket.webSocket.viewModel.SocketViewModel
import java.util.*

class BinanceAccountViewModel(private val useCase: BinanceUseCase): SocketViewModel() {

    var timer:CountDownTimer? = null
    var loopTimer:CountDownTimer? = null

    private var balance = SingleLiveEvent<BaseResource<CryptoBalanceResponseModel>>()
    val cryptoBalance: LiveData<BaseResource<CryptoBalanceResponseModel>> get() = balance

    private var assets = SingleLiveEvent<BaseResource<CryptoAssetsResponseModel>>()
    val cryptoAssets: LiveData<BaseResource<CryptoAssetsResponseModel>> get() = assets

    private var singleCoin = SingleLiveEvent<BaseResource<SingleCoinResponseModel>>()
    val singleCoinResp: LiveData<BaseResource<SingleCoinResponseModel>> get() = singleCoin

    private var trade = SingleLiveEvent<BaseResource<TradeInfoResponseModel>>()
    val tradeInfo: LiveData<BaseResource<TradeInfoResponseModel>> get() = trade

    private var chart = SingleLiveEvent<BaseResource<ArrayList<ArrayList<String>>>>()
    val chartResponse: LiveData<BaseResource<ArrayList<ArrayList<String>>>> get() = chart

    private var buyOrSell = SingleLiveEvent<BaseResource<BuyOrSellResponseModel>>()
    val buyOrSellResp: LiveData<BaseResource<BuyOrSellResponseModel>> get() = buyOrSell

    private var cryptoToCrypto = SingleLiveEvent<BaseResource<ArrayList<CryptoCryptoModel>>>()
    val cryptoToCryptoResp: LiveData<BaseResource<ArrayList<CryptoCryptoModel>>> get() = cryptoToCrypto

    private var exchangeInfo = SingleLiveEvent<BaseResource<ArrayList<ExchangeInfoModel>>>()
    val exchangeInfoResp: LiveData<BaseResource<ArrayList<ExchangeInfoModel>>> get() = exchangeInfo




    fun fetchSingleCoin(coin:String) {

        viewModelScope.launch {

            try {

                singleCoin.value =  useCase.getSingleCoin(coin = coin)

            }catch (e:Exception){ singleCoin.value = BaseResource(status = Status.FAILURE) }
        }
    }



    fun fetchCryptoBalance(currency:String) {

        viewModelScope.launch {

            try {

                balance.value =  useCase.getBalance(currency = currency)

            }catch (e:Exception){ balance.value = BaseResource(status = Status.FAILURE) }
        }
    }
    fun fetchExchangeInfo(fromAsset:String) {

        viewModelScope.launch {

            try {

                exchangeInfo.value =  useCase.getExchangeInfo(fromAsset = fromAsset)

            }catch (e:Exception){ exchangeInfo.value = BaseResource(status = Status.FAILURE) }
        }
    }



    fun fetchCartData(symbol:String,range:Int,interval:String) {

        viewModelScope.launch {

            try {

                val reqData = createResponseChartDataInInterval(range =range,symbol = symbol,interval = interval)


                chart.value =  useCase.getChart(symbols = reqData.symbol,interval =reqData.interval,startTime = reqData.startTime.toString(),endTime = reqData.endTime.toString())

            }catch (e:Exception){ chart.value = BaseResource(status = Status.FAILURE) }
        }
    }

    fun fetchCryptoAssets(onlyStable:Boolean = false) {

        viewModelScope.launch {

            try {

                    assets.value =  useCase.getCryptoAssets(onlyStable = onlyStable)


            }catch (e:Exception){ assets.value = BaseResource(status = Status.FAILURE) }
        }
    }

    fun fetchCryptoToCryptoInfo(symbols:String) {

        viewModelScope.launch {

            try {

                cryptoToCrypto.value =  useCase.getCryptoPriceChangeStatistics(symbols = symbols)

            }catch (e:Exception){ cryptoToCrypto.value = BaseResource(status = Status.FAILURE) }
        }
    }


    fun fetchTradeInfo(coin:String,type: String) {

        viewModelScope.launch {

            try {

                trade.value =  useCase.getTradeInfo(coin = coin,type = type)

            }catch (e:Exception){ trade.value = BaseResource(status = Status.FAILURE) }
        }
    }


    fun buyOrSell(type:String,requestData: BuyOrSellRequestModel) {

        viewModelScope.launch {

            try {

                buyOrSell.value =  useCase.buyOrSell(type = type,requestData = requestData)

            }catch (e:Exception){ buyOrSell.value = BaseResource(status = Status.FAILURE) }
        }
    }



    fun comBainCryptoList(responseData:CryptoAssetsResponseModel,addHeaders:Boolean = false) : ArrayList<CryptoModel>{

        val allCryptoList:ArrayList<CryptoModel> = ArrayList()



        responseData.data.self.forEach{

            allCryptoList.add(CryptoModel(
                networks = it.networks,
                self = it,
                coin = it.coin,
                name = it.name,
                balance = it.balance,
                withdrawInfo = it.WithdrawInfo,
                mainCoin = responseData.data.mainCoin,
                depositInfo = it.depositInfo,
                ramp = it.ramp

            ))

        }


        if (responseData.data.top.isNotEmpty() && addHeaders){

            allCryptoList.add(CryptoModel(sectionHeader = "Top Crypto"))
        }

        responseData.data.top.forEach{

            allCryptoList.add(CryptoModel(
                isTop = true,
                networks = it.networks,
                coin = it.coin,
                name = it.name,
                withdrawInfo = it.WithdrawInfo,
                mainCoin = responseData.data.mainCoin,
                depositInfo = it.depositInfo,
                ramp = it.ramp
            ))

        }

        if (responseData.data.available.isNotEmpty() && addHeaders){

            allCryptoList.add(CryptoModel(sectionHeader = "Explore more crypto"))
        }

        responseData.data.available.forEach{

            allCryptoList.add(CryptoModel(
                networks = it.networks,
                coin = it.coin,
                name = it.name,
                withdrawInfo = it.WithdrawInfo,
                mainCoin = responseData.data.mainCoin,
                depositInfo = it.depositInfo,
                ramp = it.ramp

            ))

        }

        return allCryptoList

    }


    private fun createResponseChartDataInInterval(range: Int, symbol: String, interval: String):ChartDataResponseModel {

        val calendar = Calendar.getInstance()

        calendar.add(Calendar.DAY_OF_YEAR, range)

        return    ChartDataResponseModel(interval = interval, symbol = symbol, startTime = calendar.time.time, endTime = System.currentTimeMillis())


    }


    fun startFetchCryptoBalanceTimer() {

        stopLoopTimer()

        viewModelScope.launch {
            try {
                timer = object : CountDownTimer(5100, 1000) {

                    @SuppressLint("SetTextI18n")
                    override fun onTick(millisUntilFinished: Long) {

                        val second = millisUntilFinished / 1000

                        if (second % 5L == 0L) {

                            try {
                                if (isConnectedToInternet(ApplicationLoader.applicationContext)) {

                                    fetchCryptoBalance(currency = CryptoSymbol.ACCOUNT_CURRENCY)
                                }

                            }catch (e:Exception){

                            }

                        }

                    }

                    override fun onFinish() {

                        startLoopTimer()

                    }
                }.start()


            } catch (_: Exception) {  }
        }
    }

    fun startLoopTimer() {

        stopTimer()

        viewModelScope.launch {
            try {
                loopTimer = object : CountDownTimer(9000000, 10000) {

                    @SuppressLint("SetTextI18n")
                    override fun onTick(millisUntilFinished: Long) {

                        try {
                            if (isConnectedToInternet(ApplicationLoader.applicationContext)) {

                                fetchCryptoBalance(currency = CryptoSymbol.ACCOUNT_CURRENCY)
                            }

                        }catch (e:Exception){

                        }

                    }

                    override fun onFinish() {

                        startLoopTimer()

                    }
                }.start()


            } catch (_: Exception) {  }
        }
    }


    fun stopTimer(){

        timer?.cancel()
        timer = null
    }

    fun stopLoopTimer(){

        loopTimer?.cancel()
        loopTimer = null
    }

}