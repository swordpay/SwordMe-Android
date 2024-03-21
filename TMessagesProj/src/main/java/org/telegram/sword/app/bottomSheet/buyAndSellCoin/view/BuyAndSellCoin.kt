package org.telegram.sword.app.bottomSheet.buyAndSellCoin.view

import android.annotation.SuppressLint
import android.content.DialogInterface
import android.content.res.ColorStateList
import android.graphics.Color
import android.graphics.Typeface
import android.view.View
import org.koin.androidx.viewmodel.ext.android.getViewModel
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentBuyAndSellCoinBinding
import org.telegram.sword.app.bottomSheet.buyAndSellCoin.model.BuyOrSellSharedModel
import org.telegram.sword.app.common.AppConst.*
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_SUCCESS
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.common.base.DialogStatus
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.extansion.ui.*
import org.telegram.sword.app.common.extansion.ui.topBar.set
import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.common.helper.function.closeKeyboard
import org.telegram.sword.app.common.helper.function.showKeyboard
import org.telegram.sword.app.common.helper.function.showNoInetDialog
import org.telegram.sword.app.common.helper.ui.SwordTextField
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.cryptoModel
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoin
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEur
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.helper.correctBalanceInEuro
import org.telegram.sword.app.home.tabs.cryptoAccount.state.isUpdateMyCryptoBalance
import org.telegram.sword.app.home.tabs.cryptoAccount.state.isUpdateMyFiatBalance
import org.telegram.sword.app.home.tabs.cryptoAccount.state.transactionsPending
import org.telegram.sword.domain.binance.model.BuyOrSellRequestModel
import org.telegram.sword.domain.binance.model.TradeInfo
import org.telegram.sword.domain.common.Status
import org.telegram.sword.sendOrRequest.MoneyTab
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.CURRENCY_SYMBOL

