package org.telegram.sword.sendOrRequest

import android.annotation.SuppressLint
import android.os.Parcelable
import androidx.core.widget.doBeforeTextChanged
import androidx.core.widget.doOnTextChanged
import com.google.gson.Gson
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.koin.androidx.viewmodel.ext.android.getViewModel
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.telegram.messenger.*
import org.telegram.messenger.UserObject.isUserSelf
import org.telegram.messenger.databinding.FragmentPayOrRequestBinding
import org.telegram.sword.app.bottomSheet.cashOrCrypto.CashOrCrypto
import org.telegram.sword.app.bottomSheet.cryptoList.view.CryptoList
import org.telegram.sword.app.common.AppConst.*
import org.telegram.sword.app.common.AppConst.BalanceType.ALL
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.base.DialogStatus
import org.telegram.sword.app.common.base.getLocalizeMessage
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.extansion.pixelRatio.setCorrectTextSize
import org.telegram.sword.app.common.extansion.ui.*
import org.telegram.sword.app.common.extansion.ui.topBar.set
import org.telegram.sword.app.common.helper.db.getUser
import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.common.helper.function.*
import org.telegram.sword.app.home.tabs.chat.viewModel.ChatViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.cryptoList
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoin
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEur
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.view.CryptoAccount
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.state.*
import org.telegram.sword.app.home.tabs.fiatAccount.account.viewModel.FiatAccountViewModel
import org.telegram.sword.domain.account.model.FiatAndCryptoBalanceResponseModel
import org.telegram.sword.domain.binance.model.CryptoCryptoModel
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.domain.binance.model.ExchangeInfoModel
import org.telegram.sword.domain.chat.model.*
import org.telegram.sword.domain.common.Status
import org.telegram.sword.socket.webSocket.const_.BinanceStreams
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.ACCOUNT_CURRENCY
import org.telegram.sword.socket.webSocket.model.response.CryptoToCryptoEventModel
import org.telegram.tgnet.TLRPC
import org.telegram.ui.Components.AvatarDrawable
import org.telegram.ui.Components.BackupImageView
import org.telegram.ui.ContactsActivity
import org.telegram.ui.DialogsActivity
import java.io.File
import java.io.Serializable
import java.math.BigInteger
import java.util.*


enum class MoneyTab {
    CASH, CRYPTO
}

data class PayOrRequestData(

    var messageId: Int? = null,

    var members: ArrayList<Users>? = null,

    var room: Rooms? = null,

    var requestData: ChatMessage? = null,

    var groupName: String? = null,

    var groupAvatar: File? = null
)


@Parcelize
data class TeleUserInfo(
    @SerializedName("accept") var accept: AcceptRequest? = null,
    @SerializedName("id") var id: Long? = null,
    @SerializedName("username") var username: String? = null,
    @SerializedName("firstName") var firstName: String? = null,
    @SerializedName("lastName") var lastName: String? = null,
    @SerializedName("accessHash") var access_hash: String? = null,
    @SerializedName("type") var type: String? = null,
    @SerializedName("bot") var bot: Boolean? = null,


    ) : Parcelable, Serializable


class SendOrRequest : BaseActivity<FragmentPayOrRequestBinding, FiatAccountViewModel>() {

    private var currentAccount = 0

    private var teleUser: TeleUserInfo? = null

    private var isAccept: Boolean = false

    private var currentUserName = EMPTY

    private var firstAvatarImageView: BackupImageView? = null

    private var centerAvatarImageView: BackupImageView? = null

    private var lastAvatarImageView: BackupImageView? = null

    private lateinit var firstAvatarDrawable: AvatarDrawable

    private lateinit var centerAvatarDrawable: AvatarDrawable

    private lateinit var lastAvatarDrawable: AvatarDrawable

    companion object {

        var addUsers: ArrayList<Users> = ArrayList()

    }

    private var isOpenInChat: Boolean = false

    private var requestData: ChatMessage? = null

    private lateinit var data: PayOrRequestData

    private var message: ((data: ChatMessage) -> Unit)? = null

    private var backInCreateGroup: (() -> Unit)? = null

    private var dismissEvent: ((gotoHome: Boolean) -> Unit)? = null

    private val binanceViewModel by viewModel<BinanceAccountViewModel>()

    private lateinit var accountsInfo: FiatAndCryptoBalanceResponseModel

    private var myCryptoList: ArrayList<CryptoModel> = ArrayList()

    private var allCrypto: ArrayList<CryptoModel> = ArrayList()

    private var filterExchangeInfoList: ArrayList<CryptoModel> = ArrayList()

    private lateinit var exchangeInfoList: ArrayList<ExchangeInfoModel>

    private var selectedCoin: CryptoModel = CryptoModel()

    private var oldSelectedCoin: CryptoModel = CryptoModel()

    private var activeTab: MoneyTab = MoneyTab.CASH

    private var isFirstRequest = true

    private var initialCryptoToMainCoinList: HashMap<String, CryptoCryptoModel> = HashMap()

    private var selectedCoinToRequestCoin = 0.0

    private var requestCoinToMainCoin = 0.0

    private var selectedCoinToMainCoin = 0.0

    private var selectedCoinToEur = 0.0

    private var requestCoinExchangeInfo: ExchangeInfoModel? = null

    private var payOrRequestType = "pay"

    private var currencyType = ACCOUNT_CURRENCY

    private var members: ArrayList<Int> = ArrayList()

    private val chatViewModel by viewModel<ChatViewModel>()

    private var userBalanceType = ALL

    private var oldCryptoAmount: Double = 0.0

    private var isOpenMinimumAmountDialog = false

    private var isGoToConsent = false

    private var amountType = "fiat"

    private var allUsersChat: ArrayList<Users> = ArrayList()

    private var clickPay: Boolean = false

    private var KYC_VERIFIED = false

    private var IS_USA_USER_LOCATION = false


    override fun getViewBinding() = FragmentPayOrRequestBinding.inflate(layoutInflater)


