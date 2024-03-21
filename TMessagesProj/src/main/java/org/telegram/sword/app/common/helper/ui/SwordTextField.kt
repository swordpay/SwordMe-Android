package org.telegram.sword.app.common.helper.ui

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Typeface
import android.text.InputFilter
import android.text.method.DigitsKeyListener
import android.text.method.HideReturnsTransformationMethod
import android.text.method.PasswordTransformationMethod
import android.util.AttributeSet
import android.view.View
import android.view.inputmethod.EditorInfo
import android.widget.LinearLayout
import androidx.appcompat.widget.AppCompatEditText
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.core.widget.addTextChangedListener
import androidx.core.widget.doBeforeTextChanged
import androidx.core.widget.doOnTextChanged
import androidx.lifecycle.LiveData
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.SwordTextField.AMOUNT
import org.telegram.sword.app.common.AppConst.SwordTextField.CARD
import org.telegram.sword.app.common.AppConst.SwordTextField.COUNTRY
import org.telegram.sword.app.common.AppConst.SwordTextField.DATE
import org.telegram.sword.app.common.AppConst.SwordTextField.EMAIL
import org.telegram.sword.app.common.AppConst.SwordTextField.EXPIRES
import org.telegram.sword.app.common.AppConst.SwordTextField.EXPIRES_MAX_YEAR_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.FIRST_OR_LAST_NAME
import org.telegram.sword.app.common.AppConst.SwordTextField.NAME
import org.telegram.sword.app.common.AppConst.SwordTextField.PASSWORD
import org.telegram.sword.app.common.AppConst.SwordTextField.PHONE
import org.telegram.sword.app.common.AppConst.SwordTextField.PHONE_OR_EMAIL
import org.telegram.sword.app.common.AppConst.SwordTextField.TEXT
import org.telegram.sword.app.common.AppConst.SwordTextField.USER_NAME
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_EMPTY
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_LENGTH_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_SUCCESS
import org.telegram.sword.app.common.AppConst.SwordTextField.WITHDRAW
import org.telegram.sword.app.common.AppConst.SwordTextField.YEAR_BIG_DATE_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.YEAR_OLD_18_ERROR
import org.telegram.sword.app.common.base.appResources
import org.telegram.sword.app.common.colors.AppColors.DARK_TEXT_COLOR
import org.telegram.sword.app.common.colors.AppColors.ERROR_COLOR
import org.telegram.sword.app.common.colors.AppColors.FIRST_APP_BLUE
import org.telegram.sword.app.common.enums.CardTypes
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

data class DoBeforeTextChangedModel(

   var text:CharSequence?=null,
   var start:Int,
   var count:Int,
   var after:Int
)

data class DoOnTextChangedModel(

    var it:CharSequence?=null,
    var index:Int,
    var afterTextCount:Int,

)

class SwordTextField:LinearLayout  {

    private val _message = SingleLiveEvent<Boolean>()
    val validationMessage: LiveData<Boolean> get() = _message

    private val doBeforeTextChanged = SingleLiveEvent<DoBeforeTextChangedModel>()
    val doBeforeTextChangedObserve: LiveData<DoBeforeTextChangedModel> get() = doBeforeTextChanged

    private val doOnTextChanged = SingleLiveEvent<DoOnTextChangedModel>()
    val doOnTextChangedObserve: LiveData<DoOnTextChangedModel> get() = doOnTextChanged


    private val text = SingleLiveEvent<String>()

    private var cardNum = EMPTY
    var errorMessage = EMPTY
    var validateType = NAME
    var expires = EMPTY
    var date = EMPTY
    var symbolCount = 3
    var valid = true
    var showPass = false
    var singleSpace = false
    var regex = EMPTY
    private var hsFocus = false



    private var mRootContainer: View = inflate(context, R.layout.sword_text_field, this)
    var parent: LinearLayoutCompat = mRootContainer.findViewById(R.id.parent)
    var swordTextField: AppCompatEditText = mRootContainer.findViewById(R.id.textField)
    private var toggle: AppCompatImageView = mRootContainer.findViewById(R.id.toggle)
    private var clearBtn: AppCompatImageView = mRootContainer.findViewById(R.id.clearBtn)
    var startIcon: AppCompatImageView = mRootContainer.findViewById(R.id.startIcon)
    var borderView: LinearLayoutCompat = mRootContainer.findViewById(R.id.line)
    var errorText: AppCompatTextView = mRootContainer.findViewById(R.id.editTextValidationError)
    private var mOnFocusChangeListener: OnFocusChangeListener? = null

