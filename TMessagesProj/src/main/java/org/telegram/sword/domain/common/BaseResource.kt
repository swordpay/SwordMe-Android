package org.telegram.sword.domain.common

import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.DEFAULT_ERROR_MESSAGE_KEY
import org.telegram.sword.app.common.base.getLocalizeMessage


enum class Status {

        SUCCESS,

        FAILURE,

        NO_INTERNET_CONNECTION,

        FORCE_UPDATE,

        USER_BLOCKED,

        VOID_SUCCESS,

    }

data class BaseResource<out T>(val code:String?=null, val status: Status = Status.SUCCESS, val data: T? = null, val message: String = getLocalizeMessage(key = DEFAULT_ERROR_MESSAGE_KEY)) {

    companion object {

        fun <T> onSuccess(data: T,message: String = EMPTY): BaseResource<T> {

            return BaseResource(status = Status.SUCCESS, data = data, message = message)
        }

        fun <T> onFailure(code:String = EMPTY, message: String, data: T? = null): BaseResource<T> {

            return BaseResource(status = Status.FAILURE, data = data, message = message,code = code)
        }
        

        fun <T> internetConnectionError(): BaseResource<T> {

            return BaseResource(status = Status.NO_INTERNET_CONNECTION)
        }

        fun <T> forceUpdate(): BaseResource<T> {

            return BaseResource(status = Status.FORCE_UPDATE)
        }
        fun <T> userBlocked(): BaseResource<T> {

            return BaseResource(status = Status.USER_BLOCKED)
        }

        fun <T> voidSuccess(): BaseResource<T> {

            return BaseResource(status = Status.VOID_SUCCESS)
        }
    }
}