package org.telegram.sword.app.common.AppConst

import org.telegram.sword.app.common.helper.db.getUser


const val CARD_NUMBER_HIDE_SYMBOL="**** **** **** "

public const val PRECISION_2 = 2
public const val PRECISION_8 = 8
const val MAX_INTEGER_PART_COUNT = 12
const val SEC = "sec"
const val EMPTY=""
const val SPACE=" "
const val PERSONAL = "PERSONAL"
const val CREATOR = "CREATOR"
const val APP_ID = "com.sword.dev"
const val CARD_TYPE = "card_type"
const val FIAT_EMOJI = "\uD83D\uDCB0"


internal object RedirectPage{

    const val RESET_PASS = "/reset-password"
    const val CHANGE_PASS = "/change-password"
    const val CARDS = "/cards"
    const val COINS = "/coins"
    const val CARD_ACTIVATION = "/card-activation"
    const val ROOM_PAYMENT = "/rooms-payment"
    const val CARD_CHARGE = "/card-charge"
    const val PAYMENT = "/payment"
    const val ACCEPT = "/transactions"
    const val RESET_PIN = "/card-reset-pin"
    const val ACCOUNTS = "/account"
    const val FIAT_TOP_UP = "/fiat-top-up"
    const val FIAT_TRANSACTION = "/fiat-transactions"
    const val INVITE_FRIENDS = "/invite"
    const val EMAIL_VERIFY = "/verify-email"
}

internal object CardType{

    const val PHYSICAL = "physical"
    const val VIRTUAL = "virtual"
    const val MASTER = "master"
    const val VISA = "visa"

}

internal object ChartInterval{

    const val M_15 = "15m"
    const val H_1 = "1h"
    const val H_2 = "2h"
    const val H_4 = "4h"
    const val D_1 = "1d"
    const val D_2 = "2d"
    const val W_1 = "1w"

}

internal object ChartRange{

    const val D1 = -1
    const val W1 = -7
    const val M1 = -30
    const val M3 = -90
    const val M6 = -180
    const val Y1 = -365

}



internal object Status{

  const val PENDING = "pending"
  const val COMPLETED = "completed"
  const val REJECTED = "rejected"
  const val REFUNDED = "refunded"
  const val CANCELLED = "cancelled"
  const val SEND = "sent"
  const val RECEIVED = "received"
  const val INITIAL = "initial"
}

internal object TransactionType{

    const val SENDER = "sender"
    const val RECEIVER = "receiver"

}

internal object Number{

    const val INVALID_NUMBER = -1

}

internal object Path{

    const val ACTIVATE = "activate"
    const val FREEZE = "freeze"
    const val SHOW_DETAILS = "details"
    const val RESET_PIN = "reset-pin"
    const val BLOCK = "block"
    const val STOLEN = "stolen"
    const val LOST = "lost"

}

internal object CardStatus{

    const val NOT_ACTIVATED = "not_activated"
    const val ACTIVATED = "active"
    const val PAYMENT_REQUIRED = "payment_required"
    const val FROZEN = "frozen"
    const val CLOSED = "closed"
    const val STOLEN = "stolen"
    const val BLOCKED = "blocked"
    const val LOST = "lost"
    const val FRAUD = "fraud"
    const val EXPIRED = "expired"
    const val SECURITY_BLOCK = "security_block"

}

internal object Fcm{
    const val DEVICE_ID = "deviceId"
    const val channelId = "notification_channel"
    const val PUSH_STATUS_SUCCESS = "paymentSuccess"
    const val PUSH_NOTIFICATION_DATA_KEY = "pushKey"
    const val FCM_KYC_VERIFICATION = "KYC_VERIFICATION"
    const val PAY = "PAY"
    const val CHAT = "chat"



}

internal object FcmKey{

