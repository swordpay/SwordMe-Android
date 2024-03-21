package org.telegram.sword.app.bottomSheet

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.app.Dialog
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.Matrix
import android.graphics.drawable.ColorDrawable
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import android.view.View
import android.view.Window
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.core.content.FileProvider
import androidx.lifecycle.lifecycleScope
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentCameraOrGalleryBinding
import org.telegram.sword.app.common.AppConst.DeviceBrand
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.AppConst.Key
import org.telegram.sword.app.common.AppConst.Key.GALLERY
import org.telegram.sword.app.common.AppConst.Key.PHOTO_CAPTURE
import org.telegram.sword.app.common.AppConst.Key.VIDEO_OR_PHOTO
import org.telegram.sword.app.common.AppConst.Key.WRAP
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.common.extansion.getRealPathFromURI
import org.telegram.sword.app.common.extansion.isGranted
import org.telegram.sword.app.common.helper.ui.showPermissionsEnableDialog
import java.io.*
import java.util.*


class CameraOrGallery(private val fetchMediaUri: ((File,Uri) -> Unit)?=null,private val fetchMedia: (File) -> Unit,
                      private val isFilShare:Boolean = false) : BaseBottomSheet<FragmentCameraOrGalleryBinding, BaseViewModel>(
    R.layout.fragment_camera_or_gallery,height = WRAP){

    private var latestTmpUri: Uri? = null

    private  var isCamera:Boolean = false

    private var ext = ".jpg"

    private var isVideo = false


    override fun getViewBinding(view: View) = FragmentCameraOrGalleryBinding .bind(view)

    override fun onBottomSheetCreated(view: View) {

    }

    override fun configUi() {

    }

    override fun clickListener(bind: FragmentCameraOrGalleryBinding) {

        bind.uploadPhotoDismissBtn.setOnClickListener{ dismiss()}

        bind.photoCapture.setOnClickListener{

                photoOrVideoDialog()

        }

        bind.photoInGallery.setOnClickListener{ openGallery() }
    }


    fun photoOrVideoDialog() {

        val dialog = Dialog(requireActivity())
        requireActivity().runOnUiThread {
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
            dialog.setCancelable(true)
            dialog.setContentView(R.layout.photo_or_video)
            val photoBtn: LinearLayoutCompat = dialog.findViewById(R.id.photoBtn)
            val videoBtn: LinearLayoutCompat = dialog.findViewById(R.id.videoBtn)

            photoBtn.setOnClickListener {



                ext = ".jpg"
                isVideo = false

                openMediaCapture()
                dialog.dismiss()
            }

            videoBtn.setOnClickListener {


                ext = ".mp4"
                isVideo = true

                openMediaCapture()
                dialog.dismiss()
            }
            dialog.show()
        }
    }




   val takeMediaResult =  registerForActivityResult(ActivityResultContracts.TakePicture()) { isSuccess ->
                if (isSuccess) {
                    latestTmpUri?.let { uri ->
                        try {

                                val bitmap = MediaStore.Images.Media.getBitmap(
                                    requireActivity().contentResolver,
                                    uri
                                )

                                val tempUri = getImageUri(requireContext().applicationContext, bitmap)
                                val file = File(tempUri.getRealPathFromURI(requireActivity()))
                                fetchMediaUri?.let { it(file,tempUri!!) }

                            dismiss()
                        } catch (e: Exception) {
                            dismiss()
                        }
                    }
                }
            }



    val pickMedia = registerForActivityResult(ActivityResultContracts.PickVisualMedia()) { uri ->

        if (uri != null) {

            val file = File(uri.getRealPathFromURI(requireActivity()))

            fetchMediaUri?.let { it(file,uri) }

        } else {

        }

        this@CameraOrGallery.dismiss()
    }



    private fun takeMedia() {
        isCamera = true
        lifecycleScope.launchWhenStarted {
            getTmpFileUri().let { uri ->
                latestTmpUri = uri
                takeMediaResult.launch(uri)
            }
        }
    }


    @SuppressLint("IntentReset")
    private fun selectMediaFromGallery() {
        isCamera = false

         pickMedia.launch(PickVisualMediaRequest(ActivityResultContracts.PickVisualMedia.ImageAndVideo))

    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {

        if (resultCode == Activity.RESULT_OK ) {


            val uri = data?.data


            uri?.let {
                try {

                    if (requestCode == VIDEO_OR_PHOTO || requestCode== GALLERY){


                            if (uri.toString().contains("image")) {

                                val bitmap = MediaStore.Images.Media.getBitmap(
                                    requireActivity().contentResolver,
                                    uri
                                )
                                val tempUri = getImageUri(requireContext().applicationContext, bitmap)
                                val finalFile = File(tempUri.getRealPathFromURI(requireActivity()))

                                fetchMediaUri?.let { it(finalFile,uri) }


                            } else  if (uri.toString().contains("video")) {


                                val file  = saveVideoToAppScopeStorage(requireActivity(),uri)

                                fetchMediaUri?.let { it(file!!,uri) }

                            }

                            this@CameraOrGallery.dismiss()

                    }else if (requestCode== Key.VIDEO_CAPTURE){

                        val file  = saveVideoToAppScopeStorage(requireActivity(),uri)

                        fetchMediaUri?.let { it(file!!,uri) }
                        this@CameraOrGallery.dismiss()
                    }else{
                        this@CameraOrGallery.dismiss()

                    }



                } catch (e: Exception) {
                    this@CameraOrGallery.dismiss()
                }
            }

        }
    }

    private fun getTmpFileUri(): Uri {
        val tmpFile = File.createTempFile("tmp_sword", ".jpg", requireActivity().cacheDir).apply {
            createNewFile()
            deleteOnExit()
        }
        return FileProvider.getUriForFile(requireContext(), "com.swordpay.me.provider", tmpFile)
    }

    private fun openMediaCapture() {
        isCamera = true
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {

            if (isGranted(
                    permissions = arrayOf(
                        Manifest.permission.CAMERA,
                        Manifest.permission.READ_MEDIA_AUDIO,
                        Manifest.permission.READ_MEDIA_IMAGES,
                        Manifest.permission.READ_MEDIA_VIDEO
                    ), requestCode = PHOTO_CAPTURE
                )
            ) {
                if (isVideo){
                    val intent = Intent(MediaStore.ACTION_VIDEO_CAPTURE)
                    startActivityForResult(intent, Key.VIDEO_CAPTURE)
                }else{
                    takeMedia()
                }

            }

        }else{
            if (isGranted(
                    permissions = arrayOf(
                        Manifest.permission.CAMERA,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE
                    ), requestCode = PHOTO_CAPTURE
                )
            ) {
                if (isVideo){
                    val intent = Intent(MediaStore.ACTION_VIDEO_CAPTURE)
                    startActivityForResult(intent, Key.VIDEO_CAPTURE)
                }else{
                    takeMedia()
                }

            }
        }
    }


    private fun openGallery() {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {

            if (isGranted(
                    permissions = arrayOf(Manifest.permission.READ_MEDIA_IMAGES,Manifest.permission.READ_MEDIA_VIDEO),
                    requestCode = GALLERY
                )
            ) {
                selectMediaFromGallery()
            }
        }else{

            if (isGranted(
                    permissions = arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                    requestCode = GALLERY
                )
            ) {
                selectMediaFromGallery()
            }
        }


    }

    private fun getImageUri(inContext: Context, inImage: Bitmap): Uri? {
        var path = EMPTY
        try {
            val title = System.currentTimeMillis().toString()
            val h = inImage.height.toFloat()
            val w = inImage.width.toFloat()
            val default = 640f
            val resized = Bitmap.createScaledBitmap(inImage, (default / (h / w)).toInt(), 640, true)
            val rotate = flipIMage(resized)
            path = MediaStore.Images.Media.insertImage(
                inContext.contentResolver,
                rotate,
                title,
                "sword"
            )
        } catch (e: Exception) { Unit }
        return Uri.parse(path)
    }

    fun saveVideoToAppScopeStorage(context: Context, videoUri: Uri?): File? {
        val mimeType: String? = context.contentResolver.getType(videoUri!!)

        if(videoUri == null || mimeType == null){
            return null
        }

        val fileName = "${System.currentTimeMillis()}.${mimeType.substring(mimeType.indexOf("/") + 1)}"

        val inputStream = context.contentResolver.openInputStream(videoUri)


        val file = File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).absolutePath + "/" + "Camera/", fileName)
        file.deleteOnExit()
        file.createNewFile()
        val out = FileOutputStream(file)
        val bos = BufferedOutputStream(out)

        val buf = ByteArray(1024)
        inputStream?.read(buf)
        do {
            bos.write(buf)
        } while (inputStream?.read(buf) !== -1)

        bos.close()
        inputStream.close()

        return file
    }

    @Deprecated("Deprecated in Java")
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantedResults: IntArray
    ) {
        var isGranted = true

        if (grantedResults.isNotEmpty()) {
            grantedResults.forEach {
                if (it != PackageManager.PERMISSION_GRANTED) {
                    isGranted = false
                    return@forEach
                }
            }

            if (isGranted) {
                when (requestCode) {

                    PHOTO_CAPTURE -> {

                        if (isVideo){
                            val intent = Intent(MediaStore.ACTION_VIDEO_CAPTURE)
                            startActivityForResult(intent, 100)
                        }else{
                            takeMedia()
                        }

                    }

                    GALLERY -> selectMediaFromGallery()
                }
            }else {


                showPermissionsEnableDialog(requireActivity(), permissionName = if (requestCode == PHOTO_CAPTURE) resources.getString(R.string.camera) else resources.getString(R.string.gallery))
            }
        }
    }
    private fun fixOrientation(): Int {

        return  when(Build.BRAND.lowercase()){

                 DeviceBrand.SAMSUNG ->{

                     if (isCamera) 90 else -90

                 }
                 else ->0

        }

    }
    private fun flipIMage(bitmap: Bitmap): Bitmap? {

        val matrix = Matrix()
        val rotation: Int = fixOrientation()
        matrix.postRotate(rotation.toFloat())
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.width, bitmap.height, matrix, true)
    }


}