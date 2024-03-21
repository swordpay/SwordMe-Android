package org.telegram.sword.app.common.base

import androidx.lifecycle.ViewModel

open class BaseViewModel:ViewModel() {

    fun<T:Any?> sharedData(data:T?){

    }
}