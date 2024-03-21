package org.telegram.sword.app.common.base

import android.app.Dialog
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.content.res.Configuration
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.net.Uri
import android.os.Bundle
import android.text.method.DigitsKeyListener
import android.view.View
import android.view.Window
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import android.widget.ProgressBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.core.widget.addTextChangedListener
import androidx.lifecycle.ViewModel
import androidx.viewbinding.ViewBinding
import com.makeramen.roundedimageview.RoundedImageView
import org.telegram.messenger.MessagesController
import org.telegram.messenger.R
import org.telegram.messenger.UserConfig
import org.telegram.messenger.utils.SwordColors
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.APP_LANGUAGE
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.helper.db.saveUser
import org.telegram.sword.app.common.helper.function.updateAccessToken
import org.telegram.sword.app.common.helper.function.updateRefreshToken
import org.telegram.sword.app.common.helper.locale.LocaleHelper
import org.telegram.sword.domain.common.*
import org.telegram.ui.LaunchActivity

enum class DialogStatus {
    SUCCESS, ERROR ,INFO,FORCE_UPDATE
}

fun Context.prefs(): SharedPreferences = getSharedPreferences("your_prefs_name", Context.MODE_PRIVATE)


class FontSizeManager(private val prefs: SharedPreferences) {

    private val unsetFontSizeValue = -1f

    var fontSize: FontSize
        get() {
            val scale = prefs.getFloat("font_scale", unsetFontSizeValue)
            return if (scale == unsetFontSizeValue) {
                FontSize.DEFAULT
            } else {
                FontSize.values().first { fontSize -> fontSize.scale == scale }
            }
        }
        set(value) {
            prefs.edit()
                .putFloat("font_scale", value.scale)
                .apply()
        }

}

enum class FontSize(val scale: Float) {
    SMALL(0.7f),
    DEFAULT(1.0f),
    LARGE(1.3f)
}

abstract class BaseActivity<BindingType : ViewBinding,ViewModelType : ViewModel>() : AppCompatActivity() {

    val loading: Dialog by lazy { loading() }


    private var isShowMessageDialog = false


    var nexPage = 0

    var hasNext = true

    protected lateinit var binding: BindingType
    protected lateinit var viewModel: ViewModelType

    abstract fun getViewBinding(): BindingType

    abstract fun onActivityCreated()

    abstract fun configUi()

    abstract fun clickListener(bind:BindingType)

    lateinit var fontSizeManager: FontSizeManager
    override fun attachBaseContext(newBase: Context) {
        fontSizeManager = FontSizeManager(newBase.prefs())
        val newConfig = Configuration(newBase.resources.configuration)
        newConfig.fontScale = fontSizeManager.fontSize.scale
        applyOverrideConfiguration(newConfig)
        super.attachBaseContext(newBase)
    }

    private fun updateFontSize(fontSize: FontSize) {
        fontSizeManager.fontSize = fontSize

    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        updateFontSize(FontSize.DEFAULT)
        binding = getViewBinding()
        setContentView(binding.root)
        onActivityCreated()
        configUi()
        clickListener(binding)
        forceUpdateObserve()
        blockedUserObserve()

    }

    fun changeAppLanguage(language:String){

        putShared(key = APP_LANGUAGE,language)

        val context =  LocaleHelper().setLocale(this, language)
    }



