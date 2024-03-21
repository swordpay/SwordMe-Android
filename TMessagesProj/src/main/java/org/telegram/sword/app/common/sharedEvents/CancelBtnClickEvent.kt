package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val cancel = SingleLiveEvent<Boolean>()

val cancelClick : LiveData<Boolean> get() =  cancel

fun  cancelBtnClickEvent() {
    cancel.value = true
}