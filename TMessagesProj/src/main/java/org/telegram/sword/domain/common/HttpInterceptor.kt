package org.telegram.sword.domain.common

import android.annotation.SuppressLint
import android.content.Context
import okhttp3.Interceptor
import okhttp3.Response
import okhttp3.internal.EMPTY_REQUEST
import org.json.JSONObject
import org.telegram.messenger.BuildVars.BUILD_VERSION_STRING
import org.telegram.sword.app.common.AppConst.Key.ACCESS_TOKEN
import org.telegram.sword.app.common.AppConst.Key.REFRESH_TOKEN
import org.telegram.sword.app.common.extansion.putShared
import org.telegram.sword.di.stopAllCalls
import org.telegram.sword.domain.common.AppVersion.PHONE_OS


class SwordHttpInterceptor(private  val context:Context) {

    @SuppressLint("SuspiciousIndentation")
    fun providesAuthInterceptor(): Interceptor = Interceptor {


            var firstRequest: Response? = null

            val requestBuilder = it.request().newBuilder()

            requestBuilder.header("x-os", PHONE_OS)

            requestBuilder.header("x-version-name", BUILD_VERSION_STRING)

            val refreshToken = "Refresh Token"
            val accessToken = "Access token"

            if (accessToken.isNotEmpty()) {

                requestBuilder.header("Authorization", String.format("Bearer %s", accessToken))
            }

            val lastRequest = requestBuilder.build()

            firstRequest = it.proceed(lastRequest)

            val response = firstRequest.networkResponse

            when (response?.code) {

                401 -> {

                    val newReq = it.request()
                        .newBuilder()
                        .header("Content-Type", "application/json")
                        .header("x-version-name", BUILD_VERSION_STRING)
                        .header("x-os", PHONE_OS)
                        .header("Authorization", String.format("Bearer %s", refreshToken))
                        .url("${swordBaseUrl()}${Version.V_1}/auth/refresh")
                        .post(EMPTY_REQUEST)
                        .build()

                    firstRequest.close()

                    firstRequest = it.proceed(newReq)

                    if (firstRequest.isSuccessful) {

                        val body: String = firstRequest.body.string()

                        val responseJSON = JSONObject(body)

                        if (!responseJSON.getJSONObject("data")
                                .isNull("accessToken") && !responseJSON.getJSONObject("data")
                                .isNull("refreshToken")
                        ) {

                            val newAccessToken =
                                responseJSON.getJSONObject("data").getString("accessToken")

                            val newRefreshToken =
                                responseJSON.getJSONObject("data").getString("refreshToken")


                            context.putShared(key = ACCESS_TOKEN, value = newAccessToken)

                            context.putShared(key = REFRESH_TOKEN, value = newRefreshToken)

                            val req = lastRequest.newBuilder()
                                .header("Content-Type", "application/json")
                                .header("Authorization", String.format("Bearer %s", newAccessToken))
                                .header(
                                    "x-version-name",
                                    BUILD_VERSION_STRING
                                )
                                .header("x-os", PHONE_OS)
                                .build()
                            it.proceed(req)

                        } else {

                            stopAllCalls()

                            firstRequest
                        }

                    } else {

                        stopAllCalls()

                        firstRequest

                    }

                }

                else -> firstRequest

            }

    }
}



var forceLogOutSession = false

fun forceLogOutSession() = forceLogOutSession

fun refreshForceLogOutSession(){ forceLogOutSession = false }


