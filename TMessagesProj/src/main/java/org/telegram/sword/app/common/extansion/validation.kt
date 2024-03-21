package org.telegram.sword.app.common.extansion

import android.annotation.SuppressLint
import android.content.Context
import android.os.Build
import android.os.VibrationEffect
import android.os.Vibrator
import android.util.TypedValue
import androidx.appcompat.widget.AppCompatEditText
import com.google.i18n.phonenumbers.NumberParseException
import com.google.i18n.phonenumbers.PhoneNumberUtil
import com.google.i18n.phonenumbers.Phonenumber
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.SwordTextField.EXPIRES_MAX_YEAR_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_EMPTY
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_LENGTH_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.VALID_SUCCESS
import org.telegram.sword.app.common.AppConst.SwordTextField.YEAR_BIG_DATE_ERROR
import org.telegram.sword.app.common.AppConst.SwordTextField.YEAR_OLD_18_ERROR
import org.telegram.sword.app.common.enums.CardTypes
import org.telegram.sword.app.common.helper.ui.SwordTextField
import java.util.*
import java.util.regex.Pattern

data class TextFieldValidStatus(var message: String = VALID_EMPTY)

fun SwordTextField.text() = this.swordTextField.text.toString()

fun SwordTextField.setText(text: String) = this.swordTextField.setText(text)

fun AppCompatEditText.emailValidation(): TextFieldValidStatus {

    return when {
        this.text.toString().isEmpty() -> {

            TextFieldValidStatus(message = VALID_EMPTY)

        }
        this.text.toString().isValidEmail() -> {

            TextFieldValidStatus(message = VALID_SUCCESS)

        }
        else -> {

            TextFieldValidStatus(message = VALID_ERROR)
        }
    }
}

@SuppressLint("SetTextI18n")
fun AppCompatEditText.emailOrPhoneValidation(it: String): TextFieldValidStatus {

    this.removeSpace(it)

    return if (this.text.toString().isEmpty()) {

        TextFieldValidStatus(message = VALID_EMPTY)

    } else if (this.text.toString().isValidEmail()) {

        TextFieldValidStatus(message = VALID_SUCCESS)

    } else if ((!this.text.toString().startsWith("+") && "+${this.text}".trim()
            .isValidPhone()) || this.text.toString().trim().isValidPhone()
    ) {

        if (!this.text.toString().startsWith("+")) {
            this.setText("+${this.text}")
            this.setSelection(this.text.toString().length)
        }

        TextFieldValidStatus(message = VALID_SUCCESS)
    } else {

        TextFieldValidStatus(message = VALID_ERROR)
    }
}


fun AppCompatEditText.lengthValidation(length: Int): TextFieldValidStatus {
    return if (this.text.toString().length < length && this.text.toString().isNotEmpty()) {

        TextFieldValidStatus(message = VALID_ERROR)

    } else if (this.text.toString().isEmpty()) {

        TextFieldValidStatus(message = VALID_EMPTY)
    } else {

        TextFieldValidStatus(message = VALID_SUCCESS)
    }
}

fun AppCompatEditText.isValidPassword(length: Int): TextFieldValidStatus {

    return if (this.text.toString().length < length && this.text.toString().isNotEmpty()) {

        TextFieldValidStatus(message = VALID_LENGTH_ERROR)

    } else if (this.text.toString().isEmpty()) {

        TextFieldValidStatus(message = VALID_EMPTY)

    } else if (this.text.toString().isValidPass()) {

        TextFieldValidStatus(message = VALID_SUCCESS)
    } else {
        TextFieldValidStatus(message = VALID_ERROR)
    }
}

fun AppCompatEditText.singleSpace(it: String) {

    if (it.startsWith(" ")) {

        this.setText(it.replace(" ", ""))

        this.setSelection(this.text.toString().length)
    }
    if (it.contains("  ")) {

        this.setText(it.replace("  ", " "))

        this.setSelection(this.text.toString().length)
    }
}

