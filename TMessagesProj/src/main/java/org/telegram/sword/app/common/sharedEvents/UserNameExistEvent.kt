package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val userName = SingleLiveEvent<Boolean>()

val isSuccessUserName : LiveData<Boolean> get() =  userName

fun usernameExists(isExist:Boolean){

    userName.value = isExist
}

private val check = SingleLiveEvent<Boolean>()

val checkName : LiveData<Boolean> get() =  check


fun checkUserName(){

    check.value = true
}