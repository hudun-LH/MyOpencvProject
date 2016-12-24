package com.example.carplate;


public class CarPlateDetection {
	public static native long matchTemplate(long imageAddr, long tempAddr);
	public static native long matchTemplate11(long imageAddr, long tempAddr);
	public static native long OpticalFlow(long imageAddr, long tempAddr);
	public static native long contourProc(long imageAddr, long tempAddr);
	public static native long contourProc0(long imageAddr, long tempAddr);
	public static native long contourProc1(long imageAddr, long tempAddr);
	public static native long contourProc2(long imageAddr, long tempAddr);
	public static native String ImageProc(int[] pixels, int w, int h,String path);
	public static native long featureTest(long objImage, long sceneImage);
	public static native void doSelectRect(int objW, int objH);
	public static native long doCamshift(long imageGray);
	public static native long doHaarClassifier(long imageGray);
	public static native long doImageShow(long imageGray);
	public static native long doLKOpenCV(long imageGray);
	public static native long doLKOpenCV0(long imageGray);
	public static native long doBlending(long imageAddr, long tempAddr);
	public static native long doBlending0(long imageAddr, long tempAddr);	
	public static native long doBlending1(long imageAddr, long tempAddr);	
	public static native long doStitching(long imageAddr, long tempAddr);	
	public static native long doStitchingDetailed(long imageAddr, long tempAddr);	
}
