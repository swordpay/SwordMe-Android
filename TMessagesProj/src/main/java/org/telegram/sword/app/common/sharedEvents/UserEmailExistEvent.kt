package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val userEmail = SingleLiveEvent<Boolean>()

val emailExists : LiveData<Boolean> get() =  userEmail

fun userEmailExists(isExist:Boolean){

    userEmail.value = isExist
}
private val check = SingleLiveEvent<Boolean>()

val checkEmail : LiveData<Boolean> get() =  check


fun checkUserEmail(){

    check.value = true
}
