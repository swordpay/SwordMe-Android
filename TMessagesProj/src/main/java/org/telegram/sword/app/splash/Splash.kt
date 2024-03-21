package org.telegram.sword.app.splash

import android.util.DisplayMetrics
import android.view.animation.AnimationUtils
import org.telegram.messenger.R
import org.telegram.messenger.databinding.ActivitySplashBinding
import org.telegram.sword.app.common.AppConst.AppState.OPENED_HOME_PAGE_KEY
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.extansion.delayOnLifecycle
import org.telegram.sword.app.common.extansion.openActivity
import org.telegram.sword.app.common.extansion.putSharedBul
import org.telegram.sword.app.common.extansion.show
import org.telegram.ui.LaunchActivity


class Splash : BaseActivity<ActivitySplashBinding, BaseViewModel>() {

    override fun getViewBinding() = ActivitySplashBinding.inflate(layoutInflater)


    override fun onActivityCreated() {

        putSharedBul(OPENED_HOME_PAGE_KEY,false)

        val displayMetrics = DisplayMetrics()
        windowManager.defaultDisplay.getMetrics(displayMetrics)

         this.windowManager.defaultDisplay.getMetrics(displayMetrics)

        window.decorView.rootView.delayOnLifecycle(600L) {

                 openActivity(activity = LaunchActivity())

                 finish()

         }
    }


    override fun configUi() {

        binding.linkLayout.show()

        binding.linkLayout.animation = AnimationUtils.loadAnimation(this, R.anim.slide_in_bottom)

        binding.swordIcon.show()

        binding.swordIcon.animation = AnimationUtils.loadAnimation(this, R.anim.slide_in_top)


    }

    override fun clickListener(bind: ActivitySplashBinding) {

    }


    @Deprecated("Deprecated in Java", ReplaceWith("finishAffinity()"))
    override fun onBackPressed() {

        finishAffinity()
    }
}

