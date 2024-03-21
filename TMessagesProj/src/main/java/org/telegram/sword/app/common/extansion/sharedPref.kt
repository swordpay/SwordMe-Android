package org.telegram.sword.app.common.extansion

import android.content.Context
import org.telegram.sword.app.common.AppConst.EMPTY

fun Context.putShared(key: String, value: String) {
    val editor = this.getSharedPreferences(key, Context.MODE_PRIVATE)?.edit()
    editor?.putString(key, value)
    editor?.apply()
}

fun Context.getShared(key: String): String {
    val sharedPreferences = this.getSharedPreferences(key, Context.MODE_PRIVATE)
    val result = sharedPreferences.getString(key,
        EMPTY
    )
    return result.toString()
}

fun putSharedJava(context:Context,key: String, value: String) {
    val editor = context.getSharedPreferences(key, Context.MODE_PRIVATE)?.edit()
    editor?.putString(key, value)
    editor?.apply()
}

fun getSharedJava(context:Context,key: String): String {
    val sharedPreferences = context.getSharedPreferences(key, Context.MODE_PRIVATE)
    val result = sharedPreferences.getString(key,
        EMPTY
    )
    return result.toString()
}

fun Context.getSharedBul(key: String): Boolean {
    val sharedPreferences = this.getSharedPreferences(key, Context.MODE_PRIVATE)
    return sharedPreferences.getBoolean(key, false)
}

fun Context.putSharedBul(key: String, value: Boolean) {
    val editor = this.getSharedPreferences(key, Context.MODE_PRIVATE)?.edit()
    editor?.putBoolean(key, value)
    editor?.apply()
}

fun getSharedBulJava(context:Context,key: String): Boolean {
    val sharedPreferences = context.getSharedPreferences(key, Context.MODE_PRIVATE)
    return sharedPreferences.getBoolean(key, false)
}

fun putSharedBulJava(context:Context,key: String, value: Boolean) {
    val editor = context.getSharedPreferences(key, Context.MODE_PRIVATE)?.edit()
    editor?.putBoolean(key, value)
    editor?.apply()
}


