package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent


private val sharedData = SingleLiveEvent<Any?>()

  val getSharedData: LiveData<Any?> get() = sharedData

  fun<T:Any?> sharedMyData(data: T?){

      sharedData.value = data as T
  }

