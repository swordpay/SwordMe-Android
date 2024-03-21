package org.telegram.sword.domain.common


import android.content.Context
import androidx.lifecycle.LiveData
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import org.telegram.messenger.AndroidUtilities
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.DEFAULT_ERROR_MESSAGE_KEY
import org.telegram.sword.app.common.base.getLocalizeMessage
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent
import org.telegram.sword.app.common.helper.function.isConnectedToInternet
import org.telegram.sword.domain.common.UserBlocked.blockUser
import org.telegram.sword.domain.common.UserBlocked.forceUpdateApp
import org.telegram.sword.domain.common.UserBlocked.sessionExpired
import retrofit2.Response

abstract class BaseDataSource(private val context:Context) {

    suspend fun <T> safeApiCall(apiCall: suspend () -> Response<T>): BaseResource<T> {

        var errMessage= EMPTY

        try {
            val response = apiCall()
            return if (response.isSuccessful) {

                val body = response.body()
                if (body != null) {

                    if (!forceLogOutSession){

                        BaseResource.onSuccess(data = body, message = response.message())

                    }else{

                        BaseResource.forceUpdate()
                    }



                }else{

                    if (!forceLogOutSession){

                        when(response.code()){


                            204,201 ->{

                                BaseResource.voidSuccess()
                            }

                            406 ->  {

                                forceUpdateApp()

                                BaseResource.forceUpdate()
                            }

                            else -> BaseResource.onFailure(message = getLocalizeMessage(key = DEFAULT_ERROR_MESSAGE_KEY))
                        }

                    }else{

                        BaseResource.forceUpdate()
                    }


                }
            } else{

                if (!forceLogOutSession){

                try {

                    val gson = Gson()

                    val error = object : TypeToken<BaseResponse>() {}.type

                    val errorResponse: BaseResponse = gson.fromJson(response.errorBody()!!.charStream(), error)

                    val errors: List<ErrorsItem> = errorResponse.errors?: emptyList()

                    if (errors.isNotEmpty()) {

                        errors.forEach {  errorsItem ->

                            errMessage += if (errors.size>1){

                                "${getLocalizeMessage(key = errorsItem.message)}\n\n"
                            }else{
                                "${getLocalizeMessage(key = errorsItem.message)}\n\n"
                            }
                        }
                    } else {

                        errMessage = getLocalizeMessage(key = errorResponse.message)
                    }

                }catch (e:Exception){
                    errMessage = getLocalizeMessage(key = DEFAULT_ERROR_MESSAGE_KEY)

                }

                when(response.code()){

                    AndroidUtilities.ACCOUNT_BLOCKED ->{

                        blockUser(errMessage)

                        return BaseResource.userBlocked()
                    }
                    AndroidUtilities.SESSION_EXPIRED ->{

                        sessionExpired(errMessage)

                        return BaseResource.userBlocked()
                    }

                    AndroidUtilities.FORCE_UPDATE->  {

                        forceUpdateApp()

                        return BaseResource.forceUpdate()

                    }else ->{


                    BaseResource.onFailure(message = errMessage, code = response.code().toString())

                    }

                }

                }else{

                    BaseResource.forceUpdate()
                }
            }


        } catch (e: Exception) {

            return  if (isConnectedToInternet(context)){
                if (!forceLogOutSession){

                    BaseResource.onFailure(message = getLocalizeMessage(key = DEFAULT_ERROR_MESSAGE_KEY))

                }else{

                    BaseResource.forceUpdate()
                }



            }else{

                BaseResource.internetConnectionError()
            }

        }
    }
}


object UserBlocked {

    fun forceUpdateApp(){

       update.postValue(true)
    }

    fun blockUser(message:String){

        blocked.postValue(message)
    }

    fun sessionExpired(message:String){

        sessionExpire.postValue(message)
    }
    fun forceLogoutSessionExpired(message:String){

        forceLogoutSessionExpire.postValue(message)
    }

}

private val blocked = SingleLiveEvent<String>()

val blockedUser : LiveData<String> get() = blocked


private val sessionExpire = SingleLiveEvent<String>()

val sessionExpireAccount : LiveData<String> get() = sessionExpire


private val forceLogoutSessionExpire = SingleLiveEvent<String>()

val forceLogoutSessionExpireAccount : LiveData<String> get() = forceLogoutSessionExpire

private val update = SingleLiveEvent<Boolean>()

val forceUpdate : LiveData<Boolean> get() =  update
