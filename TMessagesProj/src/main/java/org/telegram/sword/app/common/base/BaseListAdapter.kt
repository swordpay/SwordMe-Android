package org.telegram.sword.app.common.base

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import org.telegram.messenger.databinding.FooterBinding
import org.telegram.sword.app.common.AppConst.ForAdapter

open class BaseListAdapter(private val itemCount:Int,private val footerEnable:Boolean = true):
    RecyclerView.Adapter<RecyclerView.ViewHolder>(){


    class FooterViewHolder(binding: FooterBinding) : RecyclerView.ViewHolder(binding.root)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        return when (viewType) {

            ForAdapter.FOOTER -> FooterViewHolder(
                FooterBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )

            else -> Unit as RecyclerView.ViewHolder
        }

    }


    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {}

    override fun getItemCount(): Int = if (footerEnable) itemCount + 1 else itemCount

}