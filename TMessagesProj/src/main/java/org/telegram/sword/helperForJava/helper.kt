package org.telegram.sword.helperForJava

import android.annotation.SuppressLint
import androidx.core.graphics.drawable.toBitmap
import org.telegram.messenger.R
import org.telegram.sword.app.common.base.appResources


var instance:StaticStatesJava? = null

fun staticStatesJavaInstance():StaticStatesJava{

    if (instance == null){

       instance =  StaticStatesJava()
    }

   return instance!!
}

@SuppressLint("UseCompatLoadingForDrawables")
fun qrCenterBitmapIcon(width:Int, height:Int) = appResources?.getDrawable(R.drawable.qr_center_icon)!!.toBitmap(width, height)




