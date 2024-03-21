package org.telegram.customqrgenerator.vector.dsl
import org.telegram.customqrgenerator.QrErrorCorrectionLevel
import org.telegram.customqrgenerator.style.QrOffset
import org.telegram.customqrgenerator.style.QrShape
import org.telegram.customqrgenerator.vector.QrVectorOptions


internal class InternalQrVectorOptionsBuilderScope(
    val builder: QrVectorOptions.Builder
) : QrVectorOptionsBuilderScope {

    override var padding: Float
        get() = builder.padding
        set(value) {
            builder.setPadding(value)
        }

    override var errorCorrectionLevel: QrErrorCorrectionLevel
        get() = builder.errorCorrectionLevel
        set(value) {
            builder.setErrorCorrectionLevel(value)
        }

    override var codeShape: QrShape
    get() = builder.shape
    set(value){
        builder.setCodeShape(value)
    }

    override var fourthEyeEnabled: Boolean
    get() = builder.fourthEyeEnabled
    set(value) {
        builder.setFourthEyeEnabled(value)
    }

    override fun offset(x : Float, y : Float) {
        builder.setOffset(QrOffset(x,y))
    }

    override fun shapes(
        centralSymmetry : Boolean,
        block: QrVectorShapesBuilderScope.() -> Unit
    ) {
        InternalQrVectorShapesBuilderScope(builder,centralSymmetry)
            .apply(block)
    }

    override fun colors(block: QrVectorColorsBuilderScope.() -> Unit) {
        InternalQrVectorColorsBuilderScope(builder).apply(block)
    }

    override fun background(block: QrVectorBackgroundBuilderScope.() -> Unit) {
        InternalQrVectorBackgroundBuilderScope(builder).apply(block)
    }

    override fun logo(block: QrVectorLogoBuilderScope.() -> Unit) {
        InternalQrVectorLogoBuilderScope(builder)
            .apply(block)
    }

    override fun highlighting(block: QrHighlightingBuilderScope.() -> Unit) {
        InternalQrHighlightingBuilderScope(builder).apply(block)
    }
}