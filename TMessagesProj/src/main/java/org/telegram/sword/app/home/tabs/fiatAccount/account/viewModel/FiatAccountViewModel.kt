package org.telegram.sword.app.home.tabs.fiatAccount.account.viewModel


import androidx.lifecycle.LiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent
import org.telegram.sword.domain.account.model.FiatAndCryptoBalanceResponseModel
import org.telegram.sword.domain.account.useCase.AccountUseCase
import org.telegram.sword.domain.common.BaseResource

class FiatAccountViewModel(private val useCase: AccountUseCase):ViewModel() {

    private var allBalance = SingleLiveEvent<BaseResource<FiatAndCryptoBalanceResponseModel>>()
    val allBalanceResponse: LiveData<BaseResource<FiatAndCryptoBalanceResponseModel>> get() = allBalance



    fun fetchMyFiatAndCryptoBalance() {

        viewModelScope.launch {

            try {
                allBalance.value =  useCase.getMyFiatAndCryptoBalance()

            }catch (e:Exception){ Unit }
        }
    }



}