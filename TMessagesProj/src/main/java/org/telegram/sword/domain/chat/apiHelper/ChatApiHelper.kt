package org.telegram.sword.domain.chat.apiHelper

import org.telegram.sword.domain.chat.apiService.ChatApiService
import org.telegram.sword.domain.chat.model.AcceptRequestModelTele
import org.telegram.sword.domain.chat.model.PayOrRequestRequestModel
import retrofit2.Retrofit

class ChatApiHelper (service : Retrofit) {

    private val service = service.create(ChatApiService::class.java)

    suspend fun payOrRequest(requestData: PayOrRequestRequestModel) = service.payOrRequest(requestData = requestData)

    suspend fun acceptTele(id: String,requestData: AcceptRequestModelTele) = service.acceptTele(id = id,requestData = requestData)



}