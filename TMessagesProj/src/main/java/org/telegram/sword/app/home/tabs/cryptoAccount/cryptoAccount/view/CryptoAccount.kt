package org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.view

import android.annotation.SuppressLint
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import org.koin.androidx.viewmodel.ext.android.getViewModel
import org.telegram.messenger.R
import org.telegram.messenger.databinding.CryptoAccountBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.PRECISION_8
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.helper.function.isConnectedToInternet
import org.telegram.sword.app.common.helper.function.showNoInetDialog
import org.telegram.sword.app.home.tabs.cryptoAccount.coinDetails.view.CoinDetails
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.adapter.CryptoListAdapter
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.*
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.helper.correctBalanceInEuro
import org.telegram.sword.app.home.tabs.cryptoAccount.state.isUpdateMyCryptoBalance
import org.telegram.sword.app.home.tabs.cryptoAccount.state.pendingUpdateFetchCount
import org.telegram.sword.app.home.tabs.cryptoAccount.state.transactionsPending
import org.telegram.sword.domain.binance.model.CryptoCryptoModel
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.domain.common.Status
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.model.response.CryptoToCryptoEventModel


class CryptoAccount : BaseActivity<CryptoAccountBinding, BinanceAccountViewModel>(){

    private var initialCryptoToMainCoinList: HashMap<String, CryptoCryptoModel> = HashMap()

    private var visibilityItems:ArrayList<CryptoModel> = ArrayList()

    private var searchAssetsList:ArrayList<CryptoModel> = ArrayList()

    private  var  cryptoListAdapter: CryptoListAdapter?=null

    private lateinit var layoutManager:LinearLayoutManager

    private var pageIsCreated = false

    private var binanceOauthCode:String?=null


    companion object{

        var isFinishCryptoPage = false

    }


    override fun getViewBinding() = CryptoAccountBinding.inflate(layoutInflater)

    override fun onActivityCreated() {

        viewModel = getViewModel()

        binanceOauthCode = intent?.getStringExtra("BinanceCode")

        isUpdateMyCryptoBalance = false

        cryptoList.clear()

        fetchCryptoAccount()

        binding.cryptoSearchView.swordSearchView.isEnabled = false

        binding.cryptoSearchView.swordSearchView.isClickable = false


    }

    private fun fetchCryptoAccount(){

        cryptoBalanceResponse()

        cryptoAssetsResponse()

        initialCryptoToMainCoinResponse()

        swipeRefresh()

        fetchCryptoBalance()

        searchCryptoObserve()

        pageIsCreated = true
    }


    override fun configUi() {

        binding.shimmerLayout.startShimmer()
        binding.shimmerLayout.show()
        binding.balanceShimmer.startShimmer()
        binding.balanceShimmer.show()
        binding.cryptoBalance.hide()


    }

    override fun clickListener(bind: CryptoAccountBinding) {

        bind.onBackBtn.setOnClickListener {

            onBackPressed()
        }

    }


    private fun searchCryptoObserve(){

        binding.cryptoSearchView.searchText.observe(this){

            cryptoListAdapter?.filter?.filter(it.trim())
        }
    }


    private fun liveUpdate(index:Int, crypto:CryptoModel, stream:CryptoToCryptoEventModel){


        if (stream.data?.symbol == crypto.coin.uppercase()+mainCoin.uppercase()){

            if (stream.data.symbol == ACCOUNT_CURRENCY + mainCoin.uppercase()) {

                try {
                    mainCoinToEurPercent =  stream.data.priceChangePercent

                    mainCoinToEur = 1 /  stream.data.lastPrice.toBigDecimal().toDouble()

                    updateMyBalance()

                }catch (e:Exception){ Unit }


            }
            val viewHolder = binding.cryptoListView.findViewHolderForAdapterPosition(index) as CryptoListAdapter.CryptoViewHolder

            crypto.percent = stream.data.priceChangePercent

            crypto.balanceInFiat = stream.data.lastPrice

            viewHolder.updatePriceAndPercent(balanceInFiat =  crypto.balanceInFiat, percent = crypto.percent,coin=crypto.coin)

        }

    }


