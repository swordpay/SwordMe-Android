package org.telegram.sword.app.common.extansion.ui

import android.content.Context
import org.telegram.messenger.R
import org.telegram.messenger.databinding.SegmentTabBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.colors.AppColors.DARK_TEXT_COLOR
import org.telegram.sword.app.common.colors.AppColors.INACTIVE_SEGMENT_TEXT
import org.telegram.sword.app.common.extansion.gone
import org.telegram.sword.app.common.extansion.setHintColor
import org.telegram.sword.app.common.extansion.show

fun SegmentTabBinding.set(iconVisibility:Boolean = true, firstIcon:Int = 0, lastIcon:Int = 0, firstTabName:String = EMPTY, lastTabName:String = EMPTY){

    if (iconVisibility){

        this.firstSegmentIcon.show()
        this.lastSegmentIcon.show()
        this.firstSegmentIcon.setImageResource(firstIcon)
        this.lastSegmentIcon.setImageResource(lastIcon)

    }else{

        this.firstSegmentIcon.gone()
        this.lastSegmentIcon.gone()
    }


    this.firstSegmentName.text = firstTabName
    this.lastSegmentName.text = lastTabName

}


fun SegmentTabBinding.activateFirstTab(context: Context,){


    this.lastSegmentName.setTextColor(INACTIVE_SEGMENT_TEXT)

    this.firstSegmentName.setTextColor(DARK_TEXT_COLOR)

    this.select.animate().x(0f).duration = 120

    this.firstSegmentIcon.setHintColor(context, R.color.appBlue)

    this.lastSegmentIcon.setHintColor(context, R.color.inactiveSegmentTextColor)

//
//    this.lastSegmentName.typeface = Typeface.DEFAULT
}



fun SegmentTabBinding.activateLastTab(context: Context){


    this.firstSegmentName.setTextColor(INACTIVE_SEGMENT_TEXT)

    this.lastSegmentName.setTextColor(DARK_TEXT_COLOR)

    val size = this.lastSegmentTab.width

    this.select.animate().x(size.toFloat()).duration = 120

    this.lastSegmentIcon.setHintColor(context, R.color.appBlue)

    this.firstSegmentIcon.setHintColor(context, R.color.inactiveSegmentTextColor)

//
//    this.firstSegmentName.typeface = Typeface.DEFAULT
}