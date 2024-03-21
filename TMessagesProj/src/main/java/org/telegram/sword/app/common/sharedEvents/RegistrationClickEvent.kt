package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val register = SingleLiveEvent<Any?>()

val registrationClickEvent: LiveData<Any?> get() = register

fun userRegister(){

    register.value = true
}