fun AppCompatEditText.cardNumberFormat(it: String): TextFieldValidStatus {

    val userInput = it.replace("[^\\d]".toRegex(), "")

    if (userInput.length <= 16) {

        val sb = StringBuilder()

        for (i in userInput.indices) {

            if (i % 4 == 0 && i > 0) {
                sb.append(" ")
            }
            sb.append(userInput[i])
        }
        this.setText(sb.toString())
    }
    this.setSelection(this.text.toString().length)

    return when {
        it.isEmpty() -> {
            TextFieldValidStatus(message = VALID_EMPTY)
        }
        it.length < 19 -> {
            TextFieldValidStatus(message = VALID_ERROR)
        }
        else -> {
            TextFieldValidStatus(message = VALID_SUCCESS)
        }
    }
}

fun AppCompatEditText.dateFormat(it: String): TextFieldValidStatus {
    val userInput = it.replace("[^\\d]".toRegex(), "")
    if (userInput.length <= 8) {
        val sb = StringBuilder()
        for (i in userInput.indices) {
            if (i % 2 == 0 && i > 0) {
                if (i == 2 || i == 4) {
                    sb.append("/")
                }

            }
            sb.append(userInput[i])
        }
        this.setText(sb.toString())
    }
    this.setSelection(this.text.toString().length)

    return TextFieldValidStatus(message = this.text.toString().trim().isValidDate())

}

fun AppCompatEditText.phoneFormat(it: String): TextFieldValidStatus {

//    val sb = StringBuilder()
//
//    sb.append(userInput)
//
//    this.setText(sb.toString())
//
////    this.setSelection(this.text.toString().length)

    return if (this.text.toString().trim().isNotEmpty()) {

        if (this.text.toString().trim().isValidPhone()) {

            TextFieldValidStatus(message = VALID_SUCCESS)
        } else {
            TextFieldValidStatus(message = VALID_ERROR)
        }

    } else {
        TextFieldValidStatus(message = VALID_EMPTY)
    }


}


@SuppressLint("SimpleDateFormat")
fun AppCompatEditText.cardExpiresFormat(it: String): TextFieldValidStatus {

    val userInput = it.replace("[^\\d]".toRegex(), "")

    if (userInput.length <= 4) {
        val sb = StringBuilder()
        for (i in userInput.indices) {
            if (i % 2 == 0 && i > 0) {
                sb.append(" / ")
            }
            sb.append(userInput[i])
        }
        this.setText(sb.toString())
    }

    this.setSelection(this.text.toString().length)
    val calendar = Calendar.getInstance()

    return when {
        it.isEmpty() -> {
            TextFieldValidStatus(message = VALID_EMPTY)
        }
        it.length < 7 -> {
            TextFieldValidStatus(message = VALID_ERROR)
        }
        else -> {
            val currentYear = calendar.get(Calendar.YEAR).toString().run {
                substring(length - 2, length).toInt()
            }
            val currentMonth = calendar.get(Calendar.MONTH) + 1
            try {


                val month = it.substring(0, 2).toInt()

                val year = it.substring(it.length - 2, it.length).toInt()

                if (it.substring(0, 2).toInt() in 1..12) {

                    if ((month <= currentMonth && year > currentYear) ||

                        (month > currentMonth && year >= currentYear)

                    ) {

                        if (year - currentYear < 3 || (year - currentYear == 3 && month <= currentMonth)) {

                            TextFieldValidStatus(message = VALID_SUCCESS)
                        } else {

                            TextFieldValidStatus(message = EXPIRES_MAX_YEAR_ERROR)
                        }


                    } else {

                        TextFieldValidStatus(message = VALID_ERROR)
                    }

                } else {

                    TextFieldValidStatus(message = VALID_ERROR)
                }

            } catch (e: Exception) {
                TextFieldValidStatus(message = VALID_ERROR)
            }

        }
    }
}


fun AppCompatEditText.removeSpace(it: String) {
    if (it.contains(" ")) {
        this.setText(it.replace(" ", ""))
        this.setSelection(this.text.toString().length)
    }
}


@SuppressLint("MissingPermission")
fun Context.vibrate(duration: Long) {
    val vibrator = this.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
    if (Build.VERSION.SDK_INT >= 26) {
        vibrator.vibrate(VibrationEffect.createOneShot(duration, VibrationEffect.DEFAULT_AMPLITUDE))
    } else {
        vibrator.vibrate(duration)
    }
}

