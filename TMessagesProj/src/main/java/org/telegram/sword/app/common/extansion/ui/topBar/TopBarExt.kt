package org.telegram.sword.app.common.extansion.ui.topBar

import android.app.Activity
import android.content.res.ColorStateList
import org.telegram.messenger.databinding.ActivityTopBarBinding
import org.telegram.messenger.databinding.BottomSheetTopBarBinding
import org.telegram.messenger.databinding.LiveChatTopBarBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.extansion.firstCharacter
import org.telegram.sword.app.common.extansion.loadImage
import org.telegram.sword.app.common.extansion.show

fun BottomSheetTopBarBinding.set(title:String = EMPTY, rightButtonName:String = EMPTY, rightButtonIcon:Int? = null, rightBtnClick:(()->Unit)? = null){

    this.bottomSheetTitle.text = title

    this.bottomSheetRightBtn.text = rightButtonName

    rightButtonIcon?.let {   this.bottomSheetRightBtn.setCompoundDrawablesWithIntrinsicBounds(it, 0, 0, 0) }

    this.bottomSheetRightBtn.setOnClickListener{

        rightBtnClick?.invoke()

    }

}

fun ActivityTopBarBinding.set(context: Activity, title:String = EMPTY, rightButtonName:String = EMPTY, rightButtonIcon:Int? = null, rightBtnClick:(()->Unit)? = null){

    this.topBarTitle.text = title

    this.topBarRightBtn.text = rightButtonName

    rightButtonIcon?.let {   this.topBarRightBtn.setCompoundDrawablesWithIntrinsicBounds(it, 0, 0, 0) }

    this.topBarRightBtn.setOnClickListener{

        rightBtnClick?.invoke()

    }

    this.onBackBtn.setOnClickListener{

        context.onBackPressed()

    }

}

fun LiveChatTopBarBinding.set(context: Activity, name:String = EMPTY, userName:String = EMPTY, avatar:String, infoClick:(()->Unit)? = null){



    this.firsCharacter.show(avatar.isEmpty())

    this.yellowFon.show(avatar.isEmpty())

    if (avatar.isNotEmpty()) {

        this.avatar.loadImage(imageUrl = avatar)

        this.firsCharacter.text = EMPTY

    } else {

        this.avatar.setImageDrawable(null)

        this.avatar.backgroundTintList = ColorStateList.valueOf(AppColors.APP_BLUE)

        this.firsCharacter.text =name.firstCharacter()
    }


    this.userName.show(userName.isNotEmpty())

    this.name.text = name

    this.userName.text = userName


    this.onBackBtn.setOnClickListener{

        context.onBackPressed()

    }

}