class BuyAndSellCoin(private val data: BuyOrSellSharedModel, private val goToCrypto:(goToCrypto: Boolean) -> Unit) :
    BaseBottomSheet<FragmentBuyAndSellCoinBinding, BinanceAccountViewModel>(R.layout.fragment_buy_and_sell_coin, isDraggable = true) {

    private var approximately =  "≈"

    private  var   def: ColorStateList? = null

    private  var   selectedTab: MoneyTab = MoneyTab.CRYPTO

    private lateinit var tradeInfo: TradeInfo

    private   var cryptoData = data.crypto

    private var balanceInEuro = "0.0"

    private var balance = 0.0

    private var isMainCoinSelected = false

    var minInEur = 0.0

    var maxInEur = 0.0

    var minInCrypto = 0.0

    var maxInCrypto = 0.0

    var minInEurString = EMPTY

    var maxInEurString = EMPTY

    var minInCryptoString = EMPTY

    var maxInCryptoString = EMPTY

    var fiatPrecision = PRECISION_2

    var cryptoPrecision = PRECISION_8


    lateinit var amountEditText:SwordTextField

    override fun getViewBinding(view: View) = FragmentBuyAndSellCoinBinding.bind(view)

    override fun onBottomSheetCreated(view: View) {

        amountEditText = binding.coinCountTextField

        viewModel = getViewModel()

        isMainCoinSelected =  cryptoData.coin .uppercase() == mainCoin.uppercase()

        tradeResponse()

        fetchTradeInfo()

        buyOrSellResponse()

        cryptoTextFieldAddChangeListener()
    }

    @SuppressLint("SetTextI18n")
    override fun configUi() {

        setupSegmentTab()

        amountEditText.startIcon.setImageResource(R.drawable.euro_icon)

        binding.topBar.set ( title = data.crypto.name )

        binding.buyOrSellBtn.set(name = data.type)

        amountEditText.requestFocus()

        binding.myBalanceLayout.show(cryptoData.balance !=null)

        if (cryptoData.balance !=null){

            binding.myBalanceTitle.text = resources.getString(R.string.yourBalance) + SPACE

            balance = doubleBigDecimal(cryptoData.balance.toString()).formatStringToString().toDouble()

            balanceInEuro =  correctBalanceInEuro(balanceInFiat = if (cryptoData.coin .uppercase()== mainCoin.uppercase()) mainCoinToEur.toString() else

            cryptoData.balanceInFiat, coinCount = cryptoData.balance,isMaiCoinSelected = isMainCoinSelected).formatStringToString()

            binding.myBalance.text = numberFormatString(doubleBigDecimal(cryptoData.balance.toString()), cryptoPrecision )+

            SPACE +cryptoData.coin +" $approximately "+ numberFormatString(balanceInEuro, fiatPrecision)   + " $ACCOUNT_CURRENCY"

        }


        showKeyboard(context = requireContext())

    }

    @SuppressLint("SetTextI18n")
    private fun cryptoModelObserver(){

        cryptoModel.observe(this){

            cryptoData = it

            if (!cryptoData.balance.isNullOrEmpty()){

                balance = doubleBigDecimal(cryptoData.balance.toString()).toDouble()

                balanceInEuro =  correctBalanceInEuro(balanceInFiat = if

                        (cryptoData.coin.uppercase()== mainCoin.uppercase()) mainCoinToEur.toString() else

                            cryptoData.balanceInFiat, coinCount = cryptoData.balance,isMaiCoinSelected = isMainCoinSelected)

                binding.myBalance.text = numberFormatString(doubleBigDecimal(cryptoData.balance.toString()),

                    cryptoPrecision) + SPACE +cryptoData.coin +" $approximately "+numberFormatString( balanceInEuro, fiatPrecision)  + " $ACCOUNT_CURRENCY"

            }


            minInCrypto=  doubleBigDecimal(tradeInfo.coin.min).changePrecisionCount(tradeInfo.coin.precision?: cryptoPrecision).toDouble()

            maxInCrypto=  doubleBigDecimal(tradeInfo.coin.max).changePrecisionCount(tradeInfo.coin.precision?: cryptoPrecision).toDouble()


            minInEur =  doubleBigDecimal(tradeInfo.currency.min).changePrecisionCount(fiatPrecision).toDouble()

            maxInEur = doubleBigDecimal(tradeInfo.currency.max).changePrecisionCount(fiatPrecision).toDouble()



            if (minInEur < 1.0){

                minInEur = 1.0
            }

            if (minInEur > 1000000){

                minInEur = 1000000.0
            }



            if (maxInEur < 1.0){

                maxInEur = 1.0
            }

            if (maxInEur > 1000000){

                maxInEur = 1000000.0
            }


            minInCryptoString=  minInCrypto.toCorrectString()

            maxInCryptoString=  maxInCrypto.toCorrectString()

            minInEurString =  minInEur.toCorrectString()

            maxInEurString = maxInEur.toCorrectString()

            binding.minimum.text = "${numberFormatString(minInCryptoString,tradeInfo.coin.precision)} ${cryptoData.coin} $approximately ${numberFormatString(minInEurString,

                fiatPrecision)} $ACCOUNT_CURRENCY"

            binding.maximum.text = "${numberFormatString(maxInCryptoString,tradeInfo.coin.precision)} ${cryptoData.coin} $approximately ${numberFormatString(maxInEurString,
                fiatPrecision)} $ACCOUNT_CURRENCY"


            try {

                val amount = amountEditText.text().formatStringToString()

                var convertedAmount = "0"

                var convertedAmountString = "0"

                if (amount.isEmpty()){

                    binding.convertedAmount.gone()

                    binding.buyOrSellBtn.isEnabled(isEnable = false)

                }else{
                    when (selectedTab){

                        MoneyTab.CRYPTO ->{


                            convertedAmount =  numberFormatString(correctBalanceInEuro(balanceInFiat =  if (cryptoData.coin.uppercase() == mainCoin.uppercase()) mainCoinToEur.toString() else

                                cryptoData.balanceInFiat, coinCount = amount, isMaiCoinSelected = isMainCoinSelected), fiatPrecision)

                            convertedAmountString = approximately+ SPACE + CURRENCY_SYMBOL + SPACE +convertedAmount
                        }




                        else ->{

                            val amountInCrypto = if (cryptoData.coin.uppercase()== mainCoin.uppercase()) mainCoinToEur else correctBalanceInEuro(cryptoData.balanceInFiat.toDouble().toString()).toDouble()

                            convertedAmount = numberFormatString( doubleBigDecimal((( amount.toDouble()/amountInCrypto).toString())), cryptoPrecision)

                            convertedAmountString = approximately+ SPACE + data.crypto.coin+ SPACE + convertedAmount

                        }
                    }

                    if (amount.isNotEmpty() && amount.toDouble() > 0){

                        validation(it = amount.toDouble().toCorrectString().formatStringToString())

                        if (convertedAmount.formatStringToDouble()>0.0){


                            binding.convertedAmount.text = convertedAmountString
                        }else{

                            binding.convertedAmount.gone()
                        }

                    }else{


                        binding.buyOrSellBtn.isEnabled(isEnable = false)
                    }

                }


            }catch (e:Exception){

                binding.buyOrSellBtn.isEnabled(isEnable = false)

                binding.convertedAmount.gone()
            }

        }

    }

    private fun setupSegmentTab(){

        def = binding.amountTypeSegment.lastSegmentName.textColors

        binding.amountTypeSegment.firstSegmentName.text = getString(R.string.crypto)

        binding.amountTypeSegment.lastSegmentName.text = getString(R.string.cash)
    }

    @SuppressLint("SetTextI18n")
    private fun cryptoTextFieldAddChangeListener(){


        amountEditText.setDigits(

            if (selectedTab == MoneyTab.CRYPTO){

                if (cryptoPrecision > 0) "0123456789.," else "0123456789"

            }else{

                "0123456789.,"
            }

        )

        var beforeAmountText = EMPTY

        amountEditText.doBeforeTextChangedObserve.observe(this){

            amountEditText.beforeValidText(

                maxIntegerPartCount = MAX_INTEGER_PART_COUNT,

                maxDecimalPartCount = if (selectedTab == MoneyTab.CASH) fiatPrecision  else cryptoPrecision

            )?.let { validText ->

                beforeAmountText = validText.formatStringToString()

            }
        }

        amountEditText.doOnTextChangedObserve.observe(this){ m->

            val it = m.it
            val index = m.index
            val afterTextCount = m.afterTextCount


            amountEditText.startManipulationForPartCount(

                num = it.toString(),

                maxIntegerPartCount = MAX_INTEGER_PART_COUNT,

                maxDecimalPartCount = if (selectedTab == MoneyTab.CASH)  fiatPrecision  else  cryptoPrecision,

                beforeAmountText = beforeAmountText,

                changIndex = index,

                afterTextCount = afterTextCount

            )


            try {

                val amount = amountEditText.text().formatStringToString().ifEmpty { "0.0" }

                var convertedAmount = "0"

                var convertedAmountString = "0"

                if (amount.isEmpty() || amount.formatStringToDouble()<=0){

                    binding.convertedAmount.gone()

                    binding.buyOrSellBtn.isEnabled(isEnable = false)
                }else{
                    when (selectedTab){

                        MoneyTab.CRYPTO -> {

                            convertedAmount = numberFormatString(correctBalanceInEuro(balanceInFiat =  if (cryptoData.coin.uppercase()== mainCoin.uppercase())

                                mainCoinToEur.toString() else

                                cryptoData.balanceInFiat, coinCount = amount, isMaiCoinSelected = isMainCoinSelected),

                                fiatPrecision)

                            convertedAmountString = approximately+ SPACE + CURRENCY_SYMBOL+ SPACE + convertedAmount

                        }


                        else ->{

                            val convertedAmountInCrypto = if (cryptoData.coin.uppercase()== mainCoin.uppercase()) mainCoinToEur

                            else correctBalanceInEuro(cryptoData.balanceInFiat.toDouble().toString()).toDouble()

                            convertedAmount = numberFormatString(doubleBigDecimal(((  amount.toDouble()/convertedAmountInCrypto).toString())), cryptoPrecision)

                            convertedAmountString =    approximately+ SPACE + data.crypto.coin + SPACE + convertedAmount


                        }
                    }

                    if (amount.isNotEmpty() && amount.toDouble() > 0){

                        validation(it = amount.toDouble().toCorrectString().formatStringToString())

                        if (convertedAmount.formatStringToDouble()>0.0){


                            binding.convertedAmount.text = convertedAmountString
                        }else{

                            binding.convertedAmount.gone()
                        }



                    }else{

                        binding.buyOrSellBtn.isEnabled(isEnable = false)
                    }

                }


            }catch (e:Exception){


                binding.buyOrSellBtn.isEnabled(isEnable = false)

                binding.convertedAmount.gone()
            }

        }

    }


    @SuppressLint("SetTextI18n")
    override fun clickListener(bind: FragmentBuyAndSellCoinBinding) {

        bind.topBar.dismissButton.setOnClickListener {

            closeKeyboard(context = requireContext())

            dismiss()
        }

        bind.amountTypeSegment.firstSegmentTab.setOnClickListener{

            if (selectedTab == MoneyTab.CASH){

                changeTab(tab = MoneyTab.CRYPTO)


            }

        }

        bind.amountTypeSegment.lastSegmentTab .setOnClickListener{

            if (selectedTab == MoneyTab.CRYPTO) {

                changeTab(tab = MoneyTab.CASH)

            }
        }

        bind.buyOrSellBtn.button.setOnClickListener{

            buyOrSellRequest()
        }

        bind.minimum.setOnClickListener {

            amountEditText.setText(

                when(selectedTab){

                    MoneyTab.CRYPTO -> minInCryptoString

                    else -> minInEurString

                }
            )

            amountEditText.setSelection(amountEditText.text().length)
        }

        bind.maximum.setOnClickListener {

            amountEditText.setText(

                when(selectedTab){

                    MoneyTab.CRYPTO -> maxInCryptoString

                    else -> maxInEurString

                }
            )

            amountEditText.setSelection(amountEditText.text().length)
        }


        binding.myBalance.setOnClickListener {

            amountEditText.setText(

                when(selectedTab){

                    MoneyTab.CRYPTO -> doubleBigDecimal(cryptoData.balance?:"0.0")

                    else -> numberFormatString(balanceInEuro, fiatPrecision)

                }
            )

            amountEditText.setSelection(amountEditText.text().length)
        }

    }

    private fun changeTab( tab: MoneyTab){

        selectedTab = tab

        amountEditText.startIcon.show(selectedTab == MoneyTab.CASH)

        when(tab){

            MoneyTab.CRYPTO ->{

                binding.amountTypeSegment.also {

                    it.lastSegmentName.setTextColor(def)
                    it.firstSegmentName.setTextColor(Color.BLACK)
                    it.select.animate().x(0f).duration = 130
                    it.firstSegmentIcon.setHintColor(requireContext(),R.color.appBlue)
                    it.lastSegmentIcon.setHintColor(requireContext(),R.color.hint)
                    it.firstSegmentName.typeface = Typeface.DEFAULT_BOLD
                    it.lastSegmentName.typeface = Typeface.DEFAULT

                }

                amountEditText.setDigits( if (cryptoPrecision > 0)"0123456789.," else "0123456789")

                val amount =  if (cryptoPrecision > 0) amountEditText.text() else amountEditText.text().formatStringToDouble().toLong()

                amountEditText.setText(amount.toString())

                amountEditText.setSelection(amountEditText.text().length)

            }

            MoneyTab.CASH ->{
                binding.amountTypeSegment.also {

                    it.firstSegmentName.setTextColor(def)
                    it.lastSegmentName.setTextColor(Color.BLACK)
                    val size = binding.amountTypeSegment.lastSegmentTab.width
                    it.select.animate().x(size.toFloat()).duration = 130
                    it.lastSegmentIcon.setHintColor(requireContext(),R.color.appBlue)
                    it.firstSegmentIcon.setHintColor(requireContext(),R.color.hint)
                    it.lastSegmentName.typeface = Typeface.DEFAULT_BOLD
                    it.firstSegmentName.typeface = Typeface.DEFAULT
                }

                amountEditText.setDigits("0123456789.,")

                amountEditText.setText(amountEditText.text())

                amountEditText.setSelection(amountEditText.text().length)
            }
        }

    }


    @SuppressLint("SetTextI18n")
    private fun tradeResponse(){

        viewModel.tradeInfo.observe(this){
            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS ->{


                        it.data?.let { info ->

                            tradeInfo = info.info

                            cryptoPrecision = tradeInfo.coin.precision?: PRECISION_8


                        }

                        cryptoModelObserver()

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(requireActivity(), function = :: fetchTradeInfo)

                    else->{

                        showMessageDialog(title = getString(R.string.crypto),it.message, function =:: dismiss)
                    }
                }
            }catch (e:Exception){

                showMessageDialog(title = getString(R.string.crypto),it.message, function =:: dismiss)

            }finally {   binding.bottomSheetLoading.gone() }
        }
    }

    @SuppressLint("SetTextI18n")
    private fun buyOrSellResponse(){

        viewModel.buyOrSellResp.observe(this){
            try {
                when (it.status) {


                    Status.FORCE_UPDATE->{}

                    Status.USER_BLOCKED->{}

                    Status.SUCCESS,Status.VOID_SUCCESS ->{

                            it.data?.let { data_->

                                transactionsPending = true

                                if (!data_.data.redirectUrl.isNullOrEmpty()){

                                    isUpdateMyCryptoBalance = true

                                    isUpdateMyFiatBalance = true

                                    dismiss()

                                    requireActivity().openCustomChrome(url = data_.data.redirectUrl?: EMPTY)

                                }else {


                                    showMessageDialog(title = data.type,

                                        "Your transaction has been completed successfully",

                                        DialogStatus.SUCCESS,

                                        function = {

                                            goToCrypto(true)

                                            isUpdateMyCryptoBalance = true

                                            isUpdateMyFiatBalance = true

                                            dismiss()
                                        }
                                    )

                                }
                            }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(requireActivity(), function = :: fetchTradeInfo)

                    else->{

                        showMessageDialog(title = data.type,"Your transaction has been failed")
                    }
                }
            }catch (e:Exception){

                showMessageDialog(title = data.type,"Your transaction has been failed")

            }finally {   binding.bottomSheetLoading.gone() }
        }
    }



    override fun onDismiss(dialog: DialogInterface) {

        closeKeyboard(context = requireContext())

        super.onDismiss(dialog)
    }

    private fun fetchTradeInfo(){

        binding.bottomSheetLoading.show()

        viewModel.fetchTradeInfo(coin = cryptoData.coin, type = data.type.lowercase())

    }

    private fun buyOrSellRequest(){

        binding.bottomSheetLoading.show()

        viewModel.buyOrSell(type = data.type, requestData = BuyOrSellRequestModel(

            coin = cryptoData.coin,

            amount = amountEditText.text().formatStringToDouble(),

            currency = ACCOUNT_CURRENCY,

            amountType = when(selectedTab){

                MoneyTab.CRYPTO -> "crypto"

                MoneyTab.CASH -> "fiat"

            },
        )
        )
    }

    @SuppressLint("SetTextI18n")
    private fun validation(it:String){

        when (data.type){


            BuyOrSellType.BUY ->{

                when(selectedTab){

                    MoneyTab.CRYPTO ->{

                        (it.toDouble() in minInCrypto .. maxInCrypto).also { isCorrectRange ->

                            binding.buyOrSellBtn.isEnabled(isEnable = isCorrectRange)

                            val errorMessage = if (isCorrectRange){
                                binding.convertedAmount.show()
                                EMPTY

                            }else{

                                if (it.toDouble()< minInCrypto){

                                    minimumAmountErrorMessage(min = minInCryptoString)

                                }else{
                                    maximumAmountErrorMessage(max = maxInCryptoString)

                                }

                            }

                            amountEditText.showError(error =  errorMessage ,status = if (errorMessage.isEmpty()) VALID_SUCCESS else VALID_ERROR)

                        }


                    }

                    else ->{

                                ( it.toDouble() in minInEur ..maxInEur).also { isCorrectRange ->

                                    val errorMessage =  if (isCorrectRange){
                                        binding.convertedAmount.show()

                                        EMPTY

                                        }else{

                                            binding.convertedAmount.gone()

                                            if (it.toDouble()< minInEur){

                                                minimumAmountErrorMessage(min = minInEurString)

                                            }else{
                                                maximumAmountErrorMessage(max = maxInEurString)

                                            }
                                        }


                                    amountEditText.showError(error =  errorMessage, status = if (errorMessage.isEmpty()) VALID_SUCCESS else VALID_ERROR)


                                    binding.buyOrSellBtn.isEnabled(isEnable = isCorrectRange )


                                }



                    }
                }
            }

            BuyOrSellType.SELL ->{

                when(selectedTab){

                    MoneyTab.CRYPTO ->{

                        val cryptoBalance = if (cryptoData.coin.uppercase() == mainCoin.uppercase()){

                           doubleBigDecimal(cryptoData.balance.toString()).toDouble()


                        }else{

                            doubleBigDecimal(cryptoData.balance.toString()).toDouble().toCorrectString().formatStringToDouble()
                        }

                        (it.toDouble() in minInCrypto ..maxInCrypto).also { isCorrectRange ->

                            ( it.toDouble() <= cryptoBalance).also { isCorrectBalanceRange ->


                                val errorMessage = if (isCorrectRange && isCorrectBalanceRange){

                                    binding.convertedAmount.show()
                                    EMPTY



                                }else{
                                    binding.convertedAmount.gone()

                                    if (!isCorrectRange){

                                        if (it.toDouble()< minInCrypto){

                                            minimumAmountErrorMessage(min = minInCryptoString)

                                        }else{
                                            maximumAmountErrorMessage(max = maxInCryptoString)

                                        }

                                    }else{

                                        balanceExceededErrorMessage()
                                    }

                                }


                                amountEditText.showError(error =  errorMessage, status = if (errorMessage.isEmpty()) VALID_SUCCESS else VALID_ERROR)

                                binding.buyOrSellBtn.isEnabled(isEnable = isCorrectRange && isCorrectBalanceRange)

                            }
                        }

                    }

                    else ->{

                        val cryptoBalance = if (cryptoData.coin.uppercase() == mainCoin.uppercase())

                            (doubleBigDecimal(cryptoData.balance.toString()).toDouble()) * doubleBigDecimal(mainCoinToEur.toString()).toDouble().toCorrectString().formatStringToDouble()

                        else doubleBigDecimal(balanceInEuro).toDouble().toCorrectString().formatStringToDouble()

                            (it.toDouble() <= cryptoBalance).also { isCorrectBalanceRange->

                                    ( it.toDouble() in minInEur ..maxInEur).also { isCorrectRange ->

                                        val errorMessage = if (isCorrectRange && isCorrectBalanceRange){

                                            binding.convertedAmount.show()

                                            EMPTY

                                        }else{
                                            binding.convertedAmount.gone()

                                            if (!isCorrectRange){
                                                if (it.toDouble()< minInEur){

                                                    minimumAmountErrorMessage(min = minInEurString)

                                                }else{
                                                    maximumAmountErrorMessage(max = maxInEurString)

                                                }

                                            }else{

                                                balanceExceededErrorMessage()
                                            }

                                        }


                                        amountEditText.showError(error =  errorMessage, status = if (errorMessage.isEmpty()) VALID_SUCCESS else VALID_ERROR)

                                        binding.buyOrSellBtn.isEnabled(isEnable = isCorrectRange && isCorrectBalanceRange)


                                    }


                            }

                    }

                }

            }
        }
    }
}