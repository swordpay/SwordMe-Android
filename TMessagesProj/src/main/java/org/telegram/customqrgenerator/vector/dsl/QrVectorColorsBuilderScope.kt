package org.telegram.customqrgenerator.vector.dsl
import org.telegram.customqrgenerator.vector.style.IQrVectorColors
import org.telegram.customqrgenerator.vector.style.QrVectorColor


sealed interface QrVectorColorsBuilderScope : IQrVectorColors {
    override var ball: QrVectorColor
    override var dark: QrVectorColor
    override var frame: QrVectorColor
    override var light: QrVectorColor
}