    fun showMessageDialog(
        title: String,
        message: String,
        isDismissDialog:Boolean = true,
        status: DialogStatus = DialogStatus.ERROR,
        isShowDismissBtn:Boolean = false,
        function: (() -> Unit)? = null
    ) {

        val dialog = Dialog(this)
        runOnUiThread {
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
            dialog.setCancelable(false)
            dialog.setContentView(R.layout.dialog_message)
            val dialogTitle: TextView = dialog.findViewById(R.id.dialogTitle)
            val dialogDesc: TextView = dialog.findViewById(R.id.dialogDesc)
            val dialogStatusIcon: RoundedImageView = dialog.findViewById(R.id.messageDialogStatusIcon)
            val dialogBtn: AppCompatTextView = dialog.findViewById(R.id.dialogBtn)
            val dismissBtn: AppCompatTextView = dialog.findViewById(R.id.dismissBtn)
            dialogTitle.text = title
            dialogDesc.text = message.ifEmpty { getLocalizeMessage("") }
            when (status) {
                DialogStatus.SUCCESS -> dialogStatusIcon.setImageResource(R.drawable.success_dialog_icon)
                DialogStatus.ERROR -> dialogStatusIcon.setImageResource(R.drawable.error_dialog_icon)
                DialogStatus.INFO -> dialogStatusIcon.setImageResource(R.drawable.wrong_dialog_icon)
                DialogStatus.FORCE_UPDATE -> dialogStatusIcon.setImageResource(R.drawable.ic_play_market_icon)
            }

            dismissBtn.show(isShowDismissBtn)

            dialogBtn.setOnClickListener {


                isShowMessageDialog = false

                if (isDismissDialog){
                    dialog.dismiss()
                }

                function?.invoke()
            }
            try {
                if (!isShowMessageDialog) {
                    dialog.show()
                    isShowMessageDialog = true
                }
            }catch (e:Exception){ }

        }
    }


