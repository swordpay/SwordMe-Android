package org.telegram.sword.app.common.sharedEvents

import androidx.lifecycle.LiveData
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

private val goToEmailUndPass = SingleLiveEvent<Int>()

val goToEmailUndPassPage : LiveData<Int> get() =  goToEmailUndPass


fun changePageEvent(page:Int){

    when(page){

        1 ->  goToEmailUndPass.value = page

    }

}