package org.telegram.customqrgenerator.vector.style
import android.graphics.Path
import org.telegram.customqrgenerator.style.Neighbors

fun interface QrVectorShapeModifier {

    fun createPath(size : Float, neighbors: Neighbors) : Path
}