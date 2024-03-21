package org.telegram.sword.app.common.extansion.ui

import androidx.appcompat.widget.AppCompatImageView
import androidx.core.content.ContextCompat



fun AppCompatImageView.setTintVector(color:Int) = this.setColorFilter(ContextCompat.getColor(context, color), android.graphics.PorterDuff.Mode.SRC_IN)
