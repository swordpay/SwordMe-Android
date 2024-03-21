package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val change = SingleLiveEvent<String>()

val changeButton : LiveData<String> get() =  change

fun changeButtonEvent(it:String){
    change.value = it
}