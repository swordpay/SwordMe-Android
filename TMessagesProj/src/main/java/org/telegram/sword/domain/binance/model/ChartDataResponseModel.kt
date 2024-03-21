package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class ChartDataResponseModel(

  var  symbol:String,
  var  interval:String,
  var  startTime:Long,
  var  endTime:Long,

) : Parcelable
