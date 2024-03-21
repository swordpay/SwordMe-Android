package org.telegram.customqrgenerator.vector.dsl
import org.telegram.customqrgenerator.vector.style.IQrVectorShapes
import org.telegram.customqrgenerator.vector.style.QrVectorBallShape
import org.telegram.customqrgenerator.vector.style.QrVectorFrameShape
import org.telegram.customqrgenerator.vector.style.QrVectorPixelShape

sealed interface QrVectorShapesBuilderScope : IQrVectorShapes {
    override var darkPixel: QrVectorPixelShape
    override var lightPixel: QrVectorPixelShape
    override var ball: QrVectorBallShape
    override var frame: QrVectorFrameShape
}

