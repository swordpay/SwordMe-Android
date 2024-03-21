package org.telegram.sword.domain.account.useCase

import android.content.Context
import org.telegram.sword.domain.account.apiHelper.AccountApiHelper
import org.telegram.sword.domain.common.BaseDataSource

class AccountUseCase(private val apiHelper: AccountApiHelper, context: Context): BaseDataSource(context = context) {

    suspend fun getMyFiatAndCryptoBalance() =  safeApiCall { apiHelper.getMyFiatAndCryptoBalance() }


}