package org.telegram.sword.app.bottomSheet.cryptoList.view

import android.view.View
import androidx.recyclerview.widget.LinearLayoutManager
import org.koin.androidx.viewmodel.ext.android.getViewModel
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentMoneyListBinding
import org.telegram.sword.app.common.AppConst.Key.MACH
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.adapter.CryptoListAdapter
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.domain.binance.model.CryptoModel


class CryptoList(private val cryptoList:ArrayList<CryptoModel>, private val selectedCoin:(selectedCoin: CryptoModel) -> Unit) :

    BaseBottomSheet<FragmentMoneyListBinding, BinanceAccountViewModel>(R.layout.fragment_money_list,height = MACH,isDraggable = true) {

    private lateinit var  cryptoListAdapter: CryptoListAdapter

    private  var allCryptoList:ArrayList<CryptoModel> = cryptoList

    override fun getViewBinding(view: View) =  FragmentMoneyListBinding.bind(view)

    override fun onBottomSheetCreated(view: View) {

        viewModel = getViewModel()

        searchCryptoObserve()

        createCoinList()


    }


    override fun configUi() {}

    override fun clickListener(bind: FragmentMoneyListBinding) {}

    private fun searchCryptoObserve(){

        binding.cryptoSearchView.searchText.observe(this){

            cryptoListAdapter.filter.filter(it)
        }
    }

    private fun createCoinList() {

        LinearLayoutManager(requireContext(), LinearLayoutManager.VERTICAL, false).also {

            binding.cryptoListView.layoutManager = it

            cryptoListAdapter = CryptoListAdapter( isShowPrice = false, cryptoAssets = cryptoList,

                itemClick = :: cryptoListItemClick,searchResultIsEmpty =::searchResultIsEmpty)

            binding.cryptoListView.adapter = cryptoListAdapter

        }
    }

    private fun cryptoListItemClick(coin: CryptoModel) {

        selectedCoin(coin)

        dismiss()
    }

    private fun searchResultIsEmpty(cryptoList: ArrayList<CryptoModel>){


    }



}