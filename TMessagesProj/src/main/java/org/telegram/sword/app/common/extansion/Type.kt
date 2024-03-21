package org.telegram.sword.app.common.extansion

import android.annotation.SuppressLint
import org.telegram.sword.app.common.AppConst.EMPTY
import java.math.RoundingMode
import java.util.*


fun String?.firstCharacter() = this?.substring(0,1)?.uppercase() ?: EMPTY

fun firstCharacterJvm(text:String?) = text?.substring(0,1)?.uppercase() ?: EMPTY


@SuppressLint("SuspiciousIndentation")
fun Double.rounded():Double{

 val thisLast = this.toString().substring(this.toString().lastIndexOf("." )+ 1)


  return  when(thisLast.length - thisLast.replace("0","").length){

        thisLast.length ->"%.${1}f".format(this).toDouble()

        0,1,2 ->  "%.${3}f".format(this).toDouble()

        else -> "%.${thisLast.length - thisLast.replace("0","").length}f".format(this).toDouble()

    }

}

fun Double.roundingUp(scale:Int)=  toBigDecimal().setScale(scale, RoundingMode.UP).toDouble()

fun Double.roundingDown(scale:Int)=  toBigDecimal().setScale(scale, RoundingMode.DOWN).toDouble()

fun Double.roundingTo2ScaleOrFull():Double {

   val roundP = ((this * 100).toLong()/100.0)

   return if (roundP > 0){

       roundP

   }else{

       this
   }

}


fun numberFormatString(num:String,precisionCount:Int? = null) = num

fun String.formatStringToDouble():Double{

    var num = this

    if (num.contains(",")) {


        num = num.replace(",", "")
    }
    if (num.contains("..")) {

        num = num.replace("..", ".")
    }


    return  (num.toDoubleOrNull() ?: 0.0).toCorrectString().toDouble()

}

fun String.formatStringToString():String{

    var num = this

    try {

        if (num.contains(",")) {

            num = num.replace(",", "")
        }


    }catch (_:Exception){ }

    return  num

}

fun formatStringToStringJava(amount:String):String{

    var num = amount

    try {

        if (num.contains(",")) {

            num = num.replace(",", "")
        }


    }catch (_:Exception){ }

    return  num

}
