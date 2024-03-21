package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val click = SingleLiveEvent<Boolean>()

val nextClickEvent : LiveData<Boolean> get() =  click

fun nextPage(){

    click.value = true
}