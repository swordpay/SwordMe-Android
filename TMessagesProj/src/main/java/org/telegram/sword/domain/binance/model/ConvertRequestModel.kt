package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class ConvertRequestModel(


    var from: String,
    var to: String,
    var amount: Double,

) : Parcelable