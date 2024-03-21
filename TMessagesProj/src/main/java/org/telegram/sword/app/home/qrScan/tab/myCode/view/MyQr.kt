package org.telegram.sword.app.home.qrScan.tab.myCode.view

import android.annotation.SuppressLint
import android.graphics.drawable.Drawable
import android.view.View
import androidx.core.content.ContextCompat
import org.telegram.PhoneFormat.PhoneFormat
import org.telegram.customqrgenerator.QrData
import org.telegram.customqrgenerator.vector.QrCodeDrawable
import org.telegram.customqrgenerator.vector.QrVectorOptions
import org.telegram.customqrgenerator.vector.style.*
import org.telegram.messenger.R
import org.telegram.messenger.UserConfig
import org.telegram.messenger.UserObject
import org.telegram.messenger.databinding.FragmentMyQrBinding
import org.telegram.sword.app.common.base.BaseFragment
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.extansion.printQr
import org.telegram.sword.app.common.extansion.sharedQR
import org.telegram.sword.domain.common.BaseUrl.DEEPLINK_BASE_URL
import org.telegram.tgnet.TLRPC
import org.telegram.ui.Components.FloatingDebug.FloatingDebugController.onBackPressed


class MyQr : BaseFragment<FragmentMyQrBinding, BaseViewModel>(R.layout.fragment_my_qr) {

    var username: String? = null
    var userId:Long? = null

    override fun getViewBinding(view: View) = FragmentMyQrBinding.bind(view)

    override fun onFragmentCreated(view: View) {

        val currentAccount = UserConfig.selectedAccount

        val user: TLRPC.User? = UserConfig.getInstance(currentAccount).getCurrentUser()

        if (user != null) {

            username = UserObject.getPublicUsername(user)


            if (username == null) {

                username =
                    PhoneFormat.getInstance().format("+" + user.phone).toString().replace(" ", "");
            }

        }




    }

    @SuppressLint("Range")
    override fun configUi() {

        if (username.isNullOrEmpty()) {

            showMessageDialog(
                title = "Qr",
                message = "Username not found",
                function = { onBackPressed() })

        } else {

            val data = QrData.Text(DEEPLINK_BASE_URL + username)
            val options = QrVectorOptions.Builder()

            .setLogo(
                QrVectorLogo(

                    drawable = ContextCompat.getDrawable(requireContext(), R.drawable.qr_center_icon),
                    size = .33f,
                    padding = QrVectorLogoPadding.Natural(.1f),
                    shape = QrVectorLogoShape
                        .Circle
                )
            )
                .setColors(
                    QrVectorColors(
                        dark = QrVectorColor
                            .Solid(AppColors.BLACK),
                        ball = QrVectorColor.Solid(
                            AppColors.BLACK
                        ),
                        frame = QrVectorColor.LinearGradient(
                            colors = listOf(
                                0f to AppColors.BLACK,
                                1f to AppColors.BLACK,
                            ),
                            orientation = QrVectorColor.LinearGradient
                                .Orientation.LeftDiagonal
                        )
                    )
                )
                .setShapes(
                    QrVectorShapes(
                        darkPixel = QrVectorPixelShape
                            .RoundCorners(.5f),
                        ball = QrVectorBallShape
                            .RoundCorners(.25f),
                        frame = QrVectorFrameShape
                            .RoundCorners(.25f),
                    )
                ).build()


            val drawable: Drawable = QrCodeDrawable(data, options, null)
            binding.myQr.setImageDrawable(drawable)

        }



    }

    override fun clickListener(bind: FragmentMyQrBinding) {

        bind.printBtn.setOnClickListener {

            bind.qrSharedLayout.printQr(context = requireContext())

        }

        bind.shareBtn.setOnClickListener {

            bind.qrSharedLayout.sharedQR(context = requireContext())

        }

        bind.shareViaEmail.setOnClickListener {

            bind.qrSharedLayout.sharedQR(context = requireContext(), isViaEmail = true)

        }

    }

}