    fun showProgressDialogDialog(
        context: Context,
        title: String?=null,
        message: String?=null,
        cancelFun: (() -> Unit)? = null,
        updateProgressFun: (() -> Unit)? = null
    ) {

        try {

            val actionDialog = Dialog(context)


            runOnUiThread {


                actionDialog.requestWindowFeature(Window.FEATURE_NO_TITLE)

                actionDialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))

                actionDialog.setCancelable(false)

                actionDialog.setContentView(R.layout.progress_diaog_view)

                val progressTitle = actionDialog.findViewById<AppCompatTextView>(R.id.progressTitle)

                val progressDesc = actionDialog.findViewById<AppCompatTextView>(R.id.progressDesc)

                val progressDismissBtn = actionDialog.findViewById<AppCompatTextView>(R.id.progressDismissBtn)

                val progressBarHorizontal = actionDialog.findViewById<ProgressBar>(R.id.progressBarHorizontal)

                title?.let { t ->

                    progressTitle.show()

                    progressTitle.text = t

                }

                message?.let { m ->

                    progressDesc.show()

                    progressDesc.text = m

                }

                progressDismissBtn.setOnClickListener {

                    cancelFun?.invoke()
                }


            }


        } catch (e: Exception) {



        }
    }


    private fun forceUpdateObserve(){


        forceUpdate.observe(this){

            if (it){

                showMessageDialog(status = DialogStatus.FORCE_UPDATE,
                    isDismissDialog = false,
                    title =   resources.getString(R.string.updateApp),
                    message = resources.getString(R.string.app_update_info),
                    function = ::goToPlayMarket)

            }
        }
    }
    private fun blockedUserObserve(){


        blockedUser.observe(this){

            showBlockerUserDialog(title="Account blocked!", message = it)

        }

        sessionExpireAccount.observe(this){

            sessionExpireUserDialog()

        }

        forceLogoutSessionExpireAccount.observe(this){

            forceLogoutSessionExpireAccountDialog(message = it)

        }




    }

    fun showBlockerUserDialog(title:String,message:String){

        showActionDialog(
            title = title,
            subTitle = message,
            isSingleButton = true,
            posBtnName = "Ok",
            posBtnFunction = {

                refreshForceLogOutSession()

                MessagesController.getInstance(UserConfig.selectedAccount).performLogout(1)

                openActivity(LaunchActivity())
            }
        )
    }

    fun forceLogoutSessionExpireAccountDialog(message:String){

        showActionDialog(
            title = message,
            isSingleButton = true,
            posBtnName = "Ok",
            posBtnFunction = {

                MessagesController.getInstance(UserConfig.selectedAccount).performLogout(1)

                openActivity(LaunchActivity())
            }
        )
    }

    fun sessionExpireUserDialog(){

        updateAccessToken("", this)

        updateRefreshToken("", this)

        saveUser(null)

    }



     fun goToPlayMarket(){

        try {
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=com.swordpay.me")))
        } catch (e: ActivityNotFoundException) {
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/details?id=com.swordpay.me")))
        }
    }

     fun showActionDialog(
         context:Context =this,
         textLengthForValidation:Int?=null,
         inputType:Int?=null,
         hint: String = EMPTY,
         isShowTextField:Boolean = false,
         title:String = EMPTY,
         subTitle:String = EMPTY,
         errorMessage:String = EMPTY,
         posBtnName:String = EMPTY,
         negBtnName:String = EMPTY,
         isSingleButton:Boolean = false,
         digits:String?=null,
         posBtnCallBackFunction:((text:String,dialog:Dialog,dialogErrorText:AppCompatTextView) -> Unit)? = null,
         negativeBtnCallBackFunction:(() -> Unit)? = null,
         posBtnFunction: (() -> Unit)? = null,
         negativeBtnFunction: (() -> Unit)? = null,
         isDismissDialog:Boolean = true

    ) {
        try {



            val actionDialog = Dialog(context)

            val imm = context.getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager

            runOnUiThread {


                actionDialog.requestWindowFeature(Window.FEATURE_NO_TITLE)

                actionDialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))

                actionDialog.setCancelable(false)

                actionDialog.setContentView(R.layout.text_field_dialog)

                val positiveBtn = actionDialog.findViewById<AppCompatTextView>(R.id.positiveBtn)

                val textFieldDialogTitle = actionDialog.findViewById<AppCompatTextView>(R.id.textFieldDialogTitle)

                val textFieldDialogDesc = actionDialog.findViewById<AppCompatTextView>(R.id.textFieldDialogDesc)

                val textFieldDialogDismissBtn = actionDialog.findViewById<AppCompatTextView>(R.id.textFieldDialogDismissBtn)

                val dialogTextField = actionDialog.findViewById<EditText>(R.id.dialogTextField)

                val dialogErrorText = actionDialog.findViewById<AppCompatTextView>(R.id.dialogErrorText)

                val centerLine = actionDialog.findViewById<LinearLayoutCompat>(R.id.centerLine)

                dialogErrorText.text = errorMessage

                dialogErrorText.gone()


                inputType?.let { dialogTextField.inputType = inputType }

                dialogTextField.hint = hint

                dialogTextField.show(isShowTextField)

                textFieldDialogDismissBtn.show(!isSingleButton)

                centerLine.show(!isSingleButton)

                textFieldDialogDesc.text = subTitle

                textFieldDialogTitle.text = title

                textFieldDialogTitle.show(title.isNotEmpty())

                textFieldDialogDesc.show(subTitle.isNotEmpty())

                positiveBtn.text = posBtnName

                textFieldDialogDismissBtn.text = negBtnName

                digits?.let { dig -> dialogTextField.keyListener = DigitsKeyListener.getInstance(dig) }


                positiveBtn.isEnabled = !isShowTextField

                if (isShowTextField){

                    positiveBtn.setTextColor(SwordColors.HINT)
                }

                dialogTextField.addTextChangedListener {

                    (textLengthForValidation?:1).also { length ->


                        if (it.toString().length >=length){

                            positiveBtn.isEnabled = true

                            positiveBtn.setTextColor(SwordColors.FIRST_APP_BLUE)

                        }else {

                            positiveBtn.isEnabled = false

                            positiveBtn.setTextColor(SwordColors.HINT)
                        }

                    }


                }

                dialogTextField.onFocusChangeListener =

                    View.OnFocusChangeListener { v, isFocused ->

                        if (isFocused) {

                            imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 1)
                        }
                    }


                positiveBtn.setOnClickListener {

                    posBtnFunction?.invoke()

                    posBtnCallBackFunction?.invoke(dialogTextField.text.toString(),actionDialog,dialogErrorText)

                    imm.hideSoftInputFromWindow(dialogTextField.windowToken, 0)

                    if (isDismissDialog){

                        actionDialog.dismiss()
                    }

                }

                textFieldDialogDismissBtn.setOnClickListener {

                    negativeBtnFunction?.invoke()

                    negativeBtnCallBackFunction?.invoke()

                    imm.hideSoftInputFromWindow(dialogTextField.windowToken, 0)

                    if (isDismissDialog){

                        actionDialog.dismiss()
                    }
                }

                if (isShowTextField){
                    val window = actionDialog.window
                    actionDialog.show()
                    window?.clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM)
                    window?.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE)
                }else{

                    actionDialog.show()
                }

            }


        } catch (e: Exception) {



        }

    }



}