fun String.isValidPass(): Boolean {

    this.let {
        val passwordPattern =
            "^(?=.*\\p{Ll})(?=.*\\p{Lu})(?=.*\\p{N})(?=.*[\\p{P}\\p{S}\\p{Sc}]).{8,}\$"
        val passwordMatcher = Regex(passwordPattern)
        return passwordMatcher.find(this) != null
    }
}

fun String.isValidEmail(): Boolean {

    this.let {

        val emailPattern =
            "[a-zA-Z0-9+._%\\-]{1,256}" + "@" + "[a-zA-Z0-9][a-zA-Z0-9\\-]{0,64}" + "(" + "\\." + "[a-zA-Z0-9][a-zA-Z0-9\\-]{1,25}" + ")+"
        val emailMatcher = Regex(emailPattern)
        return emailMatcher.find(this) != null
    }
}

fun userNameFormat(it: String): TextFieldValidStatus {

    return if (it.length < 2) {
        if (it.isEmpty()) {
            TextFieldValidStatus(message = VALID_EMPTY)
        } else {
            TextFieldValidStatus(message = VALID_LENGTH_ERROR)
        }

    } else {
        if (it.replace(" ", "").isValidUserName()) {
            TextFieldValidStatus(message = VALID_SUCCESS)
        } else {
            TextFieldValidStatus(message = VALID_ERROR)
        }
    }
}

fun String.isValidUserName(): Boolean {

    val userNamePattern = "^[a-zA-Z\\d](?:[a-zA-Z\\d]|-(?=[a-zA-Z\\d])){0,38}\$"

    val userNameMatcher = Regex(userNamePattern)
    return userNameMatcher.find(this) != null
}

fun String.isValidAnyRegex(regexp: String): Boolean {

    val userNameMatcher = Regex(regexp)
    return userNameMatcher.find(this) != null
}


fun isValidAnyRegex(it: String, regexp: String): TextFieldValidStatus {

    return if (it.isEmpty()) {

        TextFieldValidStatus(message = VALID_EMPTY)

    } else {

        if (it.isValidAnyRegex(regexp = regexp)) {

            TextFieldValidStatus(message = VALID_SUCCESS)

        } else {

            TextFieldValidStatus(message = VALID_ERROR)
        }

    }
}

fun userFirstOrLastNameFormat(it: String): TextFieldValidStatus {

    return if (it.isEmpty()) {

        TextFieldValidStatus(message = VALID_EMPTY)

    } else {
        if (it.length < 2) {

            TextFieldValidStatus(message = VALID_LENGTH_ERROR)
        } else {

            if (it.isValidFirstOrLastName()) {

                TextFieldValidStatus(message = VALID_SUCCESS)

            } else {

                TextFieldValidStatus(message = VALID_ERROR)
            }
        }
    }
}

fun String.isValidFirstOrLastName(): Boolean {


    val userNameMatcher = Regex("^[a-zA-Z](?:[a-zA-Z]|[' -](?=[a-zA-Z])){0,38}\$")

    return userNameMatcher.find(this) != null
}

fun String.isValidPhone(): Boolean {

    val phoneUtil = PhoneNumberUtil.getInstance()
    return try {
        val swissNumberProto: Phonenumber.PhoneNumber? = phoneUtil.parse("+$this", "")
        phoneUtil.isValidNumber(swissNumberProto)

    } catch (e: NumberParseException) {
        false
    }
}

fun String.isValidDate(): String {

    return if (this.length == 10) {
        dateDifference(this)
    } else {
        VALID_ERROR
    }

}

