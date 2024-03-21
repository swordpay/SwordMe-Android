package org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData

import androidx.lifecycle.MutableLiveData
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.domain.binance.model.CryptoModel


var cryptoList: ArrayList<CryptoModel> = ArrayList()

    var mainCoin = EMPTY

    var mainCoinToEur = 0.0

    var mainCoinToEurPercent = 0.0

    var myBalanceInMainCoin = 0.0

    var cryptoModel: MutableLiveData<CryptoModel> = MutableLiveData()




