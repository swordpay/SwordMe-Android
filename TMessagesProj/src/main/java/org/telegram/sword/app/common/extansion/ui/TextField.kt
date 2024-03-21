package org.telegram.sword.app.common.extansion.ui

import androidx.appcompat.widget.AppCompatEditText
import org.telegram.messenger.R
import org.telegram.messenger.databinding.MoneyTextFieldBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.base.appResources
import org.telegram.sword.app.common.extansion.gone
import org.telegram.sword.app.common.extansion.loadCryptoImage
import org.telegram.sword.app.common.extansion.numberFormatString
import org.telegram.sword.app.common.extansion.show
import org.telegram.sword.socket.webSocket.const_.ACCOUNT_CURRENCY

fun MoneyTextFieldBinding.set(isLoadImage:Boolean = true,hint:String = EMPTY, coin:String = EMPTY, name:String = EMPTY, isSingleLine:Boolean = true,showDropDownIcon:Boolean = true,focusable:Boolean = true
,defaultValue:String = EMPTY){



    if (!showDropDownIcon){

        this.coinName.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0)
    }

    if (!focusable){

        this.priceTextField.isFocusable = false
        this.priceTextField.isClickable = false
        this.priceTextField.isEnabled = false
    }

    if (defaultValue.isNotEmpty()){

        this.priceTextField.setText(defaultValue)
    }

    this.priceTextField.isSingleLine = isSingleLine

    this.coinName.text = name

    if (isLoadImage){

        this.coinIcon.loadCryptoImage(coin = coin)
    }

}


fun MoneyTextFieldBinding.startShimmer(){

    this.coinCurrenciesLayout.gone()
    this.shimmer.show()
    this.shimmer.startShimmer()


}

fun MoneyTextFieldBinding.stopShimmer(){

    this.coinCurrenciesLayout.show()
    this.shimmer.gone()
    this.shimmer.stopShimmer()
}


fun MoneyTextFieldBinding.getText() = this.priceTextField.text?.trim().toString()

fun MoneyTextFieldBinding.errorText(errorText:String) {

    if (errorText.isNotEmpty()){

        this.validationErrorText.show()

        this.line.setBackgroundResource(R.color.errorColor)

        appResources?.getColor(R.color.errorColor)?.let { this.priceTextField.setTextColor(it) }


    }else{

        this.validationErrorText.gone()

        this.line.setBackgroundResource(R.color.textFieldLineColor)

        appResources?.getColor(R.color.dark_text_color)?.let { this.priceTextField.setTextColor(it) }
    }

    this.validationErrorText.text = errorText
}

fun AppCompatEditText.setCursorSelection(index:Int?){

    try {
        setSelection(index?:0)
    }catch (_:Exception){}

}


const val MIN_AMOUNT_VALUE_STRING = "1.00"
const val MIN_AMOUNT_VALUE = 1.00

fun minimumAmountErrorMessage(min:String) = "Minimum allowed value is ${numberFormatString(min)}"
fun minimumAmountErrorMessageForSendOrRequest(min:String) = "Amount too low (${ACCOUNT_CURRENCY()}${numberFormatString(min)} min)"
fun maximumAmountErrorMessage(max:String) = "Maximum allowed value is ${numberFormatString(max)}"
fun balanceExceededErrorMessage() = "Balance exceeded."