    @SuppressLint("SetTextI18n")
    override fun onActivityCreated() {


        try {

            if (DialogsActivity.onlineContactList.isEmpty() || DialogsActivity.allContactList.isEmpty()) {
                ContactsController.getInstance(UserConfig.selectedAccount).loadContacts(false, 0)
                ContactsController.getInstance(UserConfig.selectedAccount).forceImportContacts()
            }

        } catch (e: java.lang.Exception) {

        }


        try {


            KYC_VERIFIED = getUser()?.user?.status.toString() == KycStatus.VERIFIED

        } catch (e: Exception) {
        }


        addUsers.clear()

        teleUser = intent.getSerializableExtra("SharedExtraData") as TeleUserInfo?

        isAccept = teleUser?.accept != null

        if (isAccept) {

            binding.payOrRequestTextField.isFocusable = false

            if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {

                binding.payOrRequestTextField.setText(

                    teleUser?.accept?.amount.toString()
                )
            } else {


            }

            binding.senderRequestAmount.text = if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {

                ACCOUNT_CURRENCY + SPACE +

                        numberFormatString(
                            teleUser?.accept?.amount.toCorrectString().formatStringToString(),
                            PRECISION_2
                        )

            } else {

                teleUser?.accept?.currencyType + SPACE +

                        numberFormatString(
                            teleUser?.accept?.amount.toCorrectString().formatStringToString(),
                            PRECISION_8
                        )
            }


            teleUser.also {

                it?.type = it?.accept?.peer?.type
                it?.access_hash = it?.accept?.peer?.accessHash
                it?.id = it?.accept?.peer?.peerId

            }

        }

        currentAccount = UserConfig.selectedAccount

        isOpenInChat = false

        requestData = null

        val datas = PayOrRequestData()

        data = datas

        message = null

        backInCreateGroup = null

        dismissEvent = null

        viewModel = getViewModel()

        setupUsersInfo()

        balanceResponse()

        payOrRequestResponse()

        cryptoAssetsResponse()

        exchangeInfoResponse()

        acceptResponse()

        fetchBalance()

        initialCryptoToMainCoinResponse()

        priceWriteListener()

    }

    fun setTeleUserAvatar(
        avatarImageView: BackupImageView?,
        avatarDrawable: AvatarDrawable,
        user: TLRPC.User?,
        showSelf: Boolean
    ) {
        avatarDrawable.setInfo(user as TLRPC.User)
        if (UserObject.isReplyUser(user as TLRPC.User)) {
            avatarDrawable.avatarType = AvatarDrawable.AVATAR_TYPE_REPLIES
            avatarDrawable.setScaleSize(.8f)
            avatarImageView?.setImage(null, null, avatarDrawable, user)
        } else if (isUserSelf(user as TLRPC.User) && !showSelf) {
            avatarDrawable.avatarType = AvatarDrawable.AVATAR_TYPE_SAVED
            avatarDrawable.setScaleSize(.8f)
            avatarImageView?.setImage(null, null, avatarDrawable, user)
        } else {
            avatarDrawable.setScaleSize(1f)
            avatarImageView?.setForUserOrChat(user, avatarDrawable)
        }
    }

    fun setTeleChatAvatar(
        chat: TLRPC.Chat?,
        avatarImageView: BackupImageView?,
        avatarDrawable: AvatarDrawable,
    ) {
        avatarDrawable.setInfo(chat)
        if (avatarImageView != null) {
            avatarImageView.setForUserOrChat(chat, avatarDrawable)
            avatarImageView.setRoundRadius(
                if (chat != null && chat.forum) AndroidUtilities.dp(30f) else AndroidUtilities.dp(
                    30f
                )
            )
        }
    }


    private fun setupUsersInfo() {

        try {

            if (teleUser != null) {

                if (teleUser?.type == "inputPeerUser") {

                    if (teleUser?.id != null) {

                        firstAvatarImageView = BackupImageView(this)

                        firstAvatarDrawable = AvatarDrawable()

                        binding.firstUserFrame.addView(firstAvatarImageView)

                        firstAvatarImageView!!.contentDescription = "Avatar"

                        firstAvatarImageView!!.setRoundRadius(AndroidUtilities.dp(30f))

                        val user = getTeleUserInId(teleUser?.id!!)

                        setTeleUserAvatar(
                            firstAvatarImageView,
                            firstAvatarDrawable,
                            user as TLRPC.User,
                            false
                        )

                    }


                } else if (teleUser?.type == "inputPeerChat") {

                    firstAvatarImageView = BackupImageView(this)

                    firstAvatarDrawable = AvatarDrawable()

                    binding.firstUserFrame.addView(firstAvatarImageView)

                    firstAvatarImageView!!.contentDescription = "Avatar"

                    firstAvatarImageView!!.setRoundRadius(AndroidUtilities.dp(30f))


                    binding.usersNames.text = teleUser?.username ?: EMPTY

                    if (teleUser?.id != null) {

                        val chat = getChatInId(teleUser?.id!!)

                        setTeleChatAvatar(
                            chat,
                            firstAvatarImageView,
                            firstAvatarDrawable
                        )

                    }

                } else if (teleUser?.type == "inputPeerChannel") {

                    firstAvatarImageView = BackupImageView(this)

                    firstAvatarDrawable = AvatarDrawable()

                    binding.firstUserFrame.addView(firstAvatarImageView)

                    firstAvatarImageView!!.contentDescription = "Avatar"

                    firstAvatarImageView!!.setRoundRadius(AndroidUtilities.dp(30f))

                    binding.pay.show(isAccept)

                    binding.usersNames.text = teleUser?.username ?: EMPTY

                    if (teleUser?.id != null) {

                        val chat = getChatInId(teleUser?.id!!)



                        setTeleChatAvatar(

                            chat,

                            firstAvatarImageView,

                            firstAvatarDrawable
                        )

                    }

                }

            }
        } catch (e: Exception) {
        }
    }

    fun contactMapper(u: TLRPC.User): Users {

        val imageDrawable = AvatarDrawable()

        val user = getTeleUserInId(u.id as Long)

        return Users(
            teleUser = user,
            imageView = null,
            avatarDrawable = imageDrawable,
            accessHash = if (u.access_hash != null) u.access_hash.toString() else EMPTY,
            isChecked = false,
            headerName = EMPTY,
            isSelectedChip = false,
            id = u.id ?: 0,
            status = EMPTY,
            firstName = u.first_name ?: EMPTY,
            lastName = u.last_name ?: EMPTY,
            username = u.username ?: EMPTY,
            avatar = null,
            avatarDir = null
        )
    }


