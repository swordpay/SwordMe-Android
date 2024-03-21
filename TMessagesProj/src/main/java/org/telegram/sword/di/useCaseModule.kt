package org.telegram.sword.di

import org.koin.android.ext.koin.androidContext
import org.koin.dsl.module
import org.telegram.sword.domain.account.useCase.AccountUseCase
import org.telegram.sword.domain.binance.useCase.BinanceUseCase
import org.telegram.sword.domain.chat.useCase.ChatUseCase


val useCaseModule = module {

    factory  {  AccountUseCase(apiHelper = get(),androidContext()) }

    factory  {  BinanceUseCase(apiHelper = get(),androidContext()) }

    factory  {  ChatUseCase(apiHelper = get(),androidContext()) }



}
