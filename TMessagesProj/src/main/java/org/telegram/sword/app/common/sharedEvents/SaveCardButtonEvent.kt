package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val save = SingleLiveEvent<Boolean>()

val saveCard : LiveData<Boolean> get() =  save

fun saveCardEventEvent(){
    save.value = true
}