    constructor(context: Context) : this(context, null)
    constructor(context: Context, attrs: AttributeSet?) : this(context, attrs, 0)

    @SuppressLint("ResourceType")
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(
        context,
        attrs,
        defStyleAttr
    ) {

        toggle.setOnClickListener {
            passwordToggleClick()
        }


        context.theme.obtainStyledAttributes(
            attrs,
            R.styleable.SwordTextField, 0, 0
        ).apply {

            try {

                if (hasValue(R.styleable.SwordTextField_hint)) {
                    swordTextField.hint = getString(R.styleable.SwordTextField_hint)
                }


                if (hasValue(R.styleable.SwordTextField_errorText)) {

                    errorMessage = if(getString(R.styleable.SwordTextField_errorText).toString().isNotEmpty()) getString(R.styleable.SwordTextField_errorText).toString()

                    else getString(R.styleable.SwordTextField_errorText).toString()
                }

                if (hasValue(R.styleable.SwordTextField_android_inputType)) {
                    swordTextField.inputType =
                        getInt(R.styleable.SwordTextField_android_inputType, EditorInfo.TYPE_NULL)
                }
                if (hasValue(R.styleable.SwordTextField_textFieldType)) {
                    validateType = getString(R.styleable.SwordTextField_textFieldType).toString()
                }
                if (hasValue(R.styleable.SwordTextField_symbolCount)) {
                    symbolCount = getInt(R.styleable.SwordTextField_symbolCount, 3)
                }
                if (hasValue(R.styleable.SwordTextField_singleSpace)) {
                    singleSpace = getBoolean(R.styleable.SwordTextField_singleSpace, false)

                }
                if (hasValue(R.styleable.SwordTextField_maxLength)){
                    swordTextField.filters = arrayOf(InputFilter.LengthFilter(getInteger(R.styleable.SwordTextField_maxLength,1000)))
                }
                if (hasValue(R.styleable.SwordTextField_singleLine)) {
                    swordTextField.isSingleLine = getBoolean(R.styleable.SwordTextField_singleLine, true)
                }else{
                    swordTextField.isSingleLine = true
                }
                swordTextField.typeface = Typeface.createFromAsset(context.assets, "poppins_regular.ttf")
                toggleEnabled(getBoolean(R.styleable.SwordTextField_passwordToggleEnabled, false))

                clearBtn.setOnClickListener {

                    swordTextField.setText(EMPTY)

                    borderView.setBackgroundResource(R.color.textFieldLineColor)

                }

                startIcon.show(validateType == PHONE)

                if (validateType == PHONE){


                   swordTextField.keyListener = DigitsKeyListener.getInstance("0123456789")

                }

                if (validateType == AMOUNT){


                    swordTextField.keyListener = DigitsKeyListener.getInstance("0123456789.,")

                }

                if (validateType == COUNTRY) {

                    clearBtn.isEnabled = false

                    clearBtn.setImageResource(R.drawable.drop_down_icon)

                    clearBtn.show()

                    swordTextField.isFocusable = false

                } else{

                    clearBtn.isEnabled = true

                    clearBtn.setImageResource(R.drawable.x_icon)

                    clearBtn.gone()

                    swordTextField.isFocusable = true

                }

                swordTextField.addTextChangedListener {

                    clearBtn.show(!it.isNullOrEmpty())

                    text.value = it.toString()

                    if (singleSpace) {

                        swordTextField.singleSpace(it = it.toString())

                    } else {

                        swordTextField.removeSpace(it = it.toString())

                    }



                    when (validateType) {

                        AMOUNT->{

                            if (text().isEmpty() || text()=="0"){

                                changeBorderColor(status = VALID_EMPTY)
                            }

                        }

                        PASSWORD -> {
                            if(!showPass){
                                swordTextField.transformationMethod = PasswordTransformationMethod.getInstance()
                                swordTextField.setSelection(swordTextField.length())
                            }
                            swordTextField.isValidPassword(length = symbolCount)
                                .also { valid ->
                                    changeBorderColor(status = valid.message)
                                }
                        }
                        EMAIL -> {
                            swordTextField.emailValidation().also { valid ->
                                changeBorderColor(status = valid.message)
                            }
                        }
                        CARD -> {
                            if (cardNum!=it.toString()){
                                cardNum = it.toString()
                                swordTextField.cardNumberFormat(it = it.toString()).also { valid->
                                    changeBorderColor(status = valid.message)
                                }
                                when(cardNum.typesOfCreditCards()){
                                    CardTypes.MASTER ->{
                                        startIcon.setImageResource(R.drawable.ic_master_card_icon)
                                        startIcon.show()
                                    }
                                    CardTypes.VISA ->{
                                        startIcon.setImageResource(R.drawable.ic_visa_card_icon)
                                        startIcon.show()
                                    }
                                    CardTypes.UNKNOWN ->{
                                        startIcon.gone()
                                        if (!swordTextField.text.isNullOrEmpty()){
                                            changeBorderColor(status = VALID_ERROR)
                                        }

                                    }
                                }

                            }
                        }
                        DATE -> {
                            if (date!=it.toString()){
                                date = it.toString()
                                swordTextField.dateFormat(it = it.toString()).also { valid->
                                    changeBorderColor(status = valid.message)
                                }

                            }
                        }
                        PHONE -> {
                            if (date!=it.toString()){
                                date = it.toString()
                                swordTextField.phoneFormat(it = it.toString()).also { valid->
                                    changeBorderColor(status = valid.message)
                                }
                            }

                        }

                        PHONE_OR_EMAIL -> {
                          swordTextField.emailOrPhoneValidation(it = it.toString()).also { valid->
                              changeBorderColor(status = valid.message)
                          }
                        }
                        EXPIRES -> {
                            if (expires != it.toString()){
                                expires = it.toString()
                                swordTextField.cardExpiresFormat(it = it.toString()).also { valid->
                                    changeBorderColor(status = valid.message)
                                }
                            }
                        }
                        TEXT -> {
                            swordTextField.lengthValidation(length = symbolCount)
                                .also { valid ->
                                    changeBorderColor(status = valid.message)
                                }
                        }

                        USER_NAME -> {
                            userNameFormat(it = it.toString()).also { valid->
                                changeBorderColor(status = valid.message)
                            }
                        }

                        FIRST_OR_LAST_NAME -> {
                            userFirstOrLastNameFormat(it = it.toString()).also { valid->
                                changeBorderColor(status = valid.message)
                            }
                        }
                        WITHDRAW -> {
                            isValidAnyRegex(it = it.toString(),regex).also { valid->
                                changeBorderColor(status = valid.message)
                            }
                        }

                        else -> TextFieldValidStatus()
                    }
                }

                swordTextField.setOnFocusChangeListener { _, hasFocus ->

                    if (hasFocus) {

                        hsFocus = hasFocus

                        if (valid) {

                            borderView.setBackgroundResource(R.color.firstAppBlue)

                        } else {

                            borderView.setBackgroundResource(R.color.errorColor)
                        }

                    } else {

                        hsFocus=hasFocus

                        if (valid) {

                            borderView.setBackgroundResource(R.color.textFieldLineColor)

                        } else {

                            borderView.setBackgroundResource(R.color.errorColor)
                        }
                    }

                }


                swordTextField.doBeforeTextChanged { text, start, count, after ->


                    doBeforeTextChanged.value = DoBeforeTextChangedModel(text = text,start = start,count = count,after = after)


                }

                swordTextField.doOnTextChanged { it, index, _, afterTextCount ->


                    doOnTextChanged.value = DoOnTextChangedModel(it =it,index =index,afterTextCount = afterTextCount)

                }



            } finally {
                recycle()
            }
        }
    }

