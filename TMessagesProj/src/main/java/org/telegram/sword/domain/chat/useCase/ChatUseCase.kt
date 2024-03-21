package org.telegram.sword.domain.chat.useCase

import android.content.Context
import org.telegram.sword.domain.chat.apiHelper.ChatApiHelper
import org.telegram.sword.domain.chat.model.AcceptRequestModelTele
import org.telegram.sword.domain.chat.model.PayOrRequestRequestModel
import org.telegram.sword.domain.common.BaseDataSource

class ChatUseCase(private val apiHelper: ChatApiHelper, context: Context): BaseDataSource(context = context){

    suspend fun acceptTele(id: String,requestData: AcceptRequestModelTele) =  safeApiCall { apiHelper.acceptTele(id = id,requestData = requestData) }
    suspend fun payOrRequest(requestData: PayOrRequestRequestModel) =  safeApiCall { apiHelper.payOrRequest(requestData = requestData) }



}