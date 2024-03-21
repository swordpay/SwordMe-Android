package org.telegram.sword.app.home.tabs.cryptoAccount.helper

import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEur

fun correctBalanceInEuro(balanceInFiat:String,coinCount:String?=null,isMaiCoinSelected:Boolean = false):String{

    return try {
        if (isMaiCoinSelected){

            if (!coinCount.isNullOrEmpty()){

                doubleBigDecimal((mainCoinToEur * coinCount.toDouble()).toString())

            }else{

                doubleBigDecimal((mainCoinToEur).toString())
            }

        }else{
            if (!coinCount.isNullOrEmpty()){

                doubleBigDecimal((balanceInFiat.toBigDecimal().toDouble()* mainCoinToEur * coinCount.toDouble()).toString())


            }else{
                doubleBigDecimal((balanceInFiat.toBigDecimal().toDouble()* mainCoinToEur).toString())
            }
        }


    }catch (e:Exception){
        "0.0"
    }

}