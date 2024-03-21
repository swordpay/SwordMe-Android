package org.telegram.sword.app.common.base

import android.app.Dialog
import android.graphics.Color
import android.graphics.Rect
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.view.*
import android.widget.TextView
import androidx.appcompat.widget.AppCompatTextView
import androidx.core.content.res.ResourcesCompat
import androidx.lifecycle.ViewModel
import androidx.viewbinding.ViewBinding
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.makeramen.roundedimageview.RoundedImageView
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key.MACH
import org.telegram.sword.app.common.AppConst.Key.WRAP
import org.telegram.sword.app.common.extansion.gone
import org.telegram.sword.app.common.extansion.loading
import org.telegram.sword.app.common.extansion.show

enum class BottomSheetStyle{
    WHITE,GRAY
}

abstract class BaseBottomSheet<BindingType : ViewBinding,ViewModelType : ViewModel>
               (private val  resId: Int,
                private  val height :String = MACH,
                private val isDraggable:Boolean = false,
                private val style: BottomSheetStyle = BottomSheetStyle.WHITE,
                private val isFinishParentActivity:Boolean = false,
                private val customOnBackPress:Boolean = false

) : BottomSheetDialogFragment() {


    val loading: Dialog by lazy { loading() }

    var nexPage = 0

    var hasNext = true

    private var isShowMessageDialog = false

    protected lateinit var binding: BindingType

    protected lateinit var viewModel: ViewModelType

    private lateinit var dialog: BottomSheetDialog

    abstract fun getViewBinding(view: View): BindingType

    abstract fun onBottomSheetCreated(view: View)

    abstract fun configUi()

    abstract fun clickListener(bind:BindingType)


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(resId,container,false)
        binding = getViewBinding(view)
        onBottomSheetCreated(view)
        clickListener(binding)

        configUi()

        return view
    }
    override fun onStart() {
        super.onStart()
        val displayRectangle = Rect()
        val window: Window = activity!!.window
        window.decorView.getWindowVisibleDisplayFrame(displayRectangle)

        dialog.also {
            val parentLayout = dialog.findViewById<View>(R.id.design_bottom_sheet)


        parentLayout?.also { layout ->

            val behaviour = BottomSheetBehavior.from(layout)
            setupFullHeight(layout,height)
            behaviour.state = BottomSheetBehavior.STATE_EXPANDED
            behaviour.isDraggable = isDraggable


        }
            view?.requestLayout()
        }
    }


    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        setupBottomSheetStyle()
        val style = when(style){
            BottomSheetStyle.GRAY -> R.style.GrayBottomSheetDialogTheme
            else -> R.style.WhiteBottomSheetDialogTheme
        }

        dialog =   if (customOnBackPress){

            BottomSheetDialog(requireContext(), style)

        } else {
            object : BottomSheetDialog(requireContext(), style) {


                override fun onBackPressed() {

                    if (isFinishParentActivity) {

                        requireActivity().finish()

                    }

                    if (isDraggable) {

                        super.onBackPressed()
                    }


                }
            }
        }

       return dialog
    }

    private fun setupBottomSheetStyle(){

        dialog = BottomSheetDialog(
            requireContext(),
            R.style.WhiteBottomSheetDialogTheme
        )
    }


    private fun setupFullHeight(bottomSheet: View, size:String) {
        val layoutParams = bottomSheet.layoutParams
        when(size){
            WRAP-> layoutParams.height = WindowManager.LayoutParams.WRAP_CONTENT
            else-> layoutParams.height = WindowManager.LayoutParams.MATCH_PARENT
        }

        bottomSheet.layoutParams = layoutParams
    }

    fun showMessageDialog(
        title: String = EMPTY,
        message: String,
        status: DialogStatus = DialogStatus.ERROR,
        function: (() -> Unit)? = null,
        isShowDismissBtn:Boolean = false,

        ) {

        val dialog = Dialog(requireActivity())
        requireActivity().runOnUiThread {
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
            dialog.setCancelable(false)
            dialog.setContentView(R.layout.dialog_message)
            val dialogTitle: TextView = dialog.findViewById(R.id.dialogTitle)
            val dialogDesc: TextView = dialog.findViewById(R.id.dialogDesc)
            val dialogStatusIcon: RoundedImageView = dialog.findViewById(R.id.messageDialogStatusIcon)
            val dialogBtn: AppCompatTextView = dialog.findViewById(R.id.dialogBtn)
            val dismissBtn: AppCompatTextView = dialog.findViewById(R.id.dismissBtn)
            dialogTitle.show(title.isNotEmpty())
            dialogTitle.text = title
            dialogDesc.text = message.ifEmpty { getLocalizeMessage("") }
            when (status) {
                DialogStatus.SUCCESS -> dialogStatusIcon.setImageResource(R.drawable.success_dialog_icon)
                DialogStatus.ERROR -> dialogStatusIcon.setImageResource(R.drawable.error_dialog_icon)
                DialogStatus.INFO -> dialogStatusIcon.setImageResource(R.drawable.wrong_dialog_icon)
                DialogStatus.FORCE_UPDATE -> dialogStatusIcon.setImageResource(R.drawable.ic_play_market_icon)
            }

            dismissBtn.show(isShowDismissBtn)

            dialogBtn.setOnClickListener {
                isShowMessageDialog = false
                dialog.dismiss()
                function?.invoke()
            }
            try {
                if (!isShowMessageDialog) {
                    dialog.show()
                    isShowMessageDialog = true
                }
            }catch (_:Exception){ }

        }
    }

    open fun showInfoDialog(title:String? = null,message:String?=null,buttonName:String = "Ok",function: (() -> Unit)? = null) {

        val dial = Dialog(requireActivity())

        dial.requestWindowFeature(Window.FEATURE_NO_TITLE)

        dial.window!!.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))

        dial.setCancelable(false)

        dial.setContentView(R.layout.error_dialog_view)

        val btn = dial.findViewById<AppCompatTextView>(R.id.dialogBtn)

        val dialogTitle = dial.findViewById<AppCompatTextView>(R.id.dialogTitle)

        val dialogDesc = dial.findViewById<AppCompatTextView>(R.id.textFieldDialogDesc)

        btn.typeface = ResourcesCompat.getFont(context!!, R.font.graphik_semi_bold)

        if (title.isNullOrEmpty()){

            dialogTitle.gone()

        }else{

            dialogTitle.show()

            dialogTitle.text = title

        }
        if (message.isNullOrEmpty()){

            dialogDesc.gone()

        }else{

            dialogDesc.show()

            dialogDesc.text = title

        }


        dialogDesc.text = message

        btn.text = buttonName

        btn.setOnClickListener { v: View? ->

            function?.invoke()

            dial.dismiss()

        }
        dial.show()
    }

}