    fun setSelection(changIndex:Int){

        try { swordTextField.setSelection(changIndex) } catch (_: Exception) { Unit }
    }

    private fun passwordToggleClick() {
         if (showPass) {
            showPass = !showPass
            swordTextField.also {
                it.transformationMethod = PasswordTransformationMethod.getInstance()
                it.setSelection(it.length())


            }
             toggle.setImageResource(R.drawable.ic_hide_pass)

        } else {
            showPass = !showPass
            swordTextField.also {
                it.transformationMethod = HideReturnsTransformationMethod.getInstance()
                it.setSelection(it.length())

            }
             toggle.setImageResource(R.drawable.ic_show_pass)

        }
    }

    private fun toggleEnabled(enabled: Boolean) {
        if (enabled) {
            toggle.show()
        } else {
            toggle.gone()
        }
    }

    private fun changeBorderColor(status: String) {

        showErrorText(status = status)

        when (status) {
            VALID_SUCCESS -> {
                valid = true
                if (hsFocus) { borderView.setBackgroundResource(R.color.firstAppBlue) }
                _message.value = true
                appResources?.getColor(R.color.dark_text_color)?.let { swordTextField.setTextColor(it) }

            }
            VALID_ERROR -> {

                valid = false
                borderView.setBackgroundResource(R.color.errorColor)
                _message.value = false
                appResources?.getColor(R.color.errorColor)?.let { swordTextField.setTextColor(it) }
            }
            VALID_EMPTY -> {

                valid = true
                if (hsFocus) { borderView.setBackgroundResource(R.color.firstAppBlue) }
                _message.value = false
                appResources?.getColor(R.color.dark_text_color)?.let { swordTextField.setTextColor(it) }
            }
        }
    }

