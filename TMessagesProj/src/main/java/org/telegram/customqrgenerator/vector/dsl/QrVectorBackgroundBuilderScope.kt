package org.telegram.customqrgenerator.vector.dsl
import android.graphics.drawable.Drawable
import org.telegram.customqrgenerator.style.BitmapScale
import org.telegram.customqrgenerator.vector.style.IQrVectorBackground
import org.telegram.customqrgenerator.vector.style.QrVectorColor

sealed interface QrVectorBackgroundBuilderScope : IQrVectorBackground {

    override var drawable: Drawable?
    override var scale: BitmapScale
    override var color: QrVectorColor
}