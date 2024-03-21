package org.telegram.sword.app.common.extansion.ui

import android.content.Context
import android.content.res.ColorStateList
import androidx.core.content.ContextCompat
import androidx.core.view.isVisible
import org.telegram.messenger.R
import org.telegram.messenger.databinding.*
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.colors.AppColors.APP_BLUE
import org.telegram.sword.app.common.colors.AppColors.LIGHT_BLUE
import org.telegram.sword.app.common.extansion.hide
import org.telegram.sword.app.common.extansion.show

fun BlueButtonBinding.set(icon:Int? = null, name:String = EMPTY, isEnabled:Boolean = false){

    icon?.let {

        this.buttonIcon.show()
        this.buttonIcon.setImageResource(it)
    }
    this.buttonName.text = name

    this.isEnabled(isEnable = isEnabled)

}

fun BlueBorderButtonBinding.set(icon:Int? = null, name:String = EMPTY, isEnabled:Boolean = false){

    icon?.let {

        this.buttonIcon.show()
        this.buttonIcon.setImageResource(it)
    }

    this.buttonName.text = name

    this.isEnabled(isEnable = isEnabled)

}

fun BlueButtonBinding.isEnabled(isEnable: Boolean, isChangeFon:Boolean=true) {

    this.button.also {
        it.isEnabled = if (isEnable) {

            if (isChangeFon){

                it.setBackgroundResource(R.drawable.active_btn)
                this.buttonName.setTextColor(AppColors.WHITE)
            }

            true
        } else {

            if (isChangeFon){
                it.setBackgroundResource(R.drawable.inactive_btn)
                this.buttonName.setTextColor(AppColors.HINT)
            }
            false
        }
    }
}

fun GrayButtonBinding.set(title:String = EMPTY, icon:Int?=null){

    icon?.let { this.button.setCompoundDrawablesWithIntrinsicBounds(it, 0, 0, 0) }

    this.button.text = title
}

fun CircleButtonBinding.set(context: Context, title:String = EMPTY, icon:Int, isClicked:Boolean = false){

    if (isClicked){

        this.buttonFon.backgroundTintList = ColorStateList.valueOf(APP_BLUE)

        this.icon.setColorFilter(ContextCompat.getColor(context, R.color.white), android.graphics.PorterDuff.Mode.SRC_IN)


    }else{

        this.buttonFon.backgroundTintList = ColorStateList.valueOf(LIGHT_BLUE)

        this.icon.setColorFilter(ContextCompat.getColor(context, R.color.appBlue), android.graphics.PorterDuff.Mode.SRC_IN)
    }

    this.icon.setImageResource(icon)

    this.buttonName.text = title
}



fun BlueBorderButtonBinding.isEnabled(isEnable: Boolean, isChangeFon:Boolean=true) {

    this.button.also {
        it.isEnabled = if (isEnable) {

            if (isChangeFon){

                it.setBackgroundResource(R.drawable.blue_border_btn)
                this.buttonName.setTextColor(APP_BLUE)
            }
            true
        } else {

            if (isChangeFon){
                it.setBackgroundResource(R.drawable.inactive_border_btn)
                this.buttonName.setTextColor(AppColors.HINT)
            }
            false
        }
    }
}

fun FiatAccountButtonsItemBinding.set(icon:Int,title:String = EMPTY,
                                      subTitle:String = EMPTY,
                                      hasDefault:Boolean = false,
                                      color:Int

){

    this.icon.setImageResource(icon)

    this.iconFon.setImageResource(color)

    this.title.text = title

    this.subTitle.text = subTitle

    this.checkIcon.show(hasDefault)

}





fun FiatAccountButtonsItemBinding.select(){

    if (!this.checkIcon.isVisible){

        this.checkIcon.show()

    }else{

        this.checkIcon.hide()
    }



}

fun FiatAccountButtonsItemBinding.didSelect(){

    this.checkIcon.hide()

}