private fun dateDifference(userDate: String): String {
    val calendar = Calendar.getInstance()
    val day = calendar.get(Calendar.DAY_OF_MONTH)
    val month = calendar.get(Calendar.MONTH) + 1
    val year = calendar.get(Calendar.YEAR)

    val userMonth = userDate.substring(0, 2)
    val userDay = userDate.substring(3, 5)
    val userYear = userDate.substring(6, userDate.length)

    return if (userDay.toInt() in 1 until 32 && userMonth.toInt() in 1 until 13 && userYear.toInt() > 1) {

        if (year - userYear.toInt() > 120) {
            VALID_ERROR
        } else {
            if (year - userYear.toInt() > 18) {
                VALID_SUCCESS
            } else if (year - userYear.toInt() == 18) {
                if (month > userMonth.toInt()) {
                    VALID_SUCCESS
                } else if (month == userMonth.toInt()) {
                    if (day >= userDay.toInt()) {
                        VALID_SUCCESS
                    } else {
                        YEAR_OLD_18_ERROR
                    }
                } else {

                    YEAR_OLD_18_ERROR
                }
            } else {
                if (year == userYear.toInt()) {
                    if (month == userMonth.toInt()) {
                        if (userDay.toInt() > day) {
                            YEAR_BIG_DATE_ERROR
                        } else {
                            YEAR_OLD_18_ERROR
                        }
                    } else if (month < userMonth.toInt()) {
                        YEAR_BIG_DATE_ERROR
                    } else {
                        YEAR_OLD_18_ERROR
                    }
                } else if (year < userYear.toInt()) {
                    YEAR_BIG_DATE_ERROR

                } else {
                    YEAR_OLD_18_ERROR
                }

            }
        }
    } else {
        VALID_ERROR
    }

}

fun String.typesOfCreditCards(): CardTypes {
    val visaPattern = Pattern.compile("^4[0-9][0-9]{14}$")
    val masterPattern = Pattern.compile("^5[1-5][0-9]{14}$")
    return when {
        visaPattern.matcher(this.replace(" ", "")).matches() -> {
            CardTypes.VISA
        }
        masterPattern.matcher(this.replace(" ", "")).matches() -> {
            CardTypes.MASTER
        }
        else -> {
            CardTypes.UNKNOWN
        }
    }
}

@SuppressLint("UseCompatLoadingForDrawables")
fun SwordTextField.repeatPassValidation(pass: SwordTextField): Boolean {
    var valid = false

    if (this.text() == pass.text()) {
        valid = true
        pass.errorText.text = EMPTY
        this.errorText.text = EMPTY
        this.borderView.setBackgroundResource(R.color.appBlue)
        pass.borderView.setBackgroundResource(R.color.textFieldLineColor)
        this.valid = true
        pass.valid = true
    } else {
        valid = false
        this.errorText.text = resources.getString(R.string.confirmPasswordErrorMessage)
        pass.errorText.text = resources.getString(R.string.confirmPasswordErrorMessage)
        this.borderView.setBackgroundResource(R.color.errorColor)
        pass.borderView.setBackgroundResource(R.color.errorColor)
        this.valid = false
        pass.valid = false

    }

    return valid
}

fun String.isCorrectPrecision(precisionCount: Int): Boolean {

    var isCorrect = true

    if (this.replace(",", ".").contains(".")) {

        isCorrect = this.length - this.indexOf(".") - 1 <= precisionCount

    }

    return isCorrect

}

fun String.isCorrectBeforePrecision(precisionCount: Int): Boolean {

    var isCorrect = true

    if (this.replace(",", ".").contains(".")) {

        isCorrect = this.indexOf(".") <= precisionCount

    }

    return isCorrect

}

fun AppCompatEditText.maxSize(
    text: String,
    index: Int,
    afterTextCount: Int,
    prefixCount: Int,
    suffixCount: Int,
    allTextSize: Int
) {

    var myText = text

    try {

        if (text.isNotEmpty()) {

            if (myText.contains(".")) {

                if (myText.indexOf(".") > prefixCount) {

                    myText = myText.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                    this.setText(myText)

                    this.setSelection(index)

                }

                if (myText.length - myText.indexOf(".") > suffixCount + 1) {

                    myText = myText.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                    this.setText(myText)

                    this.setSelection(index)

                }


            } else {

                if (myText.length > allTextSize) {

                    myText = myText.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                    this.setText(myText)

                    this.setSelection(index)

                }

            }

        }


    } catch (e: Exception) {


    }

}


