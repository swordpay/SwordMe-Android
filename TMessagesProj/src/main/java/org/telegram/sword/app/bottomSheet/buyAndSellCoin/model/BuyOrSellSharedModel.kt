package org.telegram.sword.app.bottomSheet.buyAndSellCoin.model

import org.telegram.sword.app.common.AppConst.BuyOrSellType.BUY
import org.telegram.sword.domain.binance.model.CryptoModel


data class BuyOrSellSharedModel(

    var crypto: CryptoModel,

    var type:String = BUY

)