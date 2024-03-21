package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val isValid = SingleLiveEvent<Boolean>()

val enableButtonEvent : LiveData<Boolean> get() =  isValid

fun changeButtonStatus(valid: Boolean){

    isValid.value = valid
}