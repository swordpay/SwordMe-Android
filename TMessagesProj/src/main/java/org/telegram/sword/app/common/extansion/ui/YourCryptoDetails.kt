package org.telegram.sword.app.common.extansion.ui

import android.annotation.SuppressLint
import org.telegram.messenger.databinding.YourCryptoDetailsBinding
import org.telegram.sword.app.common.AppConst.PRECISION_2
import org.telegram.sword.app.common.AppConst.PRECISION_8
import org.telegram.sword.app.common.extansion.numberFormatString
import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoin
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEur
import org.telegram.sword.app.home.tabs.cryptoAccount.helper.correctBalanceInEuro
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol.CURRENCY_SYMBOL

@SuppressLint("SetTextI18n")
fun YourCryptoDetailsBinding.set(cryptoData: CryptoModel){

    val isMaiCoinSelected = cryptoData.coin.uppercase() == mainCoin.uppercase()

    this.yourCoinNameTitle.text = "Your ${cryptoData.name}"

    this.coinCountTitle.text = "Your ${cryptoData.name}"

    this.coinCount.text = "${numberFormatString(doubleBigDecimal(cryptoData.balance.toString()),PRECISION_8) } ${cryptoData.coin}"

    val euroValue = numberFormatString(correctBalanceInEuro(balanceInFiat = if (cryptoData.coin.uppercase() == mainCoin.uppercase()) mainCoinToEur.toString() else
        cryptoData.balanceInFiat, coinCount = cryptoData.balance, isMaiCoinSelected = isMaiCoinSelected),PRECISION_2)

    this.euroValue.text = "$CURRENCY_SYMBOL $euroValue"


}