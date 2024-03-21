package org.telegram.sword.app.home.tabs.cryptoAccount.coinDetails.view

import android.annotation.SuppressLint
import android.os.Parcelable
import androidx.annotation.NonNull
import androidx.appcompat.widget.AppCompatTextView
import androidx.core.view.isVisible
import com.google.gson.Gson
import com.robinhood.spark.SparkAdapter
import kotlinx.coroutines.*
import kotlinx.parcelize.Parcelize
import okhttp3.WebSocket
import org.koin.androidx.viewmodel.ext.android.getViewModel
import org.telegram.messenger.R
import org.telegram.messenger.databinding.ActivityCoinDetailsBinding
import org.telegram.sword.app.bottomSheet.buyAndSellCoin.model.BuyOrSellSharedModel
import org.telegram.sword.app.bottomSheet.buyAndSellCoin.view.BuyAndSellCoin
import org.telegram.sword.app.common.AppConst.BuyOrSellType.BUY
import org.telegram.sword.app.common.AppConst.BuyOrSellType.SELL
import org.telegram.sword.app.common.AppConst.ChartInterval.D_1
import org.telegram.sword.app.common.AppConst.ChartInterval.H_1
import org.telegram.sword.app.common.AppConst.ChartInterval.H_4
import org.telegram.sword.app.common.AppConst.ChartInterval.M_15
import org.telegram.sword.app.common.AppConst.ChartRange.D1
import org.telegram.sword.app.common.AppConst.ChartRange.M1
import org.telegram.sword.app.common.AppConst.ChartRange.M3
import org.telegram.sword.app.common.AppConst.ChartRange.M6
import org.telegram.sword.app.common.AppConst.ChartRange.W1
import org.telegram.sword.app.common.AppConst.ChartRange.Y1
import org.telegram.sword.app.common.AppConst.Key.SHARED_EXTRA
import org.telegram.sword.app.common.AppConst.PRECISION_8
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.colors.AppColors.DARK_TEXT_COLOR
import org.telegram.sword.app.common.colors.AppColors.GREEN_TEXT
import org.telegram.sword.app.common.colors.AppColors.RED
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.extansion.ui.set
import org.telegram.sword.app.common.extansion.ui.topBar.set
import org.telegram.sword.app.common.helper.db.getUser
import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.common.helper.function.showNoInetDialog
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.*
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.helper.correctBalanceInEuro
import org.telegram.sword.app.home.tabs.cryptoAccount.state.goToHomeTab
import org.telegram.sword.app.home.tabs.cryptoAccount.state.isUpdateMyCryptoBalance
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.domain.common.Status
import org.telegram.sword.socket.webSocket.const_.BinanceStreams
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.helper.coiPriceChangeStatistics
import org.telegram.sword.socket.webSocket.helper.createPriceChangePercentParam
import org.telegram.sword.socket.webSocket.helper.mainCoinToEuro
import org.telegram.sword.socket.webSocket.model.request.SUBSCRIBE
import org.telegram.sword.socket.webSocket.model.request.UNSUBSCRIBE
import org.telegram.sword.socket.webSocket.model.response.CryptoToCryptoEventModel
import org.telegram.sword.socket.webSocket.viewModel.webSocket
import java.io.Serializable
import java.util.*


class CoinDetails : BaseActivity<ActivityCoinDetailsBinding, BinanceAccountViewModel>(){

    private var coinDetailsWebSocket: WebSocket? = null

    val chartDataArray: ArrayList<Pair<String, Float>> = ArrayList()

    private var activeRangeButton = R.id.d1

    private var chartRange = D1

    private var chartInterval = M_15

    private lateinit var cryptoData: CryptoModel

    private var isMainCoinSelected = false

    private var debounceJob: Job? = null

    override fun getViewBinding() = ActivityCoinDetailsBinding.inflate(layoutInflater)



    override fun onActivityCreated() {


        viewModel = getViewModel()

        cryptoData = intent.getSerializableExtra(SHARED_EXTRA) as CryptoModel

        cryptoModel.value = cryptoData

        isMainCoinSelected =  cryptoData.coin.uppercase()== mainCoin.uppercase()

        fetchChartData()

        startSubscribeWebSocket()

        viewModel.connectSocket(viewModel)

        chartDataResponse()

        binanceStreamsListener()



    }




