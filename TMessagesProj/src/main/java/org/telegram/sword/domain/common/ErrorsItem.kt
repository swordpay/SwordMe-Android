package org.telegram.sword.domain.common

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.EMPTY

@Parcelize
data class ErrorsItem(
    @field:SerializedName("field")
    val field: String? = null,

    @field:SerializedName("message")
    val message: String = EMPTY,

    ) : Parcelable