    const val KYC_APPROVED = "KYC_APPROVED"
    const val KYC_REJECTED = "KYC_REJECTED"
    const val CRYPTO_TRANSACTION_RECEIVED = "CRYPTO_TRANSACTION_RECEIVED"
    const val FIAT_TRANSACTION_RECEIVED ="FIAT_TRANSACTION_RECEIVED"
    const val CRYPTO_TRANSACTION_ROLLBACK ="CRYPTO_TRANSACTION_ROLLBACK"
    const val CARD_TRANSACTION_INITIATED ="CARD_TRANSACTION_INITIATED"
    const val PAYMENT_GROUP_REQUEST_CREATED ="PAYMENT_GROUP_REQUEST_CREATED"
    const val PAYMENT_DM_REQUEST_CREATED ="PAYMENT_DM_REQUEST_CREATED"
    const val PAYMENT_GROUP_RECEIVED ="PAYMENT_GROUP_RECEIVED"
    const val PAYMENT_DM_RECEIVED ="PAYMENT_DM_RECEIVED"
    const val PAYMENT_GROUP_ACCEPTED ="PAYMENT_GROUP_ACCEPTED"
    const val PAYMENT_DM_ACCEPTED ="PAYMENT_DM_ACCEPTED"
    const val PAYMENT_GROUP_REJECTED ="PAYMENT_GROUP_REJECTED"
    const val PAYMENT_DM_REJECTED ="PAYMENT_DM_REJECTED"
    const val TEXT_MESSAGE ="TEXT_MESSAGE"
    const val PAYMENT_REQUEST_CREATED ="PAYMENT_REQUEST_CREATED"
    const val PAYMENT_RECEIVED ="PAYMENT_RECEIVED"
    const val PAYMENT_ACCEPTED ="PAYMENT_ACCEPTED"
    const val PAYMENT_REJECTED ="PAYMENT_REJECTED"
    const val CARD_STATUS_ACTIVATED ="CARD_STATUS_ACTIVATED"
    const val CARD_STATUS ="CARD_STATUS"
    const val CARD_STATUS_FROZEN ="CARD_STATUS_FROZEN"
    const val CARD_STATUS_BLOCKED ="CARD_STATUS_BLOCKED"
    const val CARD_STATUS_CLOSED ="CARD_STATUS_CLOSED"
    const val CARD_STATUS_STOLEN ="CARD_STATUS_STOLEN"
    const val CARD_STATUS_LOST ="CARD_STATUS_LOST"
    const val CARD_STATUS_FRAUD ="CARD_STATUS_FRAUD"
    const val CARD_STATUS_EXPIRED ="CARD_STATUS_EXPIRED"
    const val CARD_STATUS_SECURITY_BLOCK ="CARD_STATUS_SECURITY_BLOCK"
    const val WITHDRAWAL_FAILED ="WITHDRAWAL_FAILED"
    const val WITHDRAWAL_COMPLETED ="WITHDRAWAL_COMPLETED"

}

internal object RedirectKey{

    const val CARDS = "cards"
    const val COIN = "coin"
    const val QR = "qr"

}

internal object KycRedirectType{

    const val REDIRECT_URL = "redirectUrl"
    const val TOKEN = "token"


}

internal object DeviceBrand{

    const val SAMSUNG = "samsung"
}

internal object SwordTextField{

    const val USER_NAME = "userName"
    const val TEXT="text"
    const val NAME="name"
    const val PASSWORD="password"
    const val EMAIL="email"
    const val CARD="card"
    const val DATE="date"
    const val WITHDRAW="withdraw"
    const val EXPIRES="expires"
    const val PHONE="phone"
    const val AMOUNT="amount"
    const val COUNTRY="country"
    const val PHONE_OR_EMAIL="phoneOrEmail"
    const val FIRST_OR_LAST_NAME="firstOrLastName"

    const val YEAR_OLD_18_ERROR="18YearOldError"
    const val YEAR_BIG_DATE_ERROR="yearBigDateError"
    const val EXPIRES_MAX_YEAR_ERROR="expiresYearError"
    const val VALID_ERROR="valid_error"
    const val VALID_SUCCESS="valid_success"
    const val VALID_EMPTY="valid_empty"
    const val VALID_LENGTH_ERROR="valid_error_length"
}


