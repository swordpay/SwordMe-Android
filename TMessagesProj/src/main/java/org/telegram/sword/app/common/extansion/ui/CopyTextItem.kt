package org.telegram.sword.app.common.extansion.ui

import android.app.Activity
import org.telegram.messenger.databinding.CopyTextItemBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.helper.function.copyText

fun CopyTextItemBinding.set(title:String = EMPTY, info :String = EMPTY){

    this.title.text = title

    this.infoText.text = info

}

fun CopyTextItemBinding.copy(context: Activity){

    copyText(copyText = this.infoText.text.toString(), title = this.title.text.toString(), context =context )

}

