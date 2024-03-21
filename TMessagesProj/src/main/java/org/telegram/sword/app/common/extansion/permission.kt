package org.telegram.sword.app.common.extansion

import android.app.Activity
import android.content.pm.PackageManager
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment

fun Fragment.isGranted(permissions: Array<String>, requestCode: Int): Boolean {

    var granted = true

    permissions.forEach {
        if (ContextCompat.checkSelfPermission(
                requireContext(),
                it
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermissions(permissions, requestCode)
            granted = false
        }

    }
    return granted
}

fun Activity.isGranted(permissions: Array<String>, requestCode: Int): Boolean {

    var granted = true

    permissions.forEach {
        if (ContextCompat.checkSelfPermission(
               this,
                it
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermissions(permissions, requestCode)
            granted = false
        }

    }
    return granted
}