package org.telegram.messenger

import android.app.Application
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.GlobalContext
import org.telegram.sword.app.common.AppConst.AppState
import org.telegram.sword.app.common.AppConst.Key
import org.telegram.sword.app.common.base.appResources
import org.telegram.sword.app.common.extansion.getShared
import org.telegram.sword.app.common.extansion.putSharedBul
import org.telegram.sword.app.common.helper.db.ModelPreferencesManager
import org.telegram.sword.app.common.helper.locale.LocaleHelper
import org.telegram.sword.di.apiServiceModule
import org.telegram.sword.di.useCaseModule
import org.telegram.sword.di.viewModelModule

fun appLoadSwordConfig(context: Application){

    context.run {

        putSharedBul(AppState.OPENED_HOME_PAGE_KEY,false)

        ModelPreferencesManager.with(context)

        GlobalContext.startKoin {

            androidContext(context)

            modules(listOf(viewModelModule, useCaseModule, apiServiceModule))
        }

        val appLanguage = getShared(key = Key.APP_LANGUAGE)

        val context =   LocaleHelper().setLocale(this, appLanguage)

        appResources = context.resources?:resources
    }

}