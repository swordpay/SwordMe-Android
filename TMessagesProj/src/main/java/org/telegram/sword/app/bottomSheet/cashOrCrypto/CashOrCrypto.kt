package org.telegram.sword.app.bottomSheet.cashOrCrypto

import android.view.View
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentCashOrCryptoBinding
import org.telegram.sword.app.common.AppConst.Key
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.extansion.ui.set
import org.telegram.sword.app.common.extansion.ui.topBar.set


class CashOrCrypto(val selectedType:(isFiat:Boolean)->Unit)  : BaseBottomSheet<FragmentCashOrCryptoBinding, BaseViewModel>(R.layout.fragment_cash_or_crypto,height = Key.WRAP,isDraggable = true){


    override fun getViewBinding(view: View)= FragmentCashOrCryptoBinding.bind(view)


    override fun onBottomSheetCreated(view: View) {

    }

    override fun configUi() {

        binding.topBar.set(title = getString(R.string.selectAmountType))

        binding.cash.set(name =getString(R.string.cash))

        binding.crypto.set(name =getString(R.string.crypto))

    }

    override fun clickListener(bind: FragmentCashOrCryptoBinding) {

        bind.topBar.dismissButton.setOnClickListener{ dismiss() }

        bind.cash.item.setOnClickListener{

            selectedType(true)

            dismiss()
        }

        bind.crypto.item.setOnClickListener{

            selectedType(false)

            dismiss()
        }

    }


}