package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val show = SingleLiveEvent<Boolean>()

val isShowCancelBtn : LiveData<Boolean> get() =  show


fun addCardCancelButtonEvent(showBtn:Boolean) {
    show.value = showBtn
}