    private fun setupAddUsersInfo() {

        centerAvatarImageView?.gone()

        lastAvatarImageView?.gone()


        try {

            if (addUsers.isNotEmpty()) {

                for (i in 0 until if (addUsers.size > 2) 2 else addUsers.size) {

                    if (i == 0) {

                        centerAvatarImageView = BackupImageView(this)

                        centerAvatarDrawable = AvatarDrawable()

                        binding.centerUserFrame.addView(centerAvatarImageView)

                        centerAvatarImageView!!.contentDescription = "Avatar"

                        centerAvatarImageView!!.setRoundRadius(AndroidUtilities.dp(30f))

                        val user = getTeleUserInId(addUsers[i].id)

                        setTeleUserAvatar(
                            centerAvatarImageView,
                            centerAvatarDrawable,
                            user as TLRPC.User,
                            false
                        )


                    }

                    if (i == 1) {

                        lastAvatarImageView = BackupImageView(this)

                        lastAvatarDrawable = AvatarDrawable()

                        binding.lastUserFrame.addView(lastAvatarImageView)

                        lastAvatarImageView!!.contentDescription = "Avatar"

                        lastAvatarImageView!!.setRoundRadius(AndroidUtilities.dp(30f))

                        val user = getTeleUserInId(addUsers[i].id)

                        setTeleUserAvatar(
                            lastAvatarImageView,
                            lastAvatarDrawable,
                            user as TLRPC.User,
                            false
                        )

                    }

                }

            }
        } catch (e: Exception) {

        }

    }


    private fun getTeleUserInId(id: Long): TLRPC.User {

        val currentAccount = UserConfig.selectedAccount

        val messagesController = MessagesController.getInstance(currentAccount)

        return messagesController.getUser(id)

    }

    private fun getChatInId(id: Long): TLRPC.Chat {

        val currentAccount = UserConfig.selectedAccount

        val messagesController = MessagesController.getInstance(currentAccount)

        return messagesController.getChat(id)
    }


    fun generatePeers(): ArrayList<Peers> {

        val peers: kotlin.collections.ArrayList<Peers> = ArrayList()

        var amount =
            (if (isOpenMinimumAmountDialog) selectedCoin.exchangeInfo?.toAssetMinAmount!!.toDouble() else

                binding.payOrRequestTextField.text.toString().formatStringToString().trim()
                    .formatStringToString().toDouble())



        if (teleUser?.type == "inputPeerChat") {

            teleUser?.let {


                peers.add(
                    Peers(


                        peerId = it.id,
                        extraPeerId = it.id,
                        accessHash = it.access_hash,
                        title = "sword",
                        type = it.type,
                        amount = amount


                    )
                )


            }

        } else if (teleUser?.type == "inputPeerUser") {

            teleUser?.let {


                peers.add(
                    Peers(


                        peerId = it.id,
                        extraPeerId = it.id,
                        accessHash = it.access_hash,
                        title = "sword",
                        type = it.type,
                        amount = amount


                    )
                )

            }

            addUsers.forEach {

                val messagesController = MessagesController.getInstance(UserConfig.selectedAccount)


                val inputPeer: TLRPC.InputPeer = messagesController.getInputPeer(it.id)

                peers.add(
                    Peers(


                        peerId = inputPeer?.id,
                        extraPeerId = inputPeer?.id,
                        accessHash = if (inputPeer?.access_hash != null) inputPeer?.access_hash.toString() else null,
                        title = "sword",
                        type = inputPeer?.type,
                        amount = amount


                    )
                )
            }

        } else if (teleUser?.type == "inputPeerChannel") {

            teleUser?.let {

                peers.add(
                    Peers(


                        peerId = it.id,
                        extraPeerId = it.id,
                        accessHash = toUnsignedBigInteger(it.access_hash!!.toLong()).toString(),
                        title = teleUser?.username ?: "",
                        type = it.type,
                        amount = amount


                    )
                )

            }


        }



        return peers

    }

    private fun toUnsignedBigInteger(i: Long): BigInteger? {
        return if (i >= 0L) {
            BigInteger.valueOf(i)
        } else {
            val upper = (i ushr 32).toInt()
            val lower = i.toInt()

            BigInteger.valueOf(Integer.toUnsignedLong(upper))
                .shiftLeft(32)
                .add(BigInteger.valueOf(Integer.toUnsignedLong(lower)))
        }
    }

    private fun showUsersInfo() {

        var names = "$currentUserName,"

        addUsers.forEachIndexed { index, users ->


            names += users.firstName.ifEmpty { users.username }

            if (users.firstName.isNotEmpty() || users.username.isNotEmpty() && index != addUsers.size - 1) {

                names += ","
            }

        }

        binding.usersNames.text = if (names.endsWith(",")) {

            names.substring(0, names.length - 1)

        } else {

            names
        }

        binding.centerUserFrame.show(addUsers.isNotEmpty())

        binding.lastUserFrame.show(addUsers.size >= 2)

        binding.moreUsersBtn.show(addUsers.size > 2)

        setupAddUsersInfo()

    }