    @SuppressLint("SetTextI18n")
    override fun configUi() {

        if (getUser()?.user?.hasFiatAccount == false){

            binding.buttonsLayout.gone()

        }else{

            binding.buttonsLayout.show()

        }

        binding.topBar.set(context = this,title = cryptoData.coin)

        binding.coinName.text = cryptoData.name

        if (!cryptoData.balance.isNullOrEmpty()) {

             binding.buy.show()

             binding.sell.show()

             viewModel.fetchExchangeInfo(fromAsset = cryptoData.coin)


        } else {


           binding.buy.show()

           binding.sell.gone()


        }

        updateTopInfo()

    }

    private fun startSubscribeWebSocket(){

        viewModel.isConnectedSocket.observe(this){

            if (it==true){

               coinDetailsWebSocket = webSocket

                subscribe()
            }


        }

    }

    private fun subscribe(){

        viewModel.socketListenTo( viewModel.coiPriceChangeStatistics(method = UNSUBSCRIBE, params = createPriceChangePercentParam(coinList = arrayListOf(cryptoData))),coinDetailsWebSocket)

        viewModel.socketListenTo( viewModel.mainCoinToEuro(method = UNSUBSCRIBE),coinDetailsWebSocket)

        viewModel.socketListenTo( viewModel.coiPriceChangeStatistics(method = SUBSCRIBE, params = createPriceChangePercentParam(coinList = arrayListOf(cryptoData))),coinDetailsWebSocket)

        viewModel.socketListenTo( viewModel.mainCoinToEuro(method = SUBSCRIBE),coinDetailsWebSocket)
    }


    override fun onDestroy() {

        super.onDestroy()

        coinDetailsWebSocket?.cancel()
    }


    private fun binanceStreamsListener(){

        viewModel.socketResponse.observe(this){


                try{

                    with(it.toString()){

                        when{

                            contains(BinanceStreams.PRICE_AND_PERCENT_EVENT) -> liveChangePriceAndPercent(stream = Gson().fromJson(it, CryptoToCryptoEventModel::class.java))

                        }

                    }

                }catch (e:Exception){ Unit }



        }

    }


    @SuppressLint("SetTextI18n")
    private fun liveChangePriceAndPercent(stream: CryptoToCryptoEventModel){

           try {
               if (stream.data?.symbol == ACCOUNT_CURRENCY + mainCoin.uppercase()) {

                   mainCoinToEur = 1 / stream.data.lastPrice.toBigDecimal().toDouble()


               }else{

                   cryptoData.balanceInFiat = if (cryptoData.coin.uppercase()== mainCoin.uppercase()) mainCoinToEur.toString() else stream.data?.lastPrice?:"0.0"

                   cryptoData.percent = stream.data?.priceChangePercent?:0.0

                   cryptoModel.value = cryptoData

                   updateTopInfo()

               }


           }catch (e:Exception){ Unit }


    }

    @SuppressLint("SetTextI18n")
    private fun updateTopInfo(){

        var amount = correctBalanceInEuro(balanceInFiat =
        if (cryptoData.coin.uppercase() == mainCoin.uppercase()) {

            mainCoinToEur.toString()

        } else{
            cryptoData.balanceInFiat

        },isMaiCoinSelected = isMainCoinSelected)

        amount = numberFormatString(amount.formatStringToDouble().toCorrectString(),PRECISION_8)

        binding.amount.text = "${CryptoSymbol.CURRENCY_SYMBOL} $amount"

        val percent = if (cryptoData.coin.uppercase() == mainCoin.uppercase()) mainCoinToEurPercent else cryptoData.percent

        binding.totalReturnFromTime.also {

            if(percent > 0){

                it.setTextColor(GREEN_TEXT)

                it.text = "+${percent.roundingDown(scale = 2)}%"

            }else{

                it.text = "${percent.roundingDown(scale = 2)}%"

                it.setTextColor(RED)
            }


        }

    }


