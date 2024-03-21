package org.telegram.sword.app.common.extansion.ui

import org.telegram.messenger.databinding.DataPickerItemViewBinding
import org.telegram.messenger.databinding.NameAndRightArrowItemBinding

fun NameAndRightArrowItemBinding.set(icon:Int? = null, name:String){


    this.itemName.text = name

}

fun DataPickerItemViewBinding.set(name:String,isClickable:Boolean){


    this.itemName.text = name

}

fun DataPickerItemViewBinding.setClick(isClickable:Boolean){



}