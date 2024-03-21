package org.telegram.sword.swordTeleModels.local

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import java.io.Serializable

@Parcelize
data class AcceptInfo(

    @SerializedName("views") val views : String,
    @SerializedName("time") val time : String,
    @SerializedName("fileType") val fileType : String,
    @SerializedName("fileExtansion") val fileExtansion : String,
    @SerializedName("id") val id : String,
    @SerializedName("isOutOwner") val isOutOwner : Boolean

    ): Serializable, Parcelable
