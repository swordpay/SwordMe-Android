package org.telegram.sword.domain.common

import android.os.Parcelable
import com.google.gson.annotations.Expose
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import java.io.Serializable
@Parcelize
open class BaseResponse: Parcelable, Serializable {
    @field:SerializedName("statusName")
    val statusName: String? = null

    @field:SerializedName("statusCode")
    val statusCode: Int = 0

    @field:SerializedName("errors")
    @Expose
    val errors: List<ErrorsItem>? = null

    @field:SerializedName("message")
    @Expose
    val message: String? = null


    val isSuccess: Boolean
        get() = statusCode in 200..299

    val isAccepted: Boolean
        get() = statusName.equals("Accepted")



}

abstract class BaseListResponse : BaseResponse() {
    @field:SerializedName("_meta")
    @field:Expose
    val meta: Meta? = null
}

@Parcelize
data class Meta(
    @field:SerializedName("limit")
    val limit: Int = 0,

    @field:SerializedName("total")
    val total: Int = 0,

    @field:SerializedName("offset")
    val offset: Int = 0,

    @field:SerializedName("currentPage")
    val currentPage: Int = 0,

    @field:SerializedName("hasPrev")
    val hasPrev: Boolean = false,

    @field:SerializedName("hasNext")
    val hasNext: Boolean = false,

    @field:SerializedName("pageCount")
    val pageCount: Int = 0
) : Parcelable, Serializable

