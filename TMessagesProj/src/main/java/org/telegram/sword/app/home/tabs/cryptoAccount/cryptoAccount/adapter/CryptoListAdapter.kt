package org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.adapter

import android.annotation.SuppressLint
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.Filter
import android.widget.Filterable
import androidx.recyclerview.widget.RecyclerView
import org.telegram.messenger.R
import org.telegram.messenger.databinding.BalanceListItemBinding
import org.telegram.messenger.databinding.FooterBinding
import org.telegram.messenger.databinding.NotResultPageBinding
import org.telegram.messenger.databinding.TextHeaderItemBinding
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.ForAdapter.EMPTY_LIST
import org.telegram.sword.app.common.AppConst.ForAdapter.FOOTER
import org.telegram.sword.app.common.AppConst.ForAdapter.HEADER
import org.telegram.sword.app.common.AppConst.ForAdapter.LIST_ITEM
import org.telegram.sword.app.common.AppConst.PRECISION_8
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.extansion.*
import org.telegram.sword.app.common.helper.field.doubleBigDecimal
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.cryptoList
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoin
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEur
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.cryptoData.mainCoinToEurPercent
import org.telegram.sword.app.home.tabs.cryptoAccount.helper.correctBalanceInEuro
import org.telegram.sword.domain.binance.model.CryptoModel
import org.telegram.sword.socket.webSocket.const_.CryptoSymbol
import java.util.*

