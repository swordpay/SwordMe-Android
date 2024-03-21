package org.telegram.sword.domain.account.apiHelper

import org.telegram.sword.domain.account.apiService.AccountApiService
import retrofit2.Retrofit

class AccountApiHelper (service: Retrofit) {

    private val service = service.create(AccountApiService::class.java)

    suspend fun getMyFiatAndCryptoBalance() = service.getMyFiatAndCryptoBalance()


}