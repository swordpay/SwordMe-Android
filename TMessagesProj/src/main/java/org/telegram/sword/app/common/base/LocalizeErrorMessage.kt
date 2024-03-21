package org.telegram.sword.app.common.base

import android.content.Context
import android.content.res.Resources
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.DEFAULT_ERROR_MESSAGE_KEY
import org.telegram.sword.app.common.AppConst.Key.EMAIL_EXIST
import org.telegram.sword.app.common.AppConst.Key.USER_NAME_EXIST

var appResources: Resources? = null

    private var localizeErrorMessage:HashMap<String,String> = HashMap()

    private var defaultErrorMessage:String =
        EMPTY

    fun collectErrorMessageLocalize(conText: Context){

       defaultErrorMessage = conText.getString(R.string.SomethingWentWrong)

       localizeErrorMessage.clear()

       localizeErrorMessage = hashMapOf(

           DEFAULT_ERROR_MESSAGE_KEY to conText.getString(R.string.SomethingWentWrong),
           EMAIL_EXIST to conText.getString(R.string.EmailExist),
           USER_NAME_EXIST to conText.getString(R.string.UserNameExist),
           "err_code_invalid" to conText.getString(R.string.errCodeInvalid),
           "err_code_not_exists" to conText.getString(R.string.errCodeNotExists),
           "err_user_wrong_credentials" to conText.getString(R.string.errUserWrongCredentials),
           "err_is_phone_number" to conText.getString(R.string.errIsPhoneNumber),
           "err_user_unauthorized" to conText.getString(R.string.userUnauthorized),
           "err_user_with_email_not_found" to conText.getString(R.string.userWithEmailNotFound),
           "err_user_phone_verification_required" to conText.getString(R.string.phoneVerificationRequired),
           "err_user_not_verified" to conText.getString(R.string.errUserNotVerified),
           "err_user_already_exists" to conText.getString(R.string.errUserAlreadyExists),
           "err_user_name_already_exists" to conText.getString(R.string.errUserNameAlreadyExists),
           "err_user_forbidden" to conText.getString(R.string.errUserForbidden),
           "err_user_phone_already_exists" to conText.getString(R.string.errUserPhoneAlreadyExists),
           "err_user_email_already_exists" to conText.getString(R.string.errUserPhoneAlreadyExists),
           "err_user_email_not_verified" to conText.getString(R.string.errUserEmailNotVerified),
           "err_unknown_error" to conText.getString(R.string.SomethingWentWrong),
           "err_code_lifetime_ends" to conText.getString(R.string.errCodeLifetimeEnds),
           "err_kyc_verification_already_passed" to conText.getString(R.string.errKycVerificationAlreadyPassed),
           "err_kyc_verification_not_exists" to conText.getString(R.string.errKycVerificationNotExists),
           "err_invalid_value" to conText.getString(R.string.errInvalidValue),
           "err_firstname_is_not_empty"     to conText.getString(R.string.err_firstname_is_not_empty),
           "err_lastname_is_not_empty"      to conText.getString(R.string.err_lastname_is_not_empty),
           "err_username_is_not_empty"      to conText.getString(R.string.err_username_is_not_empty),
           "err_countryid_is_not_empty"     to conText.getString(R.string.err_countryid_is_not_empty),
           "err_birthday_is_not_empty"      to conText.getString(R.string.err_birthday_is_not_empty),
           "err_type_is_not_empty"          to conText.getString(R.string.err_type_is_not_empty),
           "err_phone_is_not_empty"         to conText.getString(R.string.err_phone_is_not_empty),
           "err_email_is_not_empty"         to conText.getString(R.string.err_email_is_not_empty),
           "err_firstname_matches"          to conText.getString(R.string.err_firstname_matches),
           "err_lastname_matches"           to conText.getString(R.string.err_lastname_matches),
           "err_username_matches"           to conText.getString(R.string.err_username_matches),
           "err_countryid_matches"          to conText.getString(R.string.err_countryid_matches),
           "err_birthday_matches"           to conText.getString(R.string.err_birthday_matches),
           "err_type_matches"               to conText.getString(R.string.err_type_matches),
           "err_phone_matches"              to conText.getString(R.string.err_phone_matches),
           "err_email_matches"              to conText.getString(R.string.err_email_matches),
           "err_cards_limit_exceeded"              to conText.getString(R.string.err_cards_limit_exceeded),
           "err_password_matches"              to conText.getString(R.string.err_password_matches),
           "err_username_is_valid_user_name"              to conText.getString(R.string.err_username_matches),
           "err_transactions_insufficient_funds" to conText.getString(R.string.err_transactions_insufficient_funds),
           "err_service_unavailable"                to                       conText.getString(R.string.err_service_unavailable),
           "err_account.iban_is_iban"                to                       conText.getString(R.string.invalidIban),
           "err_account.bic_is_bic"                to                       conText.getString(R.string.invalidBic),
           "err_code_not_exists"                to                       conText.getString(R.string.err_code_not_exists),
           "err_chat_room_not_exists"                to                       conText.getString(R.string.err_chat_room_not_exists),
           "err_user_not_found"                to                       conText.getString(R.string.err_user_not_found),
           "err_payment_not_found"                to                       conText.getString(R.string.err_payment_not_found),
           "err_undefined"                to                       conText.getString(R.string.SomethingWentWrong),
           "err_crypto_invalid_memo"            to                       conText.getString(R.string.invalidMemo),
           "err_user_deactivated"            to                       conText.getString(R.string.deactivateUserError),
           "err_user_password_same_as_old"            to                       conText.getString(R.string.passwordSameAsOld),
           "err_username_is_not_restricted_value"            to                       conText.getString(R.string.unavailableUsernameError),
           "err_username_is_not_restricted_value"            to                       conText.getString(R.string.unavailableUsernameError),
           "err_card_payment_required"            to                       conText.getString(R.string.err_card_payment_required),
           "err_login_is_phone_number_or_email"            to                       conText.getString(R.string.err_login_is_phone_number_or_email),
           "err_unhandled"            to                      defaultErrorMessage,
           "err_channelname_is_not_restricted_value"            to                     conText.getString(R.string.err_channelname_is_not_restricted_value),
           "err_same_bank_account"            to                     conText.getString(R.string.err_same_bank_account),
           )
    }


    fun getLocalizeMessage(key:String?) = localizeErrorMessage[key] ?: key?: defaultErrorMessage