class CryptoListAdapter (private val cryptoAssets: ArrayList<CryptoModel>,
                         private val isShowPrice: Boolean,
                         private val itemClick :(coin: CryptoModel) -> Unit,
                         private val searchResultIsEmpty :((cryptoList: ArrayList<CryptoModel>) -> Unit)? = null) :
    RecyclerView.Adapter<RecyclerView.ViewHolder>(),Filterable {

    var cryptoFilterList = ArrayList<CryptoModel>()

    var searchText = ""

    init {

        cryptoFilterList = cryptoAssets

    }


     @SuppressLint("NotifyDataSetChanged")
     fun updateAssets(search:String){

         try {
             searchText = search

             if (searchText.trim().isNotEmpty()){

                 val filter = ArrayList<CryptoModel>()

                 cryptoFilterList.forEach { it_1 ->

                     cryptoList.forEach { it_2 ->

                         if (it_1.coin == it_2.coin){

                             filter.add(it_2)
                         }
                     }
                 }

                 cryptoFilterList = filter

             }else{

                 cryptoFilterList = cryptoList
             }

             notifyDataSetChanged()

         }catch (e:Exception){

         }

    }


    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        return when (viewType) {

            LIST_ITEM -> CryptoViewHolder(
                BalanceListItemBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )

            HEADER -> HeaderViewHolder(
                TextHeaderItemBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )



            EMPTY_LIST -> EmptyListViewHolder(
                NotResultPageBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )

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
            is CryptoViewHolder -> {
                try {
                    holder.bind(data = cryptoFilterList[position],isShowPrice = isShowPrice)


                    holder.itemView.setOnClickListener{

                        itemClick(cryptoFilterList[position])
                    }
                }catch (_:Exception){}

            }

            is HeaderViewHolder ->{
                try {
                    holder.bind(sectionName = cryptoFilterList[position].sectionHeader)
                }catch (_:Exception){ }

            }


        }
    }

    override fun getItemCount() = cryptoFilterList.size + 1

    override fun getItemViewType(position: Int) =

        try {
            if (cryptoFilterList.isEmpty()){

                EMPTY_LIST

            }else {
                if (position == cryptoFilterList.size) {

                    FOOTER

                } else if (cryptoFilterList[position].sectionHeader.isNotEmpty()) {

                    HEADER

                } else {

                    LIST_ITEM
                }
            }
        }catch (e:Exception){

            LIST_ITEM

        }



    class CryptoViewHolder(private val binding: BalanceListItemBinding) :
        RecyclerView.ViewHolder(binding.root) {

        @SuppressLint("SetTextI18n")
        fun bind(data: CryptoModel, isShowPrice:Boolean) {
            try {

                binding.also {

                    it.name.text = EMPTY

                    it.shortName.text = EMPTY

                    it.coinBalanceInEuro.text = EMPTY

                    it.changePercent.text = EMPTY

                    it.coinIcon.setImageResource(R.drawable.crypto_default_icon)

                    it.name.text = data.name.ifEmpty { data.coin }

                    if (data.balance !=null){

                        it.shortName.text = numberFormatString(doubleBigDecimal(data.balance.toString()), PRECISION_8)

                    }else{

                        it.shortName.text = data.coin
                    }


                    if (isShowPrice){

                        if( data.coin.uppercase() == mainCoin.uppercase()){

                            var amount = correctBalanceInEuro(mainCoinToEur.toString(),isMaiCoinSelected = true)

                            amount = numberFormatString(amount.formatStringToDouble().roundingTo2ScaleOrFull().toCorrectString(), PRECISION_8)

                            it.coinBalanceInEuro.text = "${CryptoSymbol.CURRENCY_SYMBOL} $amount"

                            if (mainCoinToEurPercent < 0){

                                it.changePercent.text = mainCoinToEurPercent.roundingDown(scale = 2).toString()

                                it.changePercent.setTextColor(  AppColors.RED )

                            }else{

                                it.changePercent.text = "+${mainCoinToEurPercent.roundingDown(scale = 2)}"

                                it.changePercent.setTextColor(  AppColors.GREEN_TEXT )

                            }
                        }else{

                            var amount = correctBalanceInEuro(data.balanceInFiat)

                            amount = numberFormatString(amount.formatStringToDouble().roundingTo2ScaleOrFull().toCorrectString(), PRECISION_8)

                            it.coinBalanceInEuro.text = "${CryptoSymbol.CURRENCY_SYMBOL} $amount"



                            if (data.percent < 0){

                                it.changePercent.text = data.percent.roundingDown(scale = 2).toString()

                                it.changePercent.setTextColor(  AppColors.RED )

                            }else{

                                it.changePercent.text = "+${data.percent.roundingDown(scale = 2)}"

                                it.changePercent.setTextColor(  AppColors.GREEN_TEXT )

                            }
                        }


                    }else{

                        binding.balanceLayout.gone()

                    }



                    it.coinIcon.loadCryptoImage(coin = data.coin.uppercase())


                }
            }catch (e:Exception){



            }


        }

        @SuppressLint("SetTextI18n")
        fun updatePriceAndPercent(balanceInFiat:String, percent:Double,coin:String){
            try {
                if (coin.uppercase() == mainCoin.uppercase()){

                    var amount = correctBalanceInEuro(mainCoinToEur.toString(),isMaiCoinSelected = true)

                    amount = numberFormatString(amount.formatStringToDouble().roundingTo2ScaleOrFull().toCorrectString(), PRECISION_8)

                    binding.coinBalanceInEuro.text = "${CryptoSymbol.CURRENCY_SYMBOL} $amount"

                }else{

                    var amount = correctBalanceInEuro(balanceInFiat)

                    amount = numberFormatString(amount.formatStringToDouble().roundingTo2ScaleOrFull().toCorrectString(), PRECISION_8)

                    binding.coinBalanceInEuro.text = "${CryptoSymbol.CURRENCY_SYMBOL} $amount"

                }



                if (percent < 0){

                    binding.changePercent.text = percent.roundingDown(scale = 2).toString()

                    binding.changePercent.setTextColor(  AppColors.RED )

                }else{

                    binding.changePercent.text = "+${percent.roundingDown(scale = 2)}"

                    binding.changePercent.setTextColor(  AppColors.GREEN_TEXT )

                }
            }catch (_:Exception){ }


        }

        @SuppressLint("SetTextI18n")
        fun updateMyCoinCount(balance:String){

            try {

                binding.shortName.text = doubleBigDecimal(balance)

            }catch (_:Exception){ }


        }

    }

    class HeaderViewHolder(private val binding: TextHeaderItemBinding) : RecyclerView.ViewHolder(binding.root) {

        fun bind(sectionName: String) {

            try {
                binding.headerText.text = sectionName
            }catch (_:Exception){ }

        }
    }

    class EmptyListViewHolder(private val binding: NotResultPageBinding) : RecyclerView.ViewHolder(binding.root){

        init {

            binding.emptyInfoText.text = "The coin you entered cannot be found"

        }


    }

    class FooterViewHolder(binding: FooterBinding) : RecyclerView.ViewHolder(binding.root)




    override fun getFilter(): Filter {
        return object : Filter() {
            override fun performFiltering(constraint: CharSequence?): FilterResults {

                try {
                    val charSearch = constraint.toString()
                    cryptoFilterList = if (charSearch.isEmpty()) {
                        cryptoAssets
                    } else {
                        val resultList = ArrayList<CryptoModel>()
                        for (row in cryptoAssets) {

                            if (row.name.lowercase(Locale.ROOT).contains(charSearch.lowercase(Locale.ROOT)) ||
                                row.coin.lowercase(Locale.ROOT).contains(charSearch.lowercase(Locale.ROOT)))
                            {
                                resultList.add(row)
                            }

                        }
                        resultList
                    }
                    val filterResults = FilterResults()
                    filterResults.values = cryptoFilterList
                    return filterResults
                }catch (e:Exception){

                    val charSearch = constraint.toString()
                    cryptoFilterList = if (charSearch.isEmpty()) {
                        cryptoAssets
                    } else {
                        val resultList = ArrayList<CryptoModel>()
                        for (row in cryptoAssets) {

                            if (row.name.lowercase(Locale.ROOT).contains(charSearch.lowercase(Locale.ROOT)) ||
                                row.coin.lowercase(Locale.ROOT).contains(charSearch.lowercase(Locale.ROOT)))
                            {
                                resultList.add(row)
                            }

                        }
                        resultList
                    }
                    val filterResults = FilterResults()
                    filterResults.values = cryptoFilterList
                    return filterResults
                }

            }

            @SuppressLint("NotifyDataSetChanged")
            override fun publishResults(constraint: CharSequence?, results: FilterResults?) {
                try {
                    cryptoFilterList = results?.values as ArrayList<CryptoModel>

                    searchResultIsEmpty?.let { it(cryptoFilterList) }

                    notifyDataSetChanged()

                }catch (_:Exception){ }

            }
        }
    }

}

