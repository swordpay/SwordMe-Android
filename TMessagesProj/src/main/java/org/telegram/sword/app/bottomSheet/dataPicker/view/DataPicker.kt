package org.telegram.sword.app.bottomSheet.dataPicker.view

import android.view.View
import androidx.recyclerview.widget.LinearLayoutManager
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentDataPickerBinding
import org.telegram.sword.app.bottomSheet.dataPicker.view.adapter.DataPickerAdapter
import org.telegram.sword.app.bottomSheet.dataPicker.view.adapter.DataPickerModel
import org.telegram.sword.app.common.AppConst.Key
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.common.base.BaseViewModel


class DataPicker( dataArray: ArrayList<DataPickerModel>,private val dataCallBack: (position:Int)-> Unit) :
    BaseBottomSheet<FragmentDataPickerBinding, BaseViewModel>

    (R.layout.fragment_data_picker,height = Key.WRAP, isDraggable = true) {

    private var dataList: ArrayList<DataPickerModel> = dataArray

    lateinit var listAdapter: DataPickerAdapter

    override fun onBottomSheetCreated(view: View) {

        setupList()
    }

    override fun getViewBinding(view: View)= FragmentDataPickerBinding .bind(view)

    override fun configUi() {}

    override fun clickListener(bind: FragmentDataPickerBinding) {

        bind.topBar.dismissButton.setOnClickListener{ dismiss() }

    }


    private fun setupList(){

        LinearLayoutManager(requireContext(), LinearLayoutManager.VERTICAL, false).also {

            binding.dataPicketListView.layoutManager = it

            listAdapter = DataPickerAdapter( dataArray =  dataList, itemClick = ::listItemClick)

            binding.dataPicketListView.adapter = listAdapter

        }
    }



    private fun listItemClick(position: Int) {

        dataCallBack(position)

        dismiss()
    }


}