    private fun connectToPriceChangeEvent(){

        visibilityItems.clear()

        visibilityItems.addAll( if (searchAssetsList.isNotEmpty()) searchAssetsList else cryptoList)

    }

    private fun swipeRefresh() {

        binding.swipeRefreshCryptoList.setOnRefreshListener {


                if (isConnectedToInternet(this)){

                    viewModel.fetchCryptoAssets()

                }else{

                    showNoInetDialog(this, function = { viewModel.fetchCryptoAssets() })
                }

        }
    }


    private fun cryptoBalanceResponse(){

        viewModel.cryptoBalance.observe(this){

            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS ->{

                        myBalanceInMainCoin = 0.0

                        updateMyBalance()

                        fetchCryptoAssets()
                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(this, function = :: fetchCryptoBalance)

                    else->{

                        showMessageDialog(title = getString(R.string.crypto),it.message)
                    }
                }
            }catch (e:Exception){

                showMessageDialog(title = getString(R.string.crypto),it.message)

            }finally {

            }
        }
    }

    private fun cryptoAssetsResponse(){

        viewModel.cryptoAssets.observe(this){

            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS ->{


                        it.data?.let { resp ->


                            cryptoList = viewModel.comBainCryptoList(responseData = resp,addHeaders = true)
                        }


                        it.data?.data?.mainCoin?.let { baseCoin -> mainCoin = baseCoin }

                        if (cryptoList.isNotEmpty()){

                            fetchCryptoPriceToMainCoin()
                        }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(this, function = :: fetchCryptoAssets )

                    else->{

                        showMessageDialog(title = getString(R.string.crypto),it.message)
                    }
                }
            }catch (e:Exception){

                showMessageDialog(title = getString(R.string.crypto),it.message)

            }finally {  binding.swipeRefreshCryptoList.isRefreshing = false }
        }
    }


    @SuppressLint("NotifyDataSetChanged")
    private fun initialCryptoToMainCoinResponse(){

        viewModel.cryptoToCryptoResp.observe(this){

            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS ->{

                        try {

                            initialCryptoToMainCoinList.clear()

                            it.data?.map { crypto ->


                                initialCryptoToMainCoinList[crypto.symbol] = crypto

                            }


                            mainCoinToEurPercent = initialCryptoToMainCoinList[ACCOUNT_CURRENCY+mainCoin.uppercase()]?.priceChangePercent ?:0.0

                            mainCoinToEur = 1 /  ( initialCryptoToMainCoinList[ACCOUNT_CURRENCY+mainCoin.uppercase()]?.correctPrice() ?:0.0).toBigDecimal().toDouble() //1.0 /

                            updateMyBalance()

                            (0 until cryptoList.size).forEach {  index ->

                                if (cryptoList[index].coin.isNotEmpty()){


                                    cryptoList[index].balanceInFiat = (initialCryptoToMainCoinList[cryptoList[index].coin.uppercase()+mainCoin.uppercase()]?.correctPrice()?:"0.0").toString()

                                    cryptoList[index].percent = initialCryptoToMainCoinList[cryptoList[index].coin.uppercase()+mainCoin.uppercase()]?.priceChangePercent?:0.0

                                }

                            }

                            if (transactionsPending && pendingUpdateFetchCount < 4){


                                pendingUpdateFetchCount++

                                viewModel.fetchCryptoBalance(currency = ACCOUNT_CURRENCY)

                            }else{

                                pendingUpdateFetchCount = 0

                                transactionsPending = false
                            }

                            if (cryptoListAdapter!=null){

                                cryptoListAdapter?.updateAssets(binding.cryptoSearchView.getText().trim())

                            }else{

                                setupCoinList()
                            }

                        }catch (e:Exception){

                        }



                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(this, function = :: fetchCryptoAssets )

                    else->{

                        showUsaBlockerScreen()

                    }
                }
            }catch (e:Exception){

              showUsaBlockerScreen()

            }finally {  binding.swipeRefreshCryptoList.isRefreshing = false }
        }
    }




    private fun setupCoinList() {

         try {

             layoutManager =  LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false).also {

                 binding.cryptoListView.layoutManager = it

                     cryptoListAdapter = CryptoListAdapter( cryptoAssets = cryptoList, itemClick = :: cryptoListItemClick,

                         isShowPrice = true,searchResultIsEmpty =::searchResultIsEmpty)

                 try {
                     binding.cryptoListView.adapter = cryptoListAdapter

                     binding.shimmerLayout.stopShimmer()
                     binding.shimmerLayout.gone()
                     binding.balanceShimmer.stopShimmer()
                     binding.balanceShimmer.gone()
                     binding.cryptoBalance.show()

                     binding.cryptoSearchView.swordSearchView.isEnabled = true
                     binding.cryptoSearchView.swordSearchView.isClickable = true

                     if (viewModel.timer ==null && viewModel.loopTimer ==null){

                         viewModel.startFetchCryptoBalanceTimer()

                     }

                 }catch (_:Exception){ }



             }
         }catch (_:Exception){ }


        connectToPriceChangeEvent()


    }

   private fun cryptoListItemClick(coin: CryptoModel){

     openActivity(activity = CoinDetails(), sharedExtraData = coin)
   }


    override fun onResume() {

        super.onResume()

        if (viewModel.timer ==null && viewModel.loopTimer ==null){

            viewModel.startFetchCryptoBalanceTimer()

        }


    }

    override fun onPause() {

        super.onPause()

        viewModel.stopLoopTimer()

        viewModel.stopTimer()


    }


    private fun fetchCryptoBalance(){


        viewModel.fetchCryptoBalance(currency = ACCOUNT_CURRENCY)

    }

    private fun fetchCryptoPriceToMainCoin(){

         var symbol = EMPTY

         cryptoList.forEachIndexed { index, cryptoModel ->

          if (cryptoModel.coin.isNotEmpty() && cryptoModel.coin.uppercase()!=mainCoin.uppercase()){

                symbol+= when(index){

                    cryptoList.size-1 ->{

                        "\""  +cryptoList[index].coin.uppercase()+ cryptoModel.mainCoin+"\"]"
                    }

                    else ->{

                        "\"" + cryptoList[index].coin.uppercase()+ cryptoModel.mainCoin +"\""+','

                    }
                }

            }
        }

        if (!symbol.endsWith("]")){

            val correctSymbol = symbol.substring(0,symbol.length-1)

            symbol = "$correctSymbol]"
        }

        viewModel.fetchCryptoToCryptoInfo(symbols = "[\"$ACCOUNT_CURRENCY${mainCoin.uppercase()}\",$symbol")

    }

    private fun fetchCryptoAssets(){

        viewModel.fetchCryptoAssets()

    }


    @SuppressLint("SetTextI18n")
    private fun updateMyBalance(){

        var balance = numberFormatString((correctBalanceInEuro(balanceInFiat = myBalanceInMainCoin.toString())),PRECISION_8)

        balance = numberFormatString(balance.formatStringToDouble().roundingTo2ScaleOrFull().toCorrectString(),PRECISION_8)

        binding.cryptoBalance.text =  "${CryptoSymbol.CURRENCY_SYMBOL} $balance"

    }

    private fun searchResultIsEmpty(cryptoList: ArrayList<CryptoModel>){

        searchAssetsList.clear()

        searchAssetsList.addAll(cryptoList)

        connectToPriceChangeEvent()

    }

    private fun showUsaBlockerScreen(){

        binding.run {

            cryptoSearchView.gone()

            usaBlockerTitle.show()

            UsaBlockerView.show()

            cryptoView.gone()

        }

    }

}