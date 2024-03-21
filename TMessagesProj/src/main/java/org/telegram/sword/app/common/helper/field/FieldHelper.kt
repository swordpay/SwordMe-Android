package org.telegram.sword.app.common.helper.field

import org.telegram.sword.app.common.extansion.formatStringToString


fun doubleBigDecimal(num: String): String {

    try {

        val lastNum = num.toBigDecimal().toPlainString().formatStringToString()

        return  lastNum


    }catch (e:Exception){

        return num.formatStringToString()
    }

}


fun String.decimalString(decimalCount:Int = 0):String{

    var amount:String = this

    try {

        if (this.isNotEmpty() && decimalCount > 1){

            if (this.contains(".") ){

                val decIndex = this.indexOf(".")

                if (this.length-decIndex>decimalCount){

                    amount = this.substring(0,decIndex+decimalCount+1)
                }

            }
        }

    }catch (e:Exception){ Unit}

    return amount

}