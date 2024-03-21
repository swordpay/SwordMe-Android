package org.telegram.sword

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Application
import android.os.Bundle
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

@SuppressLint("StaticFieldLeak")
private var activeActivity: Activity? = null

class App: Application() {

    override fun onCreate() {

        putSharedBul(AppState.OPENED_HOME_PAGE_KEY,false)

        super.onCreate()

        ModelPreferencesManager.with(this)

        GlobalContext.startKoin {

            androidContext(this@App)

            modules(listOf(viewModelModule, useCaseModule, apiServiceModule))
        }

        setupActivityListener()

        val appLanguage = getShared(key = Key.APP_LANGUAGE)

        val context =   LocaleHelper().setLocale(this, appLanguage)

        appResources = context.resources?:resources

    }


    private fun setupActivityListener() {
        registerActivityLifecycleCallbacks(object : ActivityLifecycleCallbacks {
            override fun onActivityCreated(activity: Activity, savedInstanceState: Bundle?) {

                activeActivity = activity
            }
            override fun onActivityStarted(activity: Activity) {}
            override fun onActivityResumed(activity: Activity) {

            }

            override fun onActivityPaused(activity: Activity) {
//                activeActivity = null
            }

            override fun onActivityStopped(activity: Activity) {}
            override fun onActivitySaveInstanceState(activity: Activity, outState: Bundle) {}
            override fun onActivityDestroyed(activity: Activity) {}
        })
    }

    fun getActiveActivity(): Activity? {
        return activeActivity
    }


}