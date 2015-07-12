/*

The MIT License (MIT)
 
Copyright (c) Melvin Cabatuan
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/


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