fun Double?.toCorrectString(): String {

    var correctString = EMPTY

    try {


        if (this == null) {

            correctString = EMPTY

        } else if (this.toString().trim().lowercase().contains("e-")) {


            correctString = String.format("%.8f", this)


        } else {

            if (this.toString().trim().lowercase().contains("e")) {

                correctString = String.format("%.8f", this)

            } else {

                correctString = this.toString().trim()
            }

        }

        var st = correctString

        return if (correctString.isNotEmpty()) {

            if (!correctString.contains("e")) {

                if (correctString.contains(".")) {

                    var i = correctString.length - 1

                    while (correctString[i] == '0') {

                        st = st.removeSuffix("0")

                        i--
                    }


                    if (st.endsWith("0")) {

                        st = st.removeSuffix("0")
                    }

                    if (st.endsWith(".")) {

                        st = st.removeSuffix(".")
                    }

                    st

                } else {

                    correctString
                }

            } else {

                correctString.trim()
            }


        } else {
            correctString
        }
    } catch (e: Exception) {

      return  this.toString().trim()

    }
}

fun toCorrectStringForJava(amount: String?): String {

    return try {

        if (amount == null) {

            EMPTY
        } else if (amount.trim().lowercase().contains("e-")) {


            String.format("%.8f", amount.trim().toDouble())


        } else {

            if (amount.toString().trim().lowercase().contains("e")) {

                String.format("%.8f", amount)

            } else {

                amount
            }


        }


    } catch (e: Exception) {


        amount?.trim() ?: EMPTY


    }


}

fun String.numberStringRemoveAllOtherSymbol(): String {

    var correctNum = EMPTY

    val suffix = if (this.endsWith(",") || this.endsWith(".")) "." else EMPTY

    this.forEach {

        if (it.toString().contains("[0-9]".toRegex())) {

            correctNum += it
        }

    }

    return if (correctNum.isNotEmpty()) correctNum + suffix else EMPTY

}


fun String.changePrecisionCount(count: Int): String {

    var correctAmount = this


    if (this.contains(".")) {

        if (this.length - 1 - this.lastIndexOf(".") > count) {

            val precision = if (count > 0) {

                count + 1

            } else {

                count
            }

            correctAmount = this.substring(0, this.lastIndexOf(".") + precision)
        }


    }



    return correctAmount

}

fun String.doubleToLongString(): String {

    return try {
        if (this.formatStringToDouble() - this.formatStringToString().toLong() > 0) {

            this

        } else {

            this.formatStringToString().toLong().toString()
        }
    } catch (e: Exception) {

        this
    }

}


fun AppCompatEditText.autoTextSize(initialTextSize: Float, startChangeSizeTextCount: Int) {

    var autoSize = initialTextSize

    if (!this.text.isNullOrEmpty()) {

        if (this.text.toString().length > startChangeSizeTextCount) {

            autoSize =
                (initialTextSize - (this.text.toString().length - startChangeSizeTextCount) * 1.8).toFloat()

        } else {
            autoSize = initialTextSize
        }
    }

    this.setTextSize(TypedValue.COMPLEX_UNIT_DIP, autoSize)

}


