package org.telegram.sword.app.common.helper.db

import org.telegram.sword.app.common.AppConst.DbKey.SAVE_CRYPTO_ASSETS
import org.telegram.sword.app.common.AppConst.DbKey.SAVE_USER
import org.telegram.sword.domain.binance.model.CryptoAssetsResponseModel
import org.telegram.sword.domain.me.model.MeData
import org.telegram.sword.domain.me.model.MeResponse
import org.telegram.sword.domain.me.model.User

fun getUser(): MeData= MeData(

    appMode = "REGULAR",
    user = User(
        id = "userId",
        firstName = "First Name",
        lastName = "Last Name",
        phone = "000000",
        username="UserName",
        isEmailVerified = true,
        isPhoneVerified = true,
        type ="",
        status = "active",
        createdAt = "",
        updatedAt = "",
        hasCryptoAccount = true,
        hasFiatAccount = true
    )


)

fun saveUser(user: MeResponse? = null){
    ModelPreferencesManager.put(user?.data,SAVE_USER)
}

fun getCryptoAssets(): CryptoAssetsResponseModel?= ModelPreferencesManager.get<CryptoAssetsResponseModel>(SAVE_CRYPTO_ASSETS)

fun saveCryptoAssets(assets: CryptoAssetsResponseModel? = null){
    ModelPreferencesManager.put(assets,SAVE_CRYPTO_ASSETS)
}


