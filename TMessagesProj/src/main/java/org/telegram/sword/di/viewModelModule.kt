package org.telegram.sword.di

import org.koin.androidx.viewmodel.dsl.viewModel
import org.koin.dsl.module
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.home.tabs.chat.viewModel.ChatViewModel
import org.telegram.sword.app.home.tabs.cryptoAccount.cryptoAccount.viewModel.BinanceAccountViewModel
import org.telegram.sword.app.home.tabs.fiatAccount.account.viewModel.FiatAccountViewModel

val viewModelModule = module {


    viewModel{ FiatAccountViewModel(useCase = get()) }

    viewModel{ BinanceAccountViewModel(useCase = get()) }

    viewModel{ ChatViewModel(useCase = get()) }

    viewModel{ BaseViewModel() }


}