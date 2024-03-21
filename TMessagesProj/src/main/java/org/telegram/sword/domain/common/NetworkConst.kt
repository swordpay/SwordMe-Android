package org.telegram.sword.domain.common

import org.telegram.sword.domain.common.BaseUrl.DEV_BASE_URL
import org.telegram.sword.domain.common.BaseUrl.STAGING_BASE_URL
import org.telegram.sword.domain.common.ENV.ENV_STAGING

enum class ConnectTo{

    DEV, STAGING,PROD,
}

internal object ENV{

    const val ENV_STAGING = false

}


 internal object BaseUrl{


     const val DEEPLINK_BASE_URL = "https://DEEPLINK_BASE_URL/"

     const val DEV_BASE_URL = "https://DEV_BASE_URL/"

     const val STAGING_BASE_URL = "https://STAGING_BASE_URL/"

     const val BINANCE_BASE_URL = "https://api.binance.com/"

     const val CRYPTO_IMAGE_URL = "https://CRYPTO_IMAGE_URL/"

     const val F_S = ""

     const val BINANCE_STREAM_URL = "wss://stream.binance.com:9443/stream"

     const val SOCKET_IO_BASE_URL_STG = "wss://SOCKET_IO_BASE_URL_STG/"

     private const val SOCKET_IO_BASE_URL_DEV = "wss://SOCKET_IO_BASE_URL_DEV/"

     val SOCKET_IO_BASE_URL =  if (ENV_STAGING) SOCKET_IO_BASE_URL_STG else SOCKET_IO_BASE_URL_DEV


 }

internal object Version{

    const val V_1 = "v1"

    const val V_2 = "v2"

    const val V_3 = "v3"
}

internal object AppVersion{

    const val PHONE_OS = "android"

}

internal object WebUrl{

    const val PRIVACY = "PRIVACY URL"

    const val TERMS = "TERMS URL"

    const val ABOUT_AS = "ABOUT AS URL"


}

internal object Paging{

    const val PAGE_LIMIT = 15
}


fun swordBaseUrl() = if (ENV_STAGING) STAGING_BASE_URL else DEV_BASE_URL



