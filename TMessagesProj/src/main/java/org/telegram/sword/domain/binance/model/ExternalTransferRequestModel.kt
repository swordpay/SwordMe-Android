package org.telegram.sword.domain.binance.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.EMPTY
@Parcelize
data class ExternalTransferRequestModel(

   var coin:String,
   var address:String,
   var amount:Double,
   var addressName:String? = null,
   var memo:String? = null,
   var network:String = EMPTY

) : Parcelable