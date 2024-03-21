@file:Suppress("DEPRECATION")

package org.telegram.customqrgenerator.dsl

import android.graphics.drawable.Drawable
import org.telegram.customqrgenerator.QrOptions
import org.telegram.customqrgenerator.style.*

/**
 * @see QrBackground
 * */
sealed interface QrBackgroundBuilderScope : IQRBackground {

    override var drawable: Drawable?

    override var alpha : Float

    override var scale: BitmapScale

    override var color : QrColor
}

class InternalQrBackgroundBuilderScope internal constructor(
    val builder: QrOptions.Builder
) : QrBackgroundBuilderScope{

    override var drawable: Drawable?
        get() = builder.background.drawable
        set(value) = with(builder) {
            background(background.copy(drawable = value))
        }

    override var alpha: Float
        get() = builder.background.alpha
        set(value) = with(builder) {
            background(background.copy(alpha = value))
        }

    override var scale: BitmapScale
        get() = builder.background.scale
        set(value) = with(builder) {
            background(background.copy(scale = value))
        }

    override var color: QrColor
        get() = builder.background.color
        set(value) = with(builder) {
            background(background.copy(color = value))
        }
}