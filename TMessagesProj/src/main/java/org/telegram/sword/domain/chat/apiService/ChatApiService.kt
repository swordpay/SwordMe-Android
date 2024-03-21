package org.telegram.sword.domain.chat.apiService

import org.telegram.sword.domain.chat.model.*
import org.telegram.sword.domain.common.Version
import retrofit2.Response
import retrofit2.http.*

interface ChatApiService {


    @POST("${Version.V_1}/payments")
    suspend fun payOrRequest(@Body requestData: PayOrRequestRequestModel): Response<PayOrRequestResponseModelTele>

    @POST("${Version.V_1}/payments/{id}/accept")
    suspend fun acceptTele(@Path("id") id: String,@Body requestData: AcceptRequestModelTele): Response<PayOrRequestResponseModel>

}