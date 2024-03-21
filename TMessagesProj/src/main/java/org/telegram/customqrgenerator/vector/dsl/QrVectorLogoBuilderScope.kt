package org.telegram.customqrgenerator.vector.dsl
import android.graphics.drawable.Drawable
import org.telegram.customqrgenerator.style.BitmapScale
import org.telegram.customqrgenerator.vector.style.IQRVectorLogo
import org.telegram.customqrgenerator.vector.style.QrVectorColor
import org.telegram.customqrgenerator.vector.style.QrVectorLogoPadding
import org.telegram.customqrgenerator.vector.style.QrVectorLogoShape

sealed interface QrVectorLogoBuilderScope : IQRVectorLogo {

    override var drawable: Drawable?
    override var size : Float
    override var padding : QrVectorLogoPadding
    override var shape: QrVectorLogoShape
    override var scale: BitmapScale
    override var backgroundColor : QrVectorColor
}