    private fun acceptResponse() {

        chatViewModel.acceptResponse.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {

                        try {
                            if (teleUser?.accept?.peer?.title == "swordChannel") {

                                goToSavedMessage(true, ApplicationLoader.applicationContext)
                            }

                        } catch (e: Exception) {

                        }


                        if (!it.data?.data?.redirectUrl.isNullOrEmpty()) {

                            openCustomChrome(url = it.data?.data?.redirectUrl ?: EMPTY)

                        } else {

                            onBackPressed()

                        }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::accept
                    )

                    else -> {

                        showMessageDialog(title = getString(R.string.accept), it.message)
                    }
                }
            } catch (e: Exception) {

                showMessageDialog(title = getString(R.string.accept), getLocalizeMessage(""))

            } finally {
                binding.bottomSheetLoading.gone()
            }
        }
    }


    private fun accept() {

            binding.bottomSheetLoading.show()

            chatViewModel.acceptTele(

                  id = teleUser?.accept?.payId?: EMPTY,
                AcceptRequestModelTele(
                    transactionType = if (activeTab == MoneyTab.CASH) "fiat" else "crypto",
                    note = binding.addANote.text.toString(),
                    peer = teleUser?.accept?.peer!!,
                    messageId = teleUser?.accept?.messageId ?: 0,
                    currencyType = if (activeTab == MoneyTab.CASH) ACCOUNT_CURRENCY else selectedCoin.coin

                )
            )

    }


    private fun binanceStreamsListener() {

        binanceViewModel.socketResponse.observe(this) {

            try {

                with(it.toString()) {

                    when {

                        contains(BinanceStreams.PRICE_AND_PERCENT_EVENT) -> liveChangePriceAndPercent(
                            stream = Gson().fromJson(it, CryptoToCryptoEventModel::class.java)
                        )


                    }

                }

            } catch (e: Exception) {


            }

        }

    }

    private fun liveChangePriceAndPercent(stream: CryptoToCryptoEventModel) {


        if (stream.data?.symbol == ACCOUNT_CURRENCY + mainCoin.uppercase()) {

            mainCoinToEur = 1 / stream.data.lastPrice.toBigDecimal().toDouble()

        }

        if (stream.data?.symbol == selectedCoin.coin + mainCoin.uppercase()) {

            selectedCoinToMainCoin = stream.data.lastPrice.toDouble()

            if (!isAccept) {

                setupPayOrReqMinMax()
            }

        }

        if (stream.data?.symbol == teleUser?.accept?.currencyType + mainCoin.uppercase()) {

            requestCoinToMainCoin = stream.data.lastPrice.toDouble()

        }

        if (selectedCoin.coin.uppercase() == mainCoin) {

            selectedCoinToEur = mainCoinToEur

            if (teleUser?.accept?.currencyType?.uppercase() == mainCoin) {

                selectedCoinToMainCoin = 1.0

                requestCoinToMainCoin = 1.0

                if (requestCoinToMainCoin > 0) {

                    selectedCoinToRequestCoin = selectedCoinToMainCoin / requestCoinToMainCoin
                }


            } else {

                selectedCoinToMainCoin = 1.0

                if (requestCoinToMainCoin > 0) {

                    selectedCoinToRequestCoin = selectedCoinToMainCoin / requestCoinToMainCoin
                }

            }
        } else {

            if (requestCoinToMainCoin > 0) {

                selectedCoinToRequestCoin = selectedCoinToMainCoin / requestCoinToMainCoin
            }


            selectedCoinToEur = selectedCoinToMainCoin * mainCoinToEur
        }


        setupChangeAmount()

    }

    private fun connectToCoinChangAmount() {

        binanceViewModel.stopSocket()

        connectToBinance()

        oldSelectedCoin = selectedCoin


    }


    private fun connectToBinance() {

        if (isConnectedToInternet(this)) {

            binanceStreamsListener()

            binanceViewModel.connectSocket(binanceViewModel)

        }

    }


    @SuppressLint("SetTextI18n")
    override fun configUi() {

        binding.topBar.set(this, title = "Send Or Request")

        binding.pay.setCorrectTextSize(
            context = this,
            defaultSize = if (!isAccept) 16f else 19f
        )

        binding.request.setCorrectTextSize(context = this, defaultSize = 16f)



        binding.pay.isEnabled = false

        binding.request.isEnabled = false

        binding.payOrRequestTextField.enableVerticalScroll()

        binding.sendMoneySegmentTab.set(
            firstTabName = resources.getString(R.string.cash),
            lastTabName = resources.getString(R.string.crypto)
        )

        binding.addANote.enableVerticalScroll()

        if (isAccept) {

            binding.request.gone()

            binding.requestMessageInfo.show()



        } else {

            binding.request.show()

            binding.requestMessageInfo.gone()
        }

        currentUserName =
            if (teleUser?.type == "inputPeerUser") teleUser?.firstName ?: teleUser?.username
            ?: EMPTY else teleUser?.username ?: EMPTY

        binding.usersNames.text = currentUserName


    }


    private fun selectedType(isFiat: Boolean) {

        amountType = if (isFiat) "fiat" else "crypto"

        if (isFiat) {


            val text = binding.payOrRequestTextField.text.toString()
                .changePrecisionCount(count = PRECISION_2)

            binding.payOrRequestTextField.setText(text)

            binding.payOrRequestTextField.setSelection(text.length)


            binding.amountTypeIcon.setImageResource(R.drawable.bold_euro)

        } else {


            val text = binding.payOrRequestTextField.text.toString()
                .changePrecisionCount(count = PRECISION_8)

            binding.payOrRequestTextField.setText(text)

            binding.payOrRequestTextField.setSelection(text.length)

            binding.amountTypeIcon.setImageResource(R.drawable.bold_c)

        }

    }

    override fun clickListener(bind: FragmentPayOrRequestBinding) {



        bind.topBar.onBackBtn.setOnClickListener { onBackPressed() }


        bind.amountDropDownIconBtn.setOnClickListener {
            openBottomSheet(
                fragment = CashOrCrypto(
                    selectedType = ::selectedType
                )
            )
        }

        bind.sendMoneySegmentTab.firstSegmentTab.setOnClickListener {

            if (activeTab != MoneyTab.CASH) {

                val text = binding.payOrRequestTextField.text.toString()
                    .changePrecisionCount(count = PRECISION_2)

                binding.payOrRequestTextField.setText(text)

                binding.payOrRequestTextField.setSelection(text.length)

                setupSegmentTab(tab = MoneyTab.CASH)
            }
        }


        bind.sendMoneySegmentTab.lastSegmentTab.setOnClickListener {

            if (activeTab != MoneyTab.CRYPTO) {

                if (KYC_VERIFIED && !IS_USA_USER_LOCATION) {

                    setupSegmentTab(tab = MoneyTab.CRYPTO)

                } else {

                    if (!KYC_VERIFIED) {

                        showActionDialog(
                            title = "Incomplete Account",
                            subTitle = "You need to update your crypto account status",
                            negBtnName = "Cancel",
                            posBtnName = "Ok",
                            posBtnFunction = {

                                finish()

                                openActivity(activity = CryptoAccount())

                            }
                        )
                    } else {

                        showActionDialog(
                            isSingleButton = true,
                            title = "Apologies,\nbut the service is currently\nunavailable in your region.",
                            posBtnName = "Ok",
                        )

                    }

                }

            }


        }


        bind.cryptoInfo.setOnClickListener {

            if (!isAccept && allCrypto.isNotEmpty()) {

                openBottomSheet(
                    fragment = CryptoList(
                        cryptoList = allCrypto,
                        selectedCoin = ::getSelectedCoin
                    )
                )
            } else {

                openBottomSheet(
                    fragment = CryptoList(
                        cryptoList = filterExchangeInfoList,
                        selectedCoin = ::getSelectedCoin
                    )
                )

            }

        }

        bind.pay.setOnClickListener {

            payOrRequestType = "pay"

            payOrRequestClick()


        }

        bind.request.setOnClickListener {

            payOrRequestType = "request"

            clickPay = false

            payOrRequestClick()


        }


        bind.consent.refreshBtn.setOnClickListener {

            isGoToConsent = true


            openCustomChrome(url = accountsInfo.data.redirectUrl ?: EMPTY)
        }


    }


    private fun payOrRequestClick() {

        binding.payOrRequestTextField.text.also {


            if (!isAccept) {


                val amount = (it.toString().formatStringToString())
                try {

                    if (amount.isNotEmpty() || amount.toDouble() >0.0) {

                        payOrRequest()
                    }


                } catch (e: Exception) {

                    showValidationMessage(message = "Invalid amount")
                }
                setupChangeAmount()

            } else {

                accept()


            }


        }


    }

    override fun onResume() {

        super.onResume()

        try {
            if (teleUser?.type == "inputPeerUser") {

                showUsersInfo()
            }
        } catch (e: Exception) {

        }


    }


    private fun payOrRequestResponse() {

        chatViewModel.paymentsResponseTele.observe(this) {
            try {

                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {
                        if (it.data != null) {

                            if (!it.data.data?.redirectUrl.isNullOrEmpty()) {

                                it.data.data?.redirectUrl?.let { it1 -> openCustomChrome(url = it1) }


                            } else {
                                onBackPressed()
                                ContactsActivity.goToUserChat = true

                            }


                        }


                    }

                    Status.VOID_SUCCESS -> {

                        ContactsActivity.goToUserChat = true

                        onBackPressed()

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::payOrRequest
                    )

                    else -> {
                        showMessageDialog(
                            title = getString(R.string.pay_or_request),
                            it.message
                        )


                    }
                }
            } catch (e: Exception) {

                showMessageDialog(
                    title = getString(R.string.pay_or_request),
                    getLocalizeMessage("")
                )

            } finally {
                binding.bottomSheetLoading.gone()
            }
        }
    }


    private fun showUsers() {

        binding.usersLayout.show()
    }


    private fun priceWriteListener() {


        var beforeAmountText = EMPTY

        binding.payOrRequestTextField.doBeforeTextChanged { text, start, count, after ->


            binding.payOrRequestTextField.beforeValidText(
                maxIntegerPartCount = MAX_INTEGER_PART_COUNT,
                maxDecimalPartCount =

                if (activeTab == MoneyTab.CASH) {
                    PRECISION_2
                } else {
                    if (amountType == "fiat") PRECISION_2 else PRECISION_8
                },
            )?.let {

                    validText ->

                beforeAmountText = validText.formatStringToString()

            }

        }

        binding.payOrRequestTextField.doOnTextChanged { it, index, _, afterTextCount ->


            try {

                binding.payOrRequestTextField.startManipulationForPartCount(

                    num = it.toString(),

                    maxIntegerPartCount = MAX_INTEGER_PART_COUNT,

                    maxDecimalPartCount = if (activeTab == MoneyTab.CASH) {
                        PRECISION_2
                    } else {
                        if (amountType == "fiat") PRECISION_2 else PRECISION_8
                    },

                    beforeAmountText = beforeAmountText,

                    changIndex = index,

                    afterTextCount = afterTextCount

                )

                binding.payOrRequestTextField.autoTextSize(
                    initialTextSize = 48f,
                    startChangeSizeTextCount = 6
                )

                val amount = try {

                    if (binding.payOrRequestTextField.text.toString().isNotEmpty()) it.toString()
                        .formatStringToString().toDouble() else 0.0

                } catch (e: Exception) {

                    0.0
                }



                if (isAccept) {

                    binding.pay.isEnabled = true

                } else {

                    binding.pay.isEnabled = binding.payOrRequestTextField.text.toString().trim()
                        .isNotEmpty() && amount > 0
                }


                binding.request.isEnabled =
                    binding.payOrRequestTextField.text.toString().trim().isNotEmpty() && amount > 0


            } catch (e: Exception) {

                binding.pay.isEnabled = false

                binding.request.isEnabled = false

            }


        }


    }

    @SuppressLint("SetTextI18n")
    private fun setupChangeAmount() {

        try {
            if (isAccept) {

                when (activeTab) {

                    MoneyTab.CASH -> {

                        if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {

                            oldCryptoAmount = teleUser?.accept?.amount!!.toDouble()


                        } else {

                            oldCryptoAmount =
                                doubleBigDecimal((teleUser?.accept?.amount!! * requestCoinToMainCoin * mainCoinToEur).toString()).toDouble()


                        }


                    }

                    MoneyTab.CRYPTO -> {


                        if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {


                            oldCryptoAmount =
                                doubleBigDecimal((teleUser?.accept?.amount!!.toDouble() / selectedCoinToEur).toString()).toDouble()


                        } else {

                            oldCryptoAmount =
                                doubleBigDecimal((teleUser?.accept?.amount!! * (requestCoinToMainCoin / selectedCoinToMainCoin)).toString()).toDouble()

                        }


                    }

                }

                if (doubleBigDecimal(oldCryptoAmount.toString()) != doubleBigDecimal(
                        binding.payOrRequestTextField.text.toString().trim().formatStringToString()
                    )
                ) {

                    binding.payOrRequestTextField.setText(
                        doubleBigDecimal(
                            oldCryptoAmount.toString()
                        )
                    )

                }


            }


        } catch (e: Exception) {

            showMessageDialog(title = "setupChangeAmount", message = e.message.toString())

        }

    }

    private fun payOrRequest() {


        binding.bottomSheetLoading.show()

        try {
            chatViewModel.payOrRequestTele(
                requestData = PayOrRequestRequestModel(
                    transactionType = if (activeTab == MoneyTab.CRYPTO) "crypto" else "fiat",
                    note = binding.addANote.text.toString(),
                    type = payOrRequestType,
                    currencyType = currencyType,
                    amountType = if (activeTab == MoneyTab.CRYPTO) amountType else null,
                    peers = generatePeers()

                ),


            )
        } catch (e: Exception) {

            binding.bottomSheetLoading.gone()
            showMessageDialog(title = "Pay Or Request", message = getLocalizeMessage(""))

        }

    }


    @SuppressLint("SetTextI18n")
    private fun getSelectedCoin(coin: CryptoModel) {

        selectedCoin = coin

        currencyType = selectedCoin.coin

        changeCoin(coin = selectedCoin)

        fetchCryptoToMainCoin()

    }


    @SuppressLint("SetTextI18n")
    private fun changeCoin(coin: CryptoModel) {

        binding.coinIcon.loadCryptoImage(coin.coin)

        binding.coinEuro.text = "${coin.coin}/EUR"

        binding.coinEuroAmount.text =
            "1${coin.coin} = ${
                numberFormatString(
                    doubleBigDecimal(if (coin.coin == mainCoin) mainCoinToEur.toString() else this.selectedCoinToEur.toString()),
                    PRECISION_8
                )
            }EUR"


        setupChangeAmount()

    }

    private fun setupSegmentTab(tab: MoneyTab) {

        activeTab = tab

        binding.cryptoInfo.show(activeTab == MoneyTab.CRYPTO)

        binding.amountDropDownIconBtn.isEnabled =
            activeTab == MoneyTab.CRYPTO && !isAccept && tab == MoneyTab.CRYPTO

        binding.amountDropDownIcon.show(activeTab == MoneyTab.CRYPTO && !isAccept)

        when (tab) {

            MoneyTab.CASH -> {


                binding.amountTypeIcon.setImageResource(R.drawable.bold_euro)

                currencyType = ACCOUNT_CURRENCY

                binding.cryptoInfo.gone()

                binding.amountDropDownIcon.gone()

                binding.sendMoneySegmentTab.activateFirstTab(context = this)

                if (isAccept) {

                    if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {


                        binding.payOrRequestTextField.setText(
                            teleUser?.accept?.amount.toString()
                        )

                        binding.payOrRequestTextField.isFocusable = false

                    } else {
                        if (teleUser?.accept?.currencyType?.uppercase() == mainCoin.uppercase()) {
                            binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! * mainCoinToEur).toCorrectString()))

                        } else {
                            binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! * requestCoinToMainCoin * mainCoinToEur).toCorrectString()))

                        }


                        binding.payOrRequestTextField.isFocusable = false

                    }

                }
            }

            MoneyTab.CRYPTO -> {

                if (isAccept) {

                    amountType = "crypto"
                }

                if (amountType == "fiat") {

                    binding.amountTypeIcon.setImageResource(R.drawable.bold_euro)

                } else {

                    binding.amountTypeIcon.setImageResource(R.drawable.bold_c)

                }


                binding.cryptoInfo.isEnabled =
                    myCryptoList.size > 1 || allCrypto.size > 1

                binding.sendMoneySegmentTab.activateLastTab(context = this)

                changeCoin(coin = selectedCoin)

                currencyType = selectedCoin.coin

                if (isAccept) {


                    if (teleUser?.accept?.currencyType == ACCOUNT_CURRENCY) {

                        binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! / selectedCoinToMainCoin * mainCoinToEur).toString()))

                        binding.payOrRequestTextField.isFocusable = false

                    } else {

                        if (teleUser?.accept?.currencyType?.uppercase() == mainCoin.uppercase()) {

                            if (currencyType == mainCoin) {

                                binding.payOrRequestTextField.setText(teleUser?.accept?.amount.toString())

                            } else {
                                binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! / selectedCoinToMainCoin).toString()))

                            }


                        } else {

                            if (currencyType == mainCoin) {

                                binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! * (requestCoinToMainCoin)).toString()))

                            } else {

                                binding.payOrRequestTextField.setText(doubleBigDecimal((teleUser?.accept?.amount?.toDouble()!! * (requestCoinToMainCoin / selectedCoinToMainCoin)).toString()))

                            }

                        }


                        binding.payOrRequestTextField.isFocusable = false

                    }

                }

            }

        }

        if (!isAccept) {

            setupPayOrReqMinMax()

        } else {

            setupAcceptMinMax()
        }

        setupChangeAmount()

        if (IS_USA_USER_LOCATION && isAccept) {

            if (teleUser?.accept?.currencyType?.uppercase() != ACCOUNT_CURRENCY) {

                binding.amountUnavailableMessage.show()

                binding.pay.isEnabled = true
            }

        }

    }


    private fun balanceResponse() {

        viewModel.allBalanceResponse.observe(this) {
            try {

                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {

                        it.data?.let { accountsBalance -> accountsInfo = accountsBalance }

                        if (!it.data?.data?.redirectUrl.isNullOrEmpty()) {

                            binding.bottomSheetLoading.gone()

                            binding.consent.root.show()

                        } else {

                            userBalanceType = it.data?.data?.balanceType ?: ALL

                            mainCoin = it.data?.data?.mainCoin ?: "USDT"

                            if (!isAccept) {

                                fetchCryptoAssets()

                            } else {

                                if (it.data?.data?.crypto != null) {

                                    it.data.data.crypto.forEach { crypto ->

                                        myCryptoList.add(
                                            CryptoModel(
                                                coin = crypto.coin,
                                                name = EMPTY,
                                                balance = crypto.balance.toString()
                                            )
                                        )

                                    }

                                    if (myCryptoList.isNotEmpty()) {

                                        selectedCoin =
                                            if (myCryptoList.first().coin.isNotEmpty()) myCryptoList.first() else myCryptoList.seconds()

                                        oldSelectedCoin = selectedCoin
                                    }

                                }


                                showUsers()

                                fetchCryptoToMainCoin()

                            }

                        }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::fetchBalance
                    )

                    else -> {
                        showMessageDialog(
                            title = getString(R.string.pay_or_request),
                            message = it.message,
                            function = ::onBackPressed
                        )
                    }

                }
            } catch (e: Exception) {

                binding.bottomSheetLoading.gone()


            }
        }
    }


    private fun cryptoAssetsResponse() {

        binanceViewModel.cryptoAssets.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {
                        if (it.data == null) {

                            binding.bottomSheetLoading.gone()

                        } else {
                            it.data.let { resp ->

                                cryptoList = binanceViewModel.comBainCryptoList(responseData = resp)

                                allCrypto = cryptoList

                                it.data.data.mainCoin.let { baseCoin -> mainCoin = baseCoin }

                                if (allCrypto.isNotEmpty()) {

                                    selectedCoin =
                                        if (allCrypto.first().coin.isNotEmpty()) allCrypto.first() else allCrypto.seconds()
                                }



                                fetchCryptoToMainCoin()

                            }
                        }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::fetchCryptoAssets
                    )

                    else -> {
                        binding.bottomSheetLoading.gone()
                        showMessageDialog(
                            title = getString(R.string.crypto),
                            message = it.message,
                            function = ::onBackPressed


                        )
                    }
                }
            } catch (e: Exception) {
                binding.bottomSheetLoading.gone()
                showMessageDialog(title = getString(R.string.crypto), getLocalizeMessage(""))

            }
        }
    }


    private fun initialCryptoToMainCoinResponse() {

        binanceViewModel.cryptoToCryptoResp.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {

                        it.data?.map { crypto ->

                            initialCryptoToMainCoinList[crypto.symbol] = crypto

                        }

                        mainCoinToEur =
                            1.0 / initialCryptoToMainCoinList[ACCOUNT_CURRENCY + mainCoin.uppercase()]?.correctPrice()!!

                        requestCoinToMainCoin =
                            if (teleUser?.accept?.currencyType?.uppercase() != ACCOUNT_CURRENCY) {

                                if (teleUser?.accept?.currencyType?.uppercase() == mainCoin.uppercase()) {
                                    1.0
                                } else {
                                    initialCryptoToMainCoinList[teleUser?.accept?.currencyType?.uppercase() + mainCoin.uppercase()]?.correctPrice()
                                        ?: 0.0
                                }

                            } else {
                                0.0
                            }

                        selectedCoinToMainCoin =
                            if (selectedCoin.coin.uppercase() == mainCoin.uppercase()) {
                                1.0
                            } else {
                                initialCryptoToMainCoinList[selectedCoin.coin.uppercase() + mainCoin.uppercase()]?.correctPrice()
                                    ?: 0.0
                            }

                        selectedCoinToRequestCoin =
                            if (teleUser?.accept?.currencyType?.uppercase() != ACCOUNT_CURRENCY) {

                                if (teleUser?.accept?.currencyType?.uppercase() == mainCoin.uppercase()) {
                                    1.0
                                } else {
                                    selectedCoinToMainCoin / requestCoinToMainCoin
                                }
                            } else {
                                0.0
                            }

                        selectedCoinToEur =
                            if (selectedCoin.coin.uppercase() == mainCoin.uppercase()) {

                                mainCoinToEur

                            } else {
                                doubleBigDecimal((selectedCoinToMainCoin * mainCoinToEur).toString()).toDouble()
                            }

                        if (!isAccept) {


                            setupPayOrReqConfig()

                            connectToBinance()

                            connectToCoinChangAmount()

                            isFirstRequest = false

                        } else {

                            if (isFirstRequest) {

                                fetchExchangeInfo()

                            } else {

                                connectToBinance()

                                connectToCoinChangAmount()

                                setupChangeAmount()

                                setupAcceptMinMax()

                                binding.bottomSheetLoading.gone()

                            }


                        }

                    }

                    Status.NO_INTERNET_CONNECTION -> showNoInetDialog(
                        this,
                        function = ::fetchCryptoToMainCoin
                    )

                    else -> {


                        IS_USA_USER_LOCATION = true


                    }
                }
            } catch (e: Exception) {

                IS_USA_USER_LOCATION = true


            } finally {

                setupSegmentTab(activeTab)

                if (!isAccept || IS_USA_USER_LOCATION) {

                    binding.bottomSheetLoading.gone()
                }

            }
        }
    }


    private fun exchangeInfoResponse() {

        binanceViewModel.exchangeInfoResp.observe(this) {
            try {
                when (it.status) {


                    Status.FORCE_UPDATE -> {}

                    Status.USER_BLOCKED -> {}

                    Status.SUCCESS -> {

                        if (it.data != null) {

                            exchangeInfoList = it.data

                            filterConvertedCoins()

                            setupAcceptConfig()

                        } else {

                            filterExchangeInfoList = myCryptoList

                        }

                        connectToBinance()

                        connectToCoinChangAmount()

                        setupChangeAmount()

                        setupAcceptMinMax()

                        isFirstRequest = false


                    }

                    else -> {


                        IS_USA_USER_LOCATION = true


                    }

                }
            } catch (e: Exception) {

                IS_USA_USER_LOCATION = true


            } finally {

                setupSegmentTab(activeTab)

                binding.bottomSheetLoading.gone()
            }
        }
    }

    private fun filterConvertedCoins() {

        var mainCoinExchange:ExchangeInfoModel? = null

        run blocking@{
            exchangeInfoList.forEach {



                    if (it.toAsset == ACCOUNT_CURRENCY) {

                        mainCoinExchange = it

                        return@blocking
                    }


            }
        }

        run blocking@{
            exchangeInfoList.forEach {


                if (teleUser?.accept?.currencyType == mainCoin){

                    requestCoinExchangeInfo = mainCoinExchange

                    return@blocking

                }else{

                    if (teleUser?.accept?.currencyType == it.toAsset) {

                        requestCoinExchangeInfo = it

                        return@blocking
                    }
                }


            }
        }


        if (myCryptoList.isNotEmpty()) {

            myCryptoList.forEach { crypto ->

                if (crypto.coin == mainCoin) {

                    crypto.exchangeInfo = mainCoinExchange

                    filterExchangeInfoList.add(crypto)

                }

                exchangeInfoList.forEach {

                    if (it.toAsset.uppercase() == crypto.coin.uppercase()) {

                        crypto.exchangeInfo = it

                        filterExchangeInfoList.add(crypto)

                    }


                }


            }

        }


        if (filterExchangeInfoList.isEmpty() && KYC_VERIFIED) {

            if (accountsInfo.data.fiat != null) {

                binding.sendMoneySegmentTab.root.gone()

                activeTab = MoneyTab.CASH


            } else {

                onBackPressed()
            }

        }

    }


    private fun setupPayOrReqConfig() {

        if (isFirstRequest) {

            activeTab = if (accountsInfo.data.fiat == null) {

                binding.sendMoneySegmentTab.root.gone()

                MoneyTab.CRYPTO


            } else {

                binding.sendMoneySegmentTab.root.show()

                MoneyTab.CASH

            }


        }

        setupSegmentTab(activeTab)

        setupPayOrReqMinMax()


    }

    private fun setupAcceptConfig() {

        if (isFirstRequest) {

            activeTab = if (accountsInfo.data.fiat == null) {

                if (myCryptoList.isEmpty() && KYC_VERIFIED) {

                    binding.sendMoneySegmentTab.root.gone()

                    showMessageDialog(
                        title = getString(R.string.pay_or_request),
                        "Insufficient amount",
                        function = { onBackPressed() })

                    MoneyTab.CRYPTO


                } else {

                    binding.sendMoneySegmentTab.root.gone()

                    MoneyTab.CRYPTO
                }


            } else {

                if (myCryptoList.isEmpty() && KYC_VERIFIED) {

                    binding.sendMoneySegmentTab.root.gone()

                    MoneyTab.CASH


                } else {

                    binding.sendMoneySegmentTab.root.show()

                    MoneyTab.CASH
                }


            }


        }

        setupSegmentTab(activeTab)

        setupAcceptMinMax()


    }


    @SuppressLint("SetTextI18n")
    private fun setupPayOrReqMinMax() {


    }

    @SuppressLint("SetTextI18n")
    private fun setupAcceptMinMax() {

    }


    private fun fetchBalance() {

        binding.bottomSheetLoading.show()

        viewModel.fetchMyFiatAndCryptoBalance()
    }


    private fun fetchCryptoAssets() {

        binanceViewModel.fetchCryptoAssets()

    }

    private fun fetchExchangeInfo() {


        binanceViewModel.fetchExchangeInfo(fromAsset = mainCoin)

    }


    private fun fetchCryptoToMainCoin() {

        binding.bottomSheetLoading.show()

        val eurToMainCoin = "\"$ACCOUNT_CURRENCY${mainCoin.uppercase()}\","

        val symbol = if (isAccept) {

            if (teleUser?.accept?.currencyType?.uppercase() == ACCOUNT_CURRENCY) {

                if (myCryptoList.isEmpty()) {

                    "[\"${teleUser?.accept?.currencyType!!.uppercase()}${mainCoin.uppercase()}\"]"

                } else {

                    if (selectedCoin.coin.uppercase() != mainCoin.uppercase()) {

                        "[\"${teleUser?.accept?.currencyType!!.uppercase()}${mainCoin.uppercase()}\",\"${selectedCoin.coin.uppercase()}${mainCoin.uppercase()}\"]"

                    } else {

                        "[\"${teleUser?.accept?.currencyType!!.uppercase()}${mainCoin.uppercase()}\"]"

                    }


                }

            } else {

                if (teleUser?.accept?.currencyType?.uppercase() == selectedCoin.coin.uppercase() || myCryptoList.isEmpty()) {

                    if (teleUser?.accept?.currencyType?.uppercase() != mainCoin.uppercase()) {

                        "[$eurToMainCoin\"${teleUser?.accept?.currencyType?.uppercase()}${mainCoin.uppercase()}\"]"

                    } else {

                        "[\"$ACCOUNT_CURRENCY${mainCoin.uppercase()}\"]"
                    }

                } else {

                    if (teleUser?.accept?.currencyType?.uppercase() != mainCoin.uppercase() && selectedCoin.coin.uppercase() != mainCoin.uppercase()) {

                        "[$eurToMainCoin\"${teleUser?.accept?.currencyType?.uppercase()}${mainCoin.uppercase()}\",\"${selectedCoin.coin.uppercase()}${mainCoin.uppercase()}\"]"

                    } else if (teleUser?.accept?.currencyType?.uppercase() == mainCoin.uppercase() && selectedCoin.coin.uppercase() != mainCoin.uppercase()) {

                        "[$eurToMainCoin\"${selectedCoin.coin.uppercase()}${mainCoin.uppercase()}\"]"

                    } else {
                        "[$eurToMainCoin\"${teleUser?.accept?.currencyType?.uppercase()}${mainCoin.uppercase()}\"]"
                    }


                }


            }
        } else {

            if (selectedCoin.coin.uppercase() != mainCoin.uppercase() && selectedCoin.coin.isNotEmpty()) {

                "[$eurToMainCoin\"${selectedCoin.coin.uppercase()}${mainCoin.uppercase()}\"]"

            } else {

                "[\"$ACCOUNT_CURRENCY${mainCoin.uppercase()}\"]"
            }


        }



        binanceViewModel.fetchCryptoToCryptoInfo(symbols = symbol.toString())
    }


    private fun showValidationMessage(message: String) {

        showMessageDialog(
            status = DialogStatus.INFO,
            message = message,
            title = "Validation Failed"
        )

    }


}