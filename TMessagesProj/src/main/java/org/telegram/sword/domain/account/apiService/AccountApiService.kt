package org.telegram.sword.domain.account.apiService

import org.telegram.sword.domain.account.model.*
import org.telegram.sword.domain.common.Version
import retrofit2.Response
import retrofit2.http.*

interface AccountApiService {


    @GET("${Version.V_1}/accounts/balance")
    suspend fun getMyFiatAndCryptoBalance(): Response<FiatAndCryptoBalanceResponseModel>

}