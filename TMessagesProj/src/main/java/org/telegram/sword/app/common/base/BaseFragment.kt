package org.telegram.sword.app.common.base

import android.app.Dialog
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.view.View
import android.view.Window
import androidx.annotation.LayoutRes
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModel
import androidx.viewbinding.ViewBinding
//import kotlinx.android.synthetic.main.dialog_message.*
//import kotlinx.android.synthetic.main.dialog_message.view.*
import org.telegram.messenger.R
import org.telegram.sword.app.common.extansion.loading

abstract class BaseFragment<BindingType : ViewBinding,ViewModelType : ViewModel>(@LayoutRes id: Int) : Fragment(id) {

    val loading: Dialog by lazy { requireActivity().loading() }
    private var isShowMessageDialog = false

    var nexPage = 0

    var nexPage_2 = 0

    var hasNext = false

    var hasNext_2 = false

    protected lateinit var binding: BindingType

    protected lateinit var viewModel: ViewModelType

    abstract fun getViewBinding(view: View): BindingType

    abstract fun onFragmentCreated(view: View)

    abstract fun configUi()

    abstract fun clickListener(bind:BindingType)

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding = getViewBinding(view)
        onFragmentCreated(view)
        clickListener(binding)
        configUi()

    }

    fun showMessageDialog(
        title: String,
        message: String,
        status: DialogStatus = DialogStatus.ERROR,
        function: (() -> Unit)? = null
    ) {

        val dialog = Dialog(requireContext())
        requireActivity().runOnUiThread {
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
            dialog.setCancelable(false)
            dialog.setContentView(R.layout.dialog_message)

            try {
                if (!isShowMessageDialog) {
                    dialog.show()
                    isShowMessageDialog = true
                }
            }catch (e:Exception){ }

        }
    }
}