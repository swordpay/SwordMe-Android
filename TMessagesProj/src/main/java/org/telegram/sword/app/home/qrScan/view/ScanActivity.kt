package org.telegram.sword.app.home.qrScan.view

import org.telegram.messenger.R
import org.telegram.messenger.databinding.ActivityScanBinding
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.extansion.ui.activateFirstTab
import org.telegram.sword.app.common.extansion.ui.activateLastTab
import org.telegram.sword.app.common.extansion.ui.set
import org.telegram.sword.app.common.extansion.ui.setTintVector
import org.telegram.sword.app.home.qrScan.adapter.ScanViewPagerAdapter

enum class QrScanSegmentTab{
    SCAN,MY_QR
}



class ScanActivity : BaseActivity<ActivityScanBinding, BaseViewModel>() {


    private lateinit var scanPagerAdapter: ScanViewPagerAdapter

    override fun getViewBinding() = ActivityScanBinding .inflate(layoutInflater)

    override fun onActivityCreated() {

        createViewPager()


    }

    override fun configUi() {

        binding.qrScanSegmentTap.set(
            iconVisibility = false,
            firstTabName = getString(R.string.scanCode),
            lastTabName = getString(R.string.swordMe) )

    }

    override fun clickListener(bind: ActivityScanBinding) {


        bind.qrScanSegmentTap.firstSegmentTab.setOnClickListener{

            changePager(tab = QrScanSegmentTab.SCAN)

            bind.dismissButton.setTintVector(color = R.color.dark_gray)

            bind.dismissButton.setBackgroundResource(R.drawable.black_oval)
        }

        bind.qrScanSegmentTap.lastSegmentTab .setOnClickListener{

            changePager(tab = QrScanSegmentTab.MY_QR)

            bind.dismissButton.setTintVector(color = R.color.appBlue)

            bind.dismissButton.setBackgroundResource(R.color.transparent)
        }

        bind.dismissButton.setOnClickListener{

            onBackPressed()
        }

    }

    private fun createViewPager(){

        scanPagerAdapter = ScanViewPagerAdapter(supportFragmentManager)

        binding.scannerPager.run {

            adapter = scanPagerAdapter

            offscreenPageLimit = 2

        }

    }

    private fun changePager(tab:QrScanSegmentTab){

        when(tab){

            QrScanSegmentTab.SCAN -> {

                binding.scannerPager.setCurrentItem(0, true)

                binding.qrScanSegmentTap.activateFirstTab(context = this)

            }

            QrScanSegmentTab.MY_QR-> {

                binding.scannerPager.setCurrentItem(1, true)

                binding.qrScanSegmentTap.activateLastTab(context = this)
            }

        }

    }


}