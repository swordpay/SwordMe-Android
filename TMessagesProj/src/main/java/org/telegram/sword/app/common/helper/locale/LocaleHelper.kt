package org.telegram.sword.app.common.helper.locale

import android.content.Context
import android.content.res.Configuration
import android.content.res.Resources
import org.telegram.sword.app.common.AppConst.Key.APP_LANGUAGE
import org.telegram.sword.app.common.base.collectErrorMessageLocalize
import org.telegram.sword.app.common.extansion.putShared
import java.util.*


class LocaleHelper {

    fun setLocale(context: Context, language: String): Context {

        context.putShared(key = APP_LANGUAGE,language)

        return updateResourcesLegacy(context, language)

    }


    private fun updateResources(context: Context, language: String): Context? {
        val locale = Locale(language)
        Locale.setDefault(locale)
        val configuration: Configuration = context.resources.configuration
        configuration.setLocale(locale)
        configuration.setLayoutDirection(locale)
        collectErrorMessageLocalize(conText = context.createConfigurationContext(configuration)?:context)
        return context.createConfigurationContext(configuration)
    }


    private fun updateResourcesLegacy(context: Context, language: String): Context {
        val locale = Locale(language)
        Locale.setDefault(locale)
        val resources: Resources = context.resources
        val configuration: Configuration = resources.configuration
        configuration.locale = locale
        configuration.setLayoutDirection(locale)
        resources.updateConfiguration(configuration, resources.displayMetrics)
        collectErrorMessageLocalize(conText = context)
        return context
    }
}