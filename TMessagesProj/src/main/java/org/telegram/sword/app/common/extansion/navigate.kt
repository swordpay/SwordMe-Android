package org.telegram.sword.app.common.extansion

import android.annotation.SuppressLint
import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import androidx.browser.customtabs.CustomTabColorSchemeParams
import androidx.browser.customtabs.CustomTabsIntent
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.SHARED
import org.telegram.sword.app.common.AppConst.Key.SHARED_EXTRA
import org.telegram.ui.ActionBar.BaseFragment
import java.io.Serializable


fun Activity.openActivity(activity: Activity, sharedData:Any?=null, sharedExtraData: Serializable?=null,isNewTaskFlag:Boolean = false) {
    val intent = Intent(this, activity::class.java)
    sharedExtraData.let { intent.putExtra(SHARED_EXTRA, it )}
    
    if (sharedData!=null){

        when(sharedData){

            is String->  intent.putExtra(SHARED, sharedData)
            is Int->  intent.putExtra(SHARED, sharedData)
            is Long->  intent.putExtra(SHARED, sharedData)
            is Double->  intent.putExtra(SHARED,  sharedData)
            is Boolean->  intent.putExtra(SHARED,  sharedData)
        }
    }

    if (isNewTaskFlag){

        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
    }

    startActivity(intent)
}

fun Context.openActivity(activity: Activity, sharedData:Any?=null, sharedExtraData: Serializable?=null) {
    val intent = Intent(this, activity::class.java)
    sharedExtraData.let { intent.putExtra(SHARED_EXTRA, it )}

    if (sharedData!=null){

        when(sharedData){

            is String->  intent.putExtra(SHARED, sharedData)
            is Int->  intent.putExtra(SHARED, sharedData)
            is Double->  intent.putExtra(SHARED,  sharedData)
            is Boolean->  intent.putExtra(SHARED,  sharedData)
        }
    }
    startActivity(intent)
}


fun Activity.openChrome(url:String) {
    val intent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
    intent.setPackage("com.android.chrome")
    try {
        startActivity(intent)
    } catch (ex: Exception) {
         try {
             intent.setPackage(null)
             startActivity(intent)

         }catch (e:Exception){
             openCustomChrome(url)
         }

    }
}

fun Fragment.openBottomSheet(fragment: BottomSheetDialogFragment) {
    fragment.show(requireActivity().supportFragmentManager,
        EMPTY
    )
}

fun openBottomSheetInJava(context:BaseFragment, fragment: BottomSheetDialogFragment) {
    fragment.show(context.requireActivity().supportFragmentManager,
        EMPTY
    )
}

fun AppCompatActivity.openBottomSheet(fragment: BottomSheetDialogFragment) {
    fragment.show(supportFragmentManager,
        EMPTY
    )
}



fun Activity.openCustomChrome(url:String){

        val intent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        intent.setPackage("com.android.chrome")
        try {
            startActivity(intent)
        } catch (ex: Exception) {
            try {
                intent.setPackage(null)
                startActivity(intent)

            }catch (e:Exception){
                openCustomChromeBrowser(url)
            }

        }

//
//        openCustomChromeBrowser(url)
//    }

}

fun Activity.openCustomChromeBrowser(url:String){
    try {
        val builder = CustomTabsIntent.Builder()
        val params = CustomTabColorSchemeParams.Builder()
        params.setToolbarColor(ContextCompat.getColor(this, R.color.appBlue))
        builder.setDefaultColorSchemeParams(params.build())
        builder.setShowTitle(false)
        builder.setUrlBarHidingEnabled(false)
        builder.setShareState(CustomTabsIntent.SHARE_STATE_OFF)
        builder.setInstantAppsEnabled(false)
        val customBuilder = builder.build()
        customBuilder.launchUrl(this, Uri.parse(url))
    }catch (e:Exception){
        Unit
    }
}



fun launchNativeApi30(context: Context, uri: Uri?): Boolean {
    val nativeAppIntent = Intent(Intent.ACTION_VIEW, uri)
        .addCategory(Intent.CATEGORY_BROWSABLE)
        .addFlags(
            Intent.FLAG_ACTIVITY_NEW_TASK or
                    Intent.FLAG_ACTIVITY_REQUIRE_NON_BROWSER
        )
    return try {
        context.startActivity(nativeAppIntent)
        true
    } catch (ex: ActivityNotFoundException) {
        false
    }
}

@SuppressLint("QueryPermissionsNeeded")
private fun launchNativeBeforeApi30(context: Context, uri: Uri): Boolean {
    val pm = context.packageManager

    val browserActivityIntent = Intent()
        .setAction(Intent.ACTION_VIEW)
        .addCategory(Intent.CATEGORY_BROWSABLE)
        .setData(Uri.fromParts("https", "", null))
    val specializedActivityIntent = Intent(Intent.ACTION_VIEW, uri)
        .addCategory(Intent.CATEGORY_BROWSABLE)
    specializedActivityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
    context.startActivity(specializedActivityIntent)
    return true
}
fun launchUri(context: Context?, uri: Uri?) {
    val launched = if (Build.VERSION.SDK_INT >= 30) launchNativeApi30(
        context!!,
        uri
    ) else launchNativeBeforeApi30(
        context!!, uri!!
    )
    if (!launched) {
        CustomTabsIntent.Builder()
            .build()
            .launchUrl(context, uri!!)
    }
}

private fun isPackageInstalled(packageName: String, packageManager: PackageManager): Boolean {
    return try {
        packageManager.getPackageInfo(packageName, 0)
        true
    } catch (e: PackageManager.NameNotFoundException) {
        false
    }
}
