package org.telegram.sword.app.home.qrScan.profileQrPage

import android.annotation.SuppressLint
import android.graphics.drawable.Drawable
import androidx.core.content.ContextCompat
import org.telegram.PhoneFormat.PhoneFormat
import org.telegram.customqrgenerator.QrData
import org.telegram.customqrgenerator.vector.QrCodeDrawable
import org.telegram.customqrgenerator.vector.QrVectorOptions
import org.telegram.customqrgenerator.vector.style.*
import org.telegram.messenger.*
import org.telegram.messenger.databinding.ActivityProfileQrPageBinding
import org.telegram.sword.app.common.base.BaseActivity
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.colors.AppColors
import org.telegram.sword.app.common.extansion.printQr
import org.telegram.sword.app.common.extansion.sharedQR
import org.telegram.sword.domain.common.BaseUrl
import org.telegram.tgnet.TLRPC

class ProfileQrPage : BaseActivity<ActivityProfileQrPageBinding,BaseViewModel>() {

  var userId:Long? =null
  var chatId:Long? =null
  var username:String?=null

    override fun getViewBinding()= ActivityProfileQrPageBinding.inflate(layoutInflater)

    override fun onActivityCreated() {
       userId = intent.getLongExtra("user_id",0L)
       chatId = intent.getLongExtra("chat_id", 0L)

        val currentAccount = UserConfig.selectedAccount

        val messagesController = MessagesController.getInstance(currentAccount)

        if (userId != null && userId !=0L) {
            val user: TLRPC.User? = messagesController.getUser(userId)
            if (user != null) {

                username = UserObject.getPublicUsername(user)

                if (username==null){

                    username = PhoneFormat.getInstance().format("+" + user.phone).toString().replace(" ","");
                }

            }
        } else if (chatId != null && chatId !=0L) {

            val chat: TLRPC.Chat? = messagesController.getChat(chatId)

            if (chat != null) {

                username = ChatObject.getPublicUsername(chat)

            }
        }


    }

    @SuppressLint("Range")
    override fun configUi() {

        if (username.isNullOrEmpty()){

            showMessageDialog(title = "Qr", message = "Username not found", function = {onBackPressed()})

        }else{


        val data = QrData.Text(BaseUrl.DEEPLINK_BASE_URL +username)


        val options = QrVectorOptions.Builder()
            .setLogo(
                QrVectorLogo(

                    drawable = ContextCompat.getDrawable(this, R.drawable.qr_center_icon),
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


        val drawable : Drawable = QrCodeDrawable(data, options,null)
        binding.myQr.setImageDrawable(drawable)


        }
    }

    override fun clickListener(bind: ActivityProfileQrPageBinding) {

        bind.printBtn.setOnClickListener{

            bind.qrSharedLayout.printQr(context = this)

        }

        bind.shareBtn.setOnClickListener{

            bind.qrSharedLayout.sharedQR(context = this)

        }

        bind.shareViaEmail.setOnClickListener{

            bind.qrSharedLayout.sharedQR(context = this, isViaEmail = true)

        }

        bind.topBar.onBackBtn.setOnClickListener {

            onBackPressed()
        }

    }

}