fun AppCompatEditText.startManipulationForPartCount(
    num: String,
    maxIntegerPartCount: Int,
    maxDecimalPartCount: Int,
    beforeAmountText: String,
    changIndex: Int,
    afterTextCount: Int
) {


    var text = num



    try {


        if (text.isEmpty()) {
            return
        }


        if (num.contains('.') && !num.isCorrectPrecision(maxDecimalPartCount)) {

            text = num.changePrecisionCount(maxDecimalPartCount)
        }


        try {

            var ind = changIndex

            if (ind >= num.length) {

                ind = num.length - 1

                if (ind < 0) {

                    ind = 0
                }
            }

            if (num[ind] == ',' && num.contains(".") && text != "," && text != ".") {

                text = EMPTY

                num.forEachIndexed { index, c ->

                    if (index != ind) {

                        text += c
                    }
                }
                setText(text)

                try {
                    setSelection(changIndex)
                } catch (_: Exception) {
                    Unit
                }

            }
        } catch (_: Exception) {
            Unit
        }



        if (num.contains(".") && num.length > 1) {

            if (num.indexOf(".") != num.lastIndexOf(".")) {

                text = EMPTY

                num.forEachIndexed { index, c ->

                    if (index != changIndex) {

                        text += c
                    }
                }

                setText(text)

                try {
                    setSelection(changIndex)
                } catch (_: Exception) {
                    Unit
                }
            }

        }

        if (text.isEmpty()) {
            return
        }

        if (text == "," || text == ".") {

            text = EMPTY

            setText(text)

        } else if (text.endsWith(",")) {

            text = text.substring(0, text.length - 1) + "."

            setText(text)

            try {
                setSelection(text.length)
            } catch (_: Exception) {
                Unit
            }

        }

        if (text.contains("..")) {

            text = text.replace("..", ".")

            setText(text)

            try {
                setSelection(changIndex)
            } catch (_: Exception) {
                Unit
            }

        }

        if (text.isEmpty()) {
            return
        }

        val regexPattern = if (maxDecimalPartCount > 0) {

            """^\d{1,$maxIntegerPartCount}(?:\.\d{1,$maxDecimalPartCount})?$"""
        } else {

            """^\d{1,$maxIntegerPartCount}(?:\.\d+)?$"""
        }

        val it = text.formatStringToString()

        val index =
            if (changIndex - (text.length - it.length) < 0) 0 else changIndex - (text.length - it.length)

        var lastIndex = numberFormatString(it).length

        val successIndex = changIndex + afterTextCount


        if (!it.matches(Regex(regexPattern)) && it.isNotEmpty()) {

            try {

                if (it.length > maxIntegerPartCount && !it.contains('.') && beforeAmountText.contains(
                        '.'
                    )
                ) {

                    setText(numberFormatString(beforeAmountText))

                    setSelection(numberFormatString(beforeAmountText).indexOf('.') + 1)

                    try {
                        setSelection(numberFormatString(beforeAmountText).indexOf('.') + 1)
                    } catch (_: Exception) {
                        Unit
                    }

                } else {

                    it.toDouble()

                    if (!it.endsWith('.') && !it.endsWith(',')) {

                        val t = it.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                        setText(numberFormatString(t))

                        try {
                            setSelection(changIndex)
                        } catch (_: Exception) {
                            Unit
                        }

                    }

                }


            } catch (_: Exception) {

                try {
                    val t = it.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                    setText(numberFormatString(t))


                    try {
                        setSelection(successIndex)
                    } catch (e: Exception) {
                        Unit
                    }

                } catch (_: Exception) {
                    Unit
                }

            }

        } else {

            if (this.text.toString().isNotEmpty()) {

                if (changIndex < this.text.toString().length - 1) {

                    lastIndex = changIndex
                }

            }


            if (this.text.toString() != numberFormatString(it)) {

                if (it == "0.0") {

                    setText(EMPTY)

                } else {

                    setText(numberFormatString(it))
                }

                try {
                    setSelection(lastIndex)
                } catch (_: Exception) {
                    Unit
                }

            }


        }
    } catch (_: Exception) {

        setText(EMPTY)
    }
}

