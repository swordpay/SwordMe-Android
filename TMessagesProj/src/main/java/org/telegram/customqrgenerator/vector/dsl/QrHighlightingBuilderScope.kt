package org.telegram.customqrgenerator.vector.dsl
import org.telegram.customqrgenerator.HighlightingType
import org.telegram.customqrgenerator.IAnchorsHighlighting

sealed interface QrHighlightingBuilderScope : IAnchorsHighlighting {
    override var cornerEyes: HighlightingType
    override var versionEyes: HighlightingType
    override var timingLines: HighlightingType
    override val alpha: Float
}