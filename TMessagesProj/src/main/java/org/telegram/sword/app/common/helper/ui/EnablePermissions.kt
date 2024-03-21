package org.telegram.sword.app.common.helper.ui

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Dialog
import android.content.Intent
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.net.Uri
import android.provider.Settings
import android.view.Window
import androidx.appcompat.widget.AppCompatTextView
import androidx.core.content.ContextCompat.startActivity
//import kotlinx.android.synthetic.main.copy_text_item.*
//import kotlinx.android.synthetic.main.dialog_message.*
//import kotlinx.android.synthetic.main.dialog_message.view.*
//import kotlinx.android.synthetic.main.go_to_settings.*
import org.telegram.messenger.R

@SuppressLint("SetTextI18n")
fun showPermissionsEnableDialog(
    context:Activity,
    permissionName: String,
) {

    val dialog = Dialog(context)
    context.runOnUiThread {
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
        dialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
        dialog.setCancelable(false)
        dialog.setContentView(R.layout.go_to_settings)

        val permissionsTitle = dialog.findViewById<AppCompatTextView>(R.id.permissionsTitle)
        val cancelGoToSettings = dialog.findViewById<AppCompatTextView>(R.id.cancelGoToSettings)
        val goToSettings = dialog.findViewById<AppCompatTextView>(R.id.goToSettings)

        permissionsTitle.text = permissionsTitle.text.toString()+" $permissionName"

        cancelGoToSettings.setOnClickListener {

            dialog.dismiss()
        }
        goToSettings.setOnClickListener {

            dialog.dismiss()

            Intent(
                Settings.ACTION_APPLICATION_DETAILS_SETTINGS,

                Uri.parse("package:${context.packageName}")

            ).apply {

                addCategory(Intent.CATEGORY_DEFAULT)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(context,this,null)
            }
        }
        try {

                dialog.show()

        }catch (e:Exception){ Unit }

    }
}