fun SwordTextField.startManipulationForPartCount(
    num: String,
    maxIntegerPartCount: Int,
    maxDecimalPartCount: Int,
    beforeAmountText: String,
    changIndex: Int,
    afterTextCount: Int
) {


    var text = num



    try {

        if (text.isEmpty()) {

            return
        }


        if (num.contains('.') && !num.isCorrectPrecision(maxDecimalPartCount)) {

            text = num.changePrecisionCount(maxDecimalPartCount)

        }

//
//            num.formatStringToString().length-1>maxIntegerPartCount){
//
//            text = num.removeSuffix(".")
//
//            text = num.removeSuffix(",")
//
//            setText(numberFormatString(text))
//
//            setSelection(numberFormatString(text).length)
//
//        }

        try {

            var ind = changIndex

            if (ind >= num.length) {

                ind = num.length - 1

                if (ind < 0) {

                    ind = 0
                }
            }

            if (num[ind] == ',' && num.contains(".") && text != "," && text != ".") {

                text = EMPTY

                num.forEachIndexed { index, c ->

                    if (index != ind) {

                        text += c
                    }
                }
                setText(text)

                try {
                    setSelection(changIndex)
                } catch (_: Exception) {
                    Unit
                }

            }
        } catch (_: Exception) {
            Unit
        }



        if (num.contains(".") && num.length > 1) {

            if (num.indexOf(".") != num.lastIndexOf(".")) {

                text = EMPTY

                num.forEachIndexed { index, c ->

                    if (index != changIndex) {

                        text += c
                    }
                }

                setText(text)

                try {
                    setSelection(changIndex)
                } catch (_: Exception) {
                    Unit
                }
            }

        }

        if (text.isEmpty()) {
            return
        }

        if (text == "," || text == ".") {

            text = EMPTY

            setText(text)

        } else if (text.endsWith(",")) {

            text = text.substring(0, text.length - 1) + "."

            setText(text)

            try {
                setSelection(text.length)
            } catch (_: Exception) {
                Unit
            }

        }

        if (text.contains("..")) {

            text = text.replace("..", ".")

            setText(text)

            try {
                setSelection(changIndex)
            } catch (_: Exception) {
                Unit
            }

        }

        if (text.isEmpty()) {
            return
        }

        val regexPattern = if (maxDecimalPartCount > 0) {

            """^\d{1,$maxIntegerPartCount}(?:\.\d{1,$maxDecimalPartCount})?$"""
        } else {

            """^\d{1,$maxIntegerPartCount}(?:\.\d+)?$"""
        }


        val it = text.formatStringToString()

        val index =
            if (changIndex - (text.length - it.length) < 0) 0 else changIndex - (text.length - it.length)

        var lastIndex = numberFormatString(it).length

        val successIndex = changIndex + afterTextCount


        if (!it.matches(Regex(regexPattern)) && it.isNotEmpty()) {

            try {

                if (it.length > maxIntegerPartCount && !it.contains('.') && beforeAmountText.contains(
                        '.'
                    )
                ) {

                    setText(numberFormatString(beforeAmountText))

                    setSelection(numberFormatString(beforeAmountText).indexOf('.') + 1)

                    try {
                        setSelection(numberFormatString(beforeAmountText).indexOf('.') + 1)
                    } catch (_: Exception) {
                        Unit
                    }

                } else {

                    it.toDouble()

                    if (!it.endsWith('.') && !it.endsWith(',')) {

                        val t = it.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                        setText(numberFormatString(t))

                        try {
                            setSelection(changIndex)
                        } catch (_: Exception) {
                            Unit
                        }

                    }

                }


            } catch (_: Exception) {

                try {
                    val t = it.replaceRange(IntRange(index, index + afterTextCount - 1), EMPTY)

                    setText(numberFormatString(t))


                    try {
                        setSelection(successIndex)
                    } catch (e: Exception) {
                        Unit
                    }

                } catch (_: Exception) {
                    Unit
                }

            }

        } else {

            if (text().isNotEmpty()) {

                if (changIndex < text().length - 1) {

                    lastIndex = changIndex
                }

            }


            if (text() != numberFormatString(it)) {

                if (it == "0.0") {

                    setText(EMPTY)

                } else {

                    setText(numberFormatString(it))
                }

                try {
                    setSelection(lastIndex)
                } catch (_: Exception) {
                    Unit
                }

            }


        }
    } catch (_: Exception) {

        setText(EMPTY)
    }
}


fun AppCompatEditText.beforeValidText(maxIntegerPartCount: Int, maxDecimalPartCount: Int): String? {

    val regexPattern = if (maxDecimalPartCount > 0) {

        """^\d{1,$maxIntegerPartCount}(?:\.\d{1,$maxDecimalPartCount})?$"""
    } else {

        """^\d{1,$maxIntegerPartCount}(?:\.\d+)?$"""
    }

    val text = this.text.toString().formatStringToString()


    return if (text.matches(Regex(regexPattern)) || text.isEmpty()) {

        text

    } else {

        null
    }

}

fun SwordTextField.beforeValidText(maxIntegerPartCount: Int, maxDecimalPartCount: Int): String? {

    val regexPattern = if (maxDecimalPartCount > 0) {

        """^\d{1,$maxIntegerPartCount}(?:\.\d{1,$maxDecimalPartCount})?$"""
    } else {

        """^\d{1,$maxIntegerPartCount}(?:\.\d+)?$"""
    }

    val text = this.text().formatStringToString()


    return if (text.matches(Regex(regexPattern)) || text.isEmpty()) {

        text
    } else {

        null
    }

}
