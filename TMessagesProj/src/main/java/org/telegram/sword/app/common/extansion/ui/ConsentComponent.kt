package org.telegram.sword.app.common.extansion.ui

import org.telegram.messenger.databinding.ConsentPageBinding


fun ConsentPageBinding.set(title:String, buttonName:String){

    this.buttonName.text = buttonName

    this.consentTitle.text = title

}