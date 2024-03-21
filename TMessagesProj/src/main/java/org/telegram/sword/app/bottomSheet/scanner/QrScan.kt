package org.telegram.sword.app.bottomSheet.scanner

import android.annotation.SuppressLint
import android.util.DisplayMetrics
import android.view.View
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import com.google.mlkit.vision.barcode.BarcodeScanner
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.common.InputImage
import org.telegram.messenger.R
import org.telegram.messenger.databinding.FragmentQrScanBinding
import org.telegram.sword.app.common.AppConst.Key.MACH
import org.telegram.sword.app.common.base.BaseBottomSheet
import org.telegram.sword.app.common.base.BaseViewModel
import org.telegram.sword.app.home.qrScan.tab.qrScanner.util.getAspectRatio


class QrScan(private val address:(address:String) -> Unit) : BaseBottomSheet<FragmentQrScanBinding,
        BaseViewModel>(R.layout.fragment_qr_scan, isDraggable = true,
    height = MACH,) {

    private lateinit var camera: Camera

    private lateinit var cameraProvider: ProcessCameraProvider

    private val screenAspectRatio by lazy {
        val metrics = DisplayMetrics().also { binding.previewView.display.getRealMetrics(it) }
        metrics.getAspectRatio()
    }

    override fun getViewBinding(view: View) =  FragmentQrScanBinding.bind(view)

    override fun onBottomSheetCreated(view: View) {


    }

    override fun configUi() {

    }


    override fun clickListener(bind: FragmentQrScanBinding) {

        bind.dismissButton.setOnClickListener {

            dismiss()
        }

    }


    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(requireActivity())
        cameraProviderFuture.addListener( {
            val cameraProvider = cameraProviderFuture.get()
            bindPreview(cameraProvider)
        }, ContextCompat.getMainExecutor(requireActivity()))

    }


    @SuppressLint("UnsafeExperimentalUsageError", "UnsafeOptInUsageError")
    private fun bindPreview(camProvider: ProcessCameraProvider) {
        cameraProvider = camProvider

        val previewUseCase = Preview.Builder()
            .setTargetRotation(binding.previewView.display.rotation)
            .setTargetAspectRatio(screenAspectRatio)
            .build().also {
                it.setSurfaceProvider(binding.previewView.surfaceProvider)
            }
        val barcodeScanner = BarcodeScanning.getClient()
        val analysisUseCase = ImageAnalysis.Builder()
            .setTargetRotation(binding.previewView.display.rotation)
            .setTargetAspectRatio(screenAspectRatio)
            .build().also {
                it.setAnalyzer(
                    ContextCompat.getMainExecutor(requireContext())
                ) { imageProxy ->
                    processImageProxy(barcodeScanner, imageProxy)
                }
            }
        val useCaseGroup = UseCaseGroup.Builder().addUseCase(previewUseCase).addUseCase(
            analysisUseCase
        ).build()

        camera = cameraProvider.bindToLifecycle(
            this,
            CameraSelector.Builder().requireLensFacing(CameraSelector.LENS_FACING_BACK).build(),
            useCaseGroup
        )
    }


    @SuppressLint("UnsafeExperimentalUsageError", "UnsafeOptInUsageError")
    private fun processImageProxy(barcodeScanner: BarcodeScanner, imageProxy: ImageProxy) {

        imageProxy.image?.let { image ->
            val inputImage = InputImage.fromMediaImage(image, imageProxy.imageInfo.rotationDegrees)
            barcodeScanner.process(inputImage)
                .addOnSuccessListener { barcodeList ->

                    if (!barcodeList.isNullOrEmpty()) {

                        if (!barcodeList[0].rawValue.isNullOrEmpty()){

                            cameraProvider.unbindAll()

                            barcodeList[0].rawValue?.let { address(it) }
                            dismiss()

                        }
                    }

                }.addOnFailureListener {
                    image.close()
                    imageProxy.close()
                }.addOnCompleteListener {
                    image.close()
                    imageProxy.close()
                }
        }
    }


    override fun onPause() {
        super.onPause()
        cameraProvider.unbindAll()
    }

    override fun onResume() {

        try {
            startCamera()
        }catch (e:Exception){ Unit }

        super.onResume()


    }



}