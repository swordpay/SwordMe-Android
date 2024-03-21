package org.telegram.sword.app.bottomSheet.dataPicker.view.adapter

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import org.telegram.messenger.R
import org.telegram.messenger.databinding.DataPickerItemViewBinding
import org.telegram.messenger.databinding.FooterBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.ForAdapter
import org.telegram.sword.app.common.colors.AppColors.DARK_TEXT_COLOR
import org.telegram.sword.app.common.colors.AppColors.HINT

data class DataPickerModel(

    val name:String = EMPTY,
    var isClicked:Boolean = false,
    val num:Int?=null,
    val globalName:String = EMPTY

)

class DataPickerAdapter (private val dataArray: ArrayList<DataPickerModel>,
                         private val itemClick :(position: Int) -> Unit
) :
    RecyclerView.Adapter<RecyclerView.ViewHolder>() {

    override fun getItemCount() = dataArray.size+1

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {

        return when (viewType) {
            ForAdapter.LIST_ITEM -> DataViewModel(
                DataPickerItemViewBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent, false))



            else -> FooterViewHolder(
                FooterBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )
        }
    }

    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {

        when (holder) {

            is DataViewModel -> {

                holder.bind(data = dataArray[position])

                holder.itemView.setOnClickListener{

                    itemClick(position)
                }
            }


        }
    }


    override fun getItemViewType(position: Int) =

            if (position == dataArray.size) ForAdapter.FOOTER else ForAdapter.LIST_ITEM



    class DataViewModel(private val binding: DataPickerItemViewBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(data: DataPickerModel) {

            binding.itemName.setTextColor(if (data.isClicked) DARK_TEXT_COLOR else HINT)

            binding.item.setBackgroundResource(if (data.isClicked) R.drawable.gray_low_rouded_fon else R.drawable.click_effect)

            binding.itemName.text = data.name


        }
    }


    class FooterViewHolder(binding: FooterBinding) : RecyclerView.ViewHolder(binding.root)


}