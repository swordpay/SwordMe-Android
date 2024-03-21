@file:Suppress("UNUSED")

package org.telegram.customqrgenerator
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel


/**
 * QR code technology allows you to read encoded information even if
 * part of the QR code image is damaged. It also allows to have logo
 * inside the code as a part of "damage".
 * */
enum class QrErrorCorrectionLevel(
    internal val lvl : ErrorCorrectionLevel
) {

    /**
     * Minimum possible level will be used.
     * */
    Auto(ErrorCorrectionLevel.L),

    /**
     * ~7% of QR code can be damaged (or used as logo).
     * */
    Low(ErrorCorrectionLevel.L),

    /**
     * ~15% of QR code can be damaged (or used as logo).
     * */
    Medium(ErrorCorrectionLevel.M),

    /**
     * ~25% of QR code can be damaged (or used as logo).
     * */
    MediumHigh(ErrorCorrectionLevel.Q),

    /**
     * ~30% of QR code can be damaged (or used as logo).
     * */
    High(ErrorCorrectionLevel.H)
}

internal fun QrErrorCorrectionLevel.fit(
    hasLogo: Boolean,
    logoSize : Float,
) : QrErrorCorrectionLevel  {
    return if (this == QrErrorCorrectionLevel.Auto)
        when {
            !hasLogo -> QrErrorCorrectionLevel.Low
            logoSize > .3 -> QrErrorCorrectionLevel.High
            logoSize in .2 .. .3 && lvl < ErrorCorrectionLevel.Q ->
                QrErrorCorrectionLevel.MediumHigh
            logoSize > .05f && lvl < ErrorCorrectionLevel.M ->
                QrErrorCorrectionLevel.Medium
            else -> this
        } else this
}