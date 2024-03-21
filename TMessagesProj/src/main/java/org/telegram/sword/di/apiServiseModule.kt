package org.telegram.sword.di

import android.content.Context
import android.util.Log
import okhttp3.Dispatcher
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import org.koin.android.ext.koin.androidContext
import org.koin.dsl.module
import org.telegram.sword.domain.account.apiHelper.AccountApiHelper
import org.telegram.sword.domain.binance.apiHelper.BinanceApiHelper
import org.telegram.sword.domain.chat.apiHelper.ChatApiHelper
import org.telegram.sword.domain.common.BaseUrl.BINANCE_BASE_URL
import org.telegram.sword.domain.common.SwordHttpInterceptor
import org.telegram.sword.domain.common.swordBaseUrl
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit

val apiServiceModule = module {


        factory  { AccountApiHelper(getRetrofitClient(context = androidContext())) }

        factory  { ChatApiHelper(getRetrofitClient(context = androidContext())) }

        factory  { BinanceApiHelper(getRetrofitClient(context = androidContext()), binanceService = getRetrofitClientBinance(context = androidContext())) }

    }

var okHttpClient:OkHttpClient? = null

fun getRetrofitClient(context: Context) : Retrofit {

    var retrofit: Retrofit? = null

    if (retrofit == null) {

        val logging = HttpLoggingInterceptor()
        logging.setLevel(HttpLoggingInterceptor.Level.BODY)
          okHttpClient = OkHttpClient.Builder()
            .connectTimeout(300, TimeUnit.SECONDS)
            .readTimeout(300, TimeUnit.SECONDS)
            .writeTimeout(300, TimeUnit.SECONDS)
            .callTimeout(300, TimeUnit.SECONDS)
            .retryOnConnectionFailure(true)
            .addInterceptor(SwordHttpInterceptor(context =context).providesAuthInterceptor())
            .addInterceptor(HttpLoggingInterceptor { Log.e("swordRetrofit", it) }
                            .setLevel(HttpLoggingInterceptor.Level.BODY)
                            .setLevel(HttpLoggingInterceptor.Level.HEADERS)

            )
            .dispatcher(Dispatcher())
            .addInterceptor(logging)
            .build()


        retrofit = Retrofit.Builder()
            .baseUrl(swordBaseUrl())
            .addConverterFactory(GsonConverterFactory.create())
            .client(okHttpClient!!)
            .build()
    }


    return retrofit!!
}



fun getRetrofitClientBinance(context: Context) : Retrofit {

    var retrofit: Retrofit? = null

    if (retrofit == null) {

        val logging = HttpLoggingInterceptor()
        logging.setLevel(HttpLoggingInterceptor.Level.BODY)
        val okHttpClient = OkHttpClient.Builder()
            .connectTimeout(300, TimeUnit.SECONDS)
            .readTimeout(300, TimeUnit.SECONDS)
            .writeTimeout(300, TimeUnit.SECONDS)
            .callTimeout(300, TimeUnit.SECONDS)
            .retryOnConnectionFailure(true)
            .addInterceptor(HttpLoggingInterceptor { Log.e("swordRetrofit", it) }
                .setLevel(HttpLoggingInterceptor.Level.BODY))
            .dispatcher(Dispatcher())
            .addInterceptor(logging)
            .build()
        retrofit = Retrofit.Builder()
            .baseUrl( BINANCE_BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .client(okHttpClient)
            .build()
    }

    return retrofit!!
}


fun stopAllCalls(){

    okHttpClient?.let { it.dispatcher.cancelAll() }

}
