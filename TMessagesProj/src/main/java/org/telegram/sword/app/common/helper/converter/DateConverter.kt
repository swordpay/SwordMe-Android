@file:Suppress("IMPLICIT_CAST_TO_ANY")

package org.telegram.sword.app.common.helper.converter


import android.os.Build
import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import org.telegram.sword.app.common.AppConst.EMPTY
import java.text.SimpleDateFormat
import java.time.ZoneId
import java.util.*

fun convertDate(date:String): DateModel{

    val mont:HashMap<String,String> = hashMapOf(

        "01" to "Jan",
        "02" to "Feb",
        "03" to "Mar",
        "04" to "Apr",
        "05" to "May",
        "06" to "June",
        "07" to "July",
        "08" to "Aug",
        "09" to "Sept",
        "10" to "Oct",
        "11" to "Nov",
        "12" to "Dec",

    )

    return try {
        val utcFormatter = SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'", Locale.US)
        utcFormatter.timeZone = TimeZone.getTimeZone("UTC")
        val utcDate = utcFormatter.parse(date)
        val zoneId =   if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {

            ZoneId.systemDefault().id
        }else{
            TimeZone.getTimeZone("America/New_York").id
        }
        val calendar = Calendar.getInstance()
        val year = calendar[Calendar.YEAR]



        val dateFormatter = SimpleDateFormat("dd MM yyyy", Locale.US)
        dateFormatter.timeZone = TimeZone.getTimeZone(zoneId)

        val timeFormatter = SimpleDateFormat("HH:mm", Locale.US)
        timeFormatter.timeZone = TimeZone.getTimeZone(zoneId)

        val monthFormatter = SimpleDateFormat("MM", Locale.US)
        monthFormatter.timeZone = TimeZone.getTimeZone(zoneId)
        val month = mont[monthFormatter.format(utcDate)]


        var d = dateFormatter.format(utcDate)

        if (d.endsWith(""+year)){

            d =  d.replace(""+year,"")

        }



        DateModel( date = d.replace(" ${monthFormatter.format(utcDate)} "," $month "),
            time = timeFormatter.format(utcDate))
    }catch (e:Exception){
        DateModel()
    }

}
@Parcelize
data class DateModel(

    var date:String = EMPTY,
    var time:String = EMPTY

) : Parcelable






