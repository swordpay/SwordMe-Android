package org.telegram.sword.app.home.tabs.chat.viewModel

import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent
import org.telegram.sword.domain.chat.model.AcceptRequestModelTele
import org.telegram.sword.domain.chat.model.PayOrRequestRequestModel
import org.telegram.sword.domain.chat.model.PayOrRequestResponseModel
import org.telegram.sword.domain.chat.model.PayOrRequestResponseModelTele
import org.telegram.sword.domain.chat.useCase.ChatUseCase
import org.telegram.sword.domain.common.BaseResource
import org.telegram.sword.domain.common.Status

class ChatViewModel(private val useCase: ChatUseCase):ViewModel() {

    private var paymentsTele = SingleLiveEvent<BaseResource<PayOrRequestResponseModelTele>>()
    val paymentsResponseTele: LiveData<BaseResource<PayOrRequestResponseModelTele>> get() = paymentsTele

    private var accept = SingleLiveEvent<BaseResource<PayOrRequestResponseModel>>()
    val acceptResponse: LiveData<BaseResource<PayOrRequestResponseModel>> get() = accept

    fun acceptTele(id:String,requestData: AcceptRequestModelTele) {

        viewModelScope.launch {

            try {
                accept.value  =  useCase.acceptTele( id = id ,requestData = requestData)

            }catch (e:Exception){ accept.value = BaseResource(status = Status.FAILURE) }
        }
    }



    fun payOrRequestTele(requestData: PayOrRequestRequestModel) {


        viewModelScope.launch {

            try {

                paymentsTele.value =  useCase.payOrRequest(requestData = requestData )

            }catch (e:Exception){

                paymentsTele.value = BaseResource(status = Status.FAILURE)

            }
        }
    }

}