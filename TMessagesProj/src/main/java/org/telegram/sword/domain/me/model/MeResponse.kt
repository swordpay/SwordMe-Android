package org.telegram.sword.domain.me.model

import android.os.Parcelable
import com.google.gson.annotations.SerializedName
import kotlinx.parcelize.Parcelize
import org.telegram.sword.domain.common.BaseResponse
import java.io.Serializable

@Parcelize
data class MeResponse(

    @SerializedName("data") val data : MeData,



): BaseResponse() , Parcelable, Serializable

@Parcelize
data class MeData(

    @SerializedName("appMode") var appMode : String,//COMPLIANCE  or  REGULAR
    @SerializedName("user") val user : User

) : Parcelable, Serializable

@Parcelize
data class User(
    @SerializedName("id") val id : String?=null,
    @SerializedName("firstName") val firstName : String?=null,
    @SerializedName("lastName") val lastName : String?=null,
    @SerializedName("phone") val phone : String,
    @SerializedName("email") val email : String?=null,
    @SerializedName("username") val username : String?=null,
    @SerializedName("isEmailVerified") var isEmailVerified : Boolean,
    @SerializedName("isPhoneVerified") val isPhoneVerified : Boolean,
    @SerializedName("type") val type : String,
    @SerializedName("birthday") val birthday : String?=null,
    @SerializedName("avatar") var avatar : String? =null,
    @SerializedName("country") val country : Country?=null,
    @SerializedName("status") val status : String,
    @SerializedName("createdAt") val createdAt : String,
    @SerializedName("updatedAt") val updatedAt : String,
    @SerializedName("hasCryptoAccount") val hasCryptoAccount : Boolean,
    @SerializedName("hasFiatAccount") val hasFiatAccount : Boolean,
) : Parcelable, Serializable

@Parcelize
data class Country (

    @SerializedName("id") val id : Int,
    @SerializedName("createdAt") val createdAt : String,
    @SerializedName("updatedAt") val updatedAt : String,
    @SerializedName("name") val name : String,
    @SerializedName("alpha2") val alpha2 : String,
    @SerializedName("alpha3") val alpha3 : String,
    @SerializedName("continent") val continent : String,
    @SerializedName("phoneCode") val phoneCode : String,
    @SerializedName("currency") val currency : String
) : Parcelable, Serializable