internal object Key{
    const val WRAP="wrap"
    const val VERIFY_EMAIL="verify_email"
    const val VERIFY_EMAIL_TOKEN="verify_email_token"
    const val MACH="mach"
    const val PHOTO_CAPTURE = 100
    const val VIDEO_CAPTURE = 110
    const val FILE_STORAGE = 155
    const val GALLERY = 200
    const val VIDEO_OR_PHOTO = 200
    const val SUCCESS = "success"
    const val SHARED = "SharedData"
    const val SHARED_EXTRA = "SharedExtraData"
    const val ACCESS_TOKEN = "accessToken"
    const val REFRESH_TOKEN = "refreshToken"
    const val PHONE_NUMBER = "phoneNumber"
    const val IF_GO_TO_SAVED_MESSAGE = "ifGoToSavedMessage"
    const val IMMORTAL_TOKEN = "immortalToken"
    const val APP_LANGUAGE = "app_language"
    const val APP_FIRST_LAUNCH = "app_first_launch"
    const val DEFAULT_ERROR_MESSAGE_KEY = "SomethingWentWrong"
    const val EMAIL_EXIST = "emailExist"
    const val USER_NAME_EXIST = "userNameExist"
    const val FINGERPRINT_KEY = "fingerprintKey"
    const val IS_OPENED_FINGERPRINT = "isOpenedFingerprint"
    const val IS_OPENED_NOTIFICATION = "isOpenedNotification"
    const val IS_DEVICE_REGISTRED = "isDeviceRegistred"
}

internal object PermissionRequestCode{

    const val CAMERA = 300

    const val READ_CONTACTS = 400
}

internal object BuyOrSellType{

    const val BUY = "Buy"

    const val SELL = "Sell"

}

internal object ForAdapter{

    const val SHIMMER = -1
    const val HEADER = 0
    const val LIST_ITEM = 1
    const val FOOTER = 2
    const val REQUEST_DETAILS = 3
    const val SEND_REMINDER = 4
    const val EMPTY_LIST = 5
    const val CHAT_LIST_DATE = 6

}

internal object ForChatAdapter{

    const val MY_MESSAGE = 6
    const val FRIENDS_MESSAGE = 7
    const val MY_REQUEST = 8
    const val FRIENDS_REQUEST = 9
    const val CLOSED_MY_REQUEST = 10
    const val MY_ACCEPT = 11
    const val FRIENDS_ACCEPT = 12
    const val CHAT_FOOTER = 13
    const val MY_REJECT = 14
    const val FRIENDS_REJECT = 15
    const val MY_PAY = 16
    const val FRIENDS_PAY = 17
    const val CARD_BLOCKED = 18
    const val ACTIVE_CARD = 19
}



internal object AppLanguage{
    const val ARM="hy"
    const val ENG=""
}

internal object BalanceType{
    const val ALL="all"
    const val CRYPTO="crypto"
    const val FIAT="fiat"
}

internal object KycStatus{

    const val VERIFIED="active"
    const val NOT_VERIFIED="initial"
    const val IN_PROCESS="in_process"
    const val FAILED="rejected"

}
internal object KycType{

    const val INTERGIRO="intergiro"
    const val BINANCE="binance"
}
internal object AppState{

    const val OPENED_HOME_PAGE_KEY="openedHomePageKey"
}

internal object DbKey{
    const val SAVE_USER="saveUser"
    const val SAVE_CRYPTO_ASSETS="saveCryptoAssets"
    const val SAVE_PAY_OR_REQUEST="savePayOrRequest"
}


internal object PixelRatio{

    const val BASE_XDPI = 395

}





internal object AccountStatus{

    const val COMPLIANCE = "COMPLIANCE"

    const val REGULAR = "REGULAR"

    fun isRegularAppMode() =  getUser()?.appMode == REGULAR

    fun isComplianceAppMode() =  getUser()?.appMode == COMPLIANCE

}