    override fun clickListener(bind: ActivityCoinDetailsBinding) {

        bind.infoBtn.setOnClickListener {

            showActionDialog(context = this, title = "Information", subTitle = "Coin price change percentage during the past 24 hours", isSingleButton = true, posBtnName = "Ok" )

        }

        bind.buy.setOnClickListener {


               openBottomSheet(fragment = BuyAndSellCoin(data = BuyOrSellSharedModel(crypto = cryptoData, type = BUY),goToCrypto = ::goToCrypto))


        }

        bind.sell.setOnClickListener {

          openBottomSheet(fragment = BuyAndSellCoin(data = BuyOrSellSharedModel(crypto = cryptoData, type = SELL),goToCrypto = ::goToCrypto))



        }


        bind.d1.setOnClickListener {

            chartRange = D1

            chartInterval = M_15

            rangeButton(clickedRangeButtonId = R.id.d1)

            fetchChartData()
        }

        bind.w1.setOnClickListener {

            chartRange = W1

            chartInterval = H_1

            rangeButton(clickedRangeButtonId = R.id.w1)

            fetchChartData()
        }

        bind.m1.setOnClickListener {

            chartRange = M1

            chartInterval = H_4

            rangeButton(clickedRangeButtonId = R.id.m1)

            fetchChartData()
        }

        bind.m3.setOnClickListener {

            chartRange = M3

            chartInterval = D_1

            rangeButton(clickedRangeButtonId = R.id.m3)

            fetchChartData()
        }

        bind.m6.setOnClickListener {

            chartRange = M6

            chartInterval = D_1

            rangeButton(clickedRangeButtonId = R.id.m6)

            fetchChartData()
        }

        bind.y1.setOnClickListener {

            chartRange = Y1

            chartInterval = D_1

            rangeButton(clickedRangeButtonId = R.id.y1)

            fetchChartData()
        }

    }

    private fun chartDataResponse() {

        viewModel.chartResponse.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS -> {


                        chartDataArray.clear()

                        it.data?.forEachIndexed { index, data ->


                            chartDataArray.add("$index" to doubleBigDecimal((data[4].toDouble() * mainCoinToEur).toString()).toFloat())

                        }

                        binding.statisticsChart.animation.duration = 500L

                        binding.statisticsChart.animate(chartDataArray)


                        if (binding.chartLoading.isVisible){

                            stopCartLoading()
                        }


                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::fetchChartData
                    )

                    else -> {

                        showMessageDialog(title = cryptoData.coin, it.message)
                    }
                }
            } catch (e: Exception) {

                showMessageDialog(title = cryptoData.coin, it.message)

            } finally {

            }
        }
    }

    private fun singleCoinResponse() {

        viewModel.singleCoinResp.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS -> {

                        cryptoData.balance = it.data?.data?.balance.toString()

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::fetchSingleCoin
                    )

                    else -> {

                        showMessageDialog(title = cryptoData.coin, it.message)
                    }
                }
            } catch (e: Exception) {

                showMessageDialog(title = cryptoData.coin, it.message)

            } finally {
                loading.dismiss()
            }
        }
    }



    private fun rangeButton(clickedRangeButtonId: Int) {

        if (clickedRangeButtonId != activeRangeButton) {

            findViewById<AppCompatTextView>(activeRangeButton).also {
                it.setBackgroundResource(0)
                it.setTextColor(DARK_TEXT_COLOR)
            }
            findViewById<AppCompatTextView>(clickedRangeButtonId).also {
                it.setBackgroundResource(R.drawable.black_20_dp_radius)
                it.setTextColor(AppColors.WHITE)
            }
            activeRangeButton = clickedRangeButtonId
        }
    }


    private fun fetchSingleCoin() {

        loading.show()

        viewModel.fetchSingleCoin(coin = cryptoData.coin)

    }

    private fun fetchChartData() {

        binding.chartLoading.show(chartDataArray.isEmpty())

        debounceJob?.cancel()

        debounceJob = CoroutineScope(Dispatchers.Main).launch {

            delay(300)

            val coin = if (cryptoData.coin.uppercase() == mainCoin.uppercase()) ACCOUNT_CURRENCY else  cryptoData.coin

            viewModel.fetchCartData(symbol = coin + mainCoin.uppercase(), range = chartRange, interval = chartInterval)
        }

    }


    private fun goToCrypto(goToCrypto: Boolean){

        if (goToCrypto){

            goToHomeTab = false

            onBackPressed()

        }
    }

    override fun onResume() {

        super.onResume()





        if (goToHomeTab){

            goToHomeTab = false

            onBackPressed()
        }
        if (isUpdateMyCryptoBalance){

            singleCoinResponse()

            fetchSingleCoin()

        }
    }


    private  fun stopCartLoading(){

        binding.chartLoading.gone()

    }


    class ChartAdapter(val chartData:ArrayList<Float>) : SparkAdapter() {

        override fun getCount(): Int {
            return chartData.size
        }

        @NonNull
        override fun getItem(index: Int): Any {
            return chartData[index]
        }

        override fun getY(index: Int): Float {
            return chartData[index]
        }
    }



}

@Parcelize
data class ConvertData(

    var selectedCrypto:CryptoModel,

    var cryptoList:ArrayList<CryptoModel>

): Serializable, Parcelable
