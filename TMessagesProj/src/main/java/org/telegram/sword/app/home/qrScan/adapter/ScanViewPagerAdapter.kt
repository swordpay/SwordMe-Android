package org.telegram.sword.app.home.qrScan.adapter

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentPagerAdapter
import org.telegram.sword.app.home.qrScan.tab.myCode.view.MyQr
import org.telegram.sword.app.home.qrScan.tab.qrScanner.view.QrScanner

class ScanViewPagerAdapter(fm: FragmentManager): FragmentPagerAdapter(fm, BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {

    override fun getItem(position: Int): Fragment {

        return when (position) {

            0 ->  QrScanner()
            1 ->  MyQr()

            else->null
        }!!

    }
    override fun getCount() = 2

    override fun getItemPosition(`object`: Any): Int {
        return POSITION_NONE
    }
}