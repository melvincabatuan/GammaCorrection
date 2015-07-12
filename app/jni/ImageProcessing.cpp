#include "com_cabatuan_gammacorrection_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "GammaCorrection"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



float t;
Mat srcBGR;
Mat lut; 
uchar *corr = NULL; // Correction LUT

/*
 * Class:     com_cabatuan_gammacorrection_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_com_cabatuan_gammacorrection_MainActivity_process
  (JNIEnv *pEnv, jobject clazz, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);
    
    
/***********************************************************************************************/
 	// Create the correction LUT on initialization
 	if (corr == NULL){
 	
 		uchar * corr = (uchar*)malloc(256*sizeof(uchar));

 		for(int i = 0; i < 256; i++){
    		corr[i] = (int)( pow( (float) i / 255.0, (float)1.0 / 2.2 ) * 255.0 );  // 2.2 is gamma
 		}
 		
 		lut = Mat(1, 256, CV_8UC1, corr);
 	
 	}

/***********************************************************************************************/
    /// Native Image Processing HERE... 
    if(srcBGR.empty())
       srcBGR = Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC3);
    
    
    cvtColor(src, srcBGR, CV_YUV420sp2RGB);  // 3-6 ms

   
    t = (float)getTickCount();   
    LUT( srcBGR, lut, srcBGR);
    t = 1000*((float)getTickCount() - t)/getTickFrequency();
    LOGI("Native processing took %0.2f ms.", t);
    
    
    cvtColor(srcBGR, mbgra, CV_BGR2BGRA);
 
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}