    private fun showErrorText(status: String) {
        when (validateType) {
            NAME -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text =
                        EMPTY
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            TEXT -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = errorMessage
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            DATE -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.invalidDate)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                    YEAR_OLD_18_ERROR -> errorText.text = resources.getString(R.string.year18_OldErrorMessage)
                    YEAR_BIG_DATE_ERROR -> errorText.text = resources.getString(R.string.yearBigErrorMessage)
                }
            }
            PASSWORD -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.passwordErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                    VALID_LENGTH_ERROR -> errorText.text = resources.getString(R.string.passwordLengthErrorMessage)
                }
            }
            EMAIL -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.emailErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            CARD -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.bankCardErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            PHONE -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.phoneErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            PHONE_OR_EMAIL -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.invalidCredential)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }
            USER_NAME -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.userNameErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                    VALID_LENGTH_ERROR -> errorText.text = resources.getString(R.string.userNameLengthErrorMessage)
                }
            }
            EXPIRES -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.expiresErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                    EXPIRES_MAX_YEAR_ERROR -> errorText.text = resources.getString(R.string.expiresMaxYearErrorMessage)
                }
            }

            WITHDRAW -> {
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = resources.getString(R.string.externalAddressErrorMessage)
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }

            FIRST_OR_LAST_NAME -> {
                when (status) {
                    VALID_LENGTH_ERROR -> errorText.text =

                        when(swordTextField.hint){

                            resources.getString(R.string.firstName) ->resources.getString(R.string.firstNameLengthErrorMessage)

                            else -> resources.getString(R.string.lastNameLengthErrorMessage)
                        }

                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = errorMessage
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }

            else ->{
                when (status) {
                    VALID_SUCCESS -> errorText.text =
                        EMPTY
                    VALID_ERROR -> errorText.text = errorMessage
                    VALID_EMPTY -> errorText.text =
                        EMPTY
                }
            }

        }

    }


    override fun setOnFocusChangeListener(l: OnFocusChangeListener) {
        mOnFocusChangeListener = l
    }

    fun setupRegexp(regexp:String){

        regex = regexp
    }

    fun showError(error:String,status: String){

        if (errorText.text.toString() == error){

            return
        }

        errorText.text =  error

        when (status) {
            VALID_SUCCESS -> {

                valid = true

                if (hsFocus) { borderView.setBackgroundColor(FIRST_APP_BLUE)}

                 swordTextField.setTextColor(DARK_TEXT_COLOR)
            }
            VALID_ERROR -> {

                valid = false

                borderView.setBackgroundColor(ERROR_COLOR)

                this.swordTextField.setTextColor(ERROR_COLOR)
            }

            VALID_EMPTY -> {

                valid = true

                if (hsFocus) { borderView.setBackgroundColor(FIRST_APP_BLUE)}

                swordTextField.setTextColor(DARK_TEXT_COLOR)

            }
        }

    }

    fun setDigits(digits:String){

        swordTextField.keyListener = DigitsKeyListener.getInstance(digits)
    }

}