package org.telegram.sword.app.common.extansion.pixelRatio

import android.content.Context
import android.util.TypedValue
import android.view.View
import androidx.appcompat.widget.AppCompatTextView
import org.telegram.sword.app.common.AppConst.PixelRatio.BASE_XDPI

fun Context.correctSize(defaultSize:Float) =

    if (this.resources.displayMetrics.xdpi > 300){

        (BASE_XDPI / this.resources.displayMetrics.xdpi * defaultSize).toInt()

    }else{

        (1.2  * defaultSize).toInt()

    }

fun View.correctPadding(context:Context,defaultStartPadding:Int,defaultTopPadding:Int){



    val start = (context.correctSize(defaultStartPadding.toFloat()) * 2.0 ).toInt()

    val top = (context.correctSize(defaultTopPadding.toFloat()) * 2.3 ).toInt()

    this.setPadding(start,top,start,top)

}

fun AppCompatTextView.setCorrectTextSize(context: Context,defaultSize:Float){


    setTextSize(TypedValue.COMPLEX_UNIT_DIP, defaultSize)


}

fun setCorrectTextSizeForJava(textView:AppCompatTextView,context: Context,defaultSize:Float){


    textView.setTextSize(TypedValue.COMPLEX_UNIT_DIP, defaultSize)

}

fun Context.dpAsPixel(size:Int) = (size * resources.displayMetrics.density).toInt()


fun getDisplayAbsoluteValue(displayW:Int):Float = displayW.toFloat()/1080