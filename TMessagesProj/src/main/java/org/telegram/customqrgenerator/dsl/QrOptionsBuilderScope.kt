@file:Suppress("DEPRECATION")

package org.telegram.customqrgenerator.dsl
import org.telegram.customqrgenerator.QrErrorCorrectionLevel
import org.telegram.customqrgenerator.QrOptions
import org.telegram.customqrgenerator.createQrOptions
import org.telegram.customqrgenerator.style.*

/**
 * Used to create [QrOptions] with DSL
 *
 * @see createQrOptions
 * */
sealed interface QrOptionsBuilderScope {

    var shape : QrShape

    /**
     * Padding of the QR code relative to [width] and [height].
     * */
    val padding : Float

    val width : Int

    val height : Int

    var errorCorrectionLevel : QrErrorCorrectionLevel

    /**
     * Set QR code offset
     *
     * @see QrOffset
     * */
    fun offset(block : QrOffsetBuilderScope.() -> Unit)

    /**
     * Customize QR code logo
     *
     * @see QrLogo
     * */
    fun logo(block : QrLogoBuilderScope.() -> Unit)

    /**
     * Customize QR code background
     *
     * @see QrBackground
     * */
    fun background(block: QrBackgroundBuilderScope.() -> Unit)

    /**
     * Customize QR code colors
     *
     * @see QrColors
     * */
    fun colors(block : QrColorsBuilderScope.() -> Unit)

    /**
     * Customize QR code shapes
     *
     * @see QrElementsShapes
     * */
    fun shapes(block : QrElementsShapesBuilderScope.() -> Unit)
}



fun QrOptionsBuilderScope(builder: QrOptions.Builder) : QrOptionsBuilderScope =
    InternalQrOptionsBuilderScope(builder)



private class InternalQrOptionsBuilderScope(
    private val builder: QrOptions.Builder
) : QrOptionsBuilderScope {

    override fun offset(block: QrOffsetBuilderScope.() -> Unit) {
        InternalQrOffsetBuilderScope(builder).apply(block)
    }

    override fun logo(block: QrLogoBuilderScope.() -> Unit) {
        InternalQrLogoBuilderScope(
            builder,
            width = builder.width,
            height = builder.height
        ).apply(block)
    }

    override fun background(block: QrBackgroundBuilderScope.() -> Unit) {
        InternalQrBackgroundBuilderScope(builder).apply(block)
    }

    override fun colors(block: QrColorsBuilderScope.() -> Unit) {
        InternalColorsBuilderScope(builder).apply(block)
    }

    override fun shapes(block: QrElementsShapesBuilderScope.() -> Unit) {
        InternalQrElementsShapesBuilderScope(builder).apply(block)
    }

    override var shape: QrShape
        get() = builder.codeShape
        set(value) {
            builder.codeShape(value)
        }

    override val padding: Float
        get() = builder.padding

    override val width: Int by builder::width

    override val height: Int by builder::height

    override var errorCorrectionLevel: QrErrorCorrectionLevel
        get() = builder.errorCorrectionLevel
        set(value) {
            builder.errorCorrectionLevel(value)
        }
}