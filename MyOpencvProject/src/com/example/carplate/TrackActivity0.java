package com.example.carplate;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;


import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;
import android.app.Activity;
import android.content.pm.ActivityInfo;


public class TrackActivity0 extends Activity implements OnTouchListener,CvCameraViewListener2{
    private Mat mRgba;
    private Mat mGray;
    
	private CameraBridgeViewBase mOpenCvCameraView;
	public static int touch_x,touch_y;
	
	private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
				case LoaderCallbackInterface.SUCCESS:
					System.loadLibrary("imageproc");
					mOpenCvCameraView.setOnTouchListener(TrackActivity0.this);
			        mOpenCvCameraView.enableView();
					break;
				default:
				{
					super.onManagerConnected(status);
				} break;
			}
		}
	};
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		setContentView(R.layout.trackactivity0_main);
		
		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.color_blob_detection_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
	}
	
	@Override
	public void onResume(){
	        super.onResume();
	        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
	}
	
	@Override
	public void onCameraViewStarted(int width, int height) {
		// TODO Auto-generated method stub
		mRgba = new Mat(height, width, CvType.CV_8UC4);
		mGray = new Mat();
	}

	@Override
	public void onCameraViewStopped() {
		// TODO Auto-generated method stub
		 mRgba.release();
		 mGray.release();
	}

	boolean run_frame_flag = false;
	int run_frame;
	Mat subMat;
	long add_frame;
	
	boolean frameFlag=false;
	long frame_addr;
	    
	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// TODO Auto-generated method stub
		mRgba = inputFrame.rgba();
		//mGray = inputFrame.gray();
		
		run_frame++;
		if(run_frame>4){
		   run_frame  = 0;
		   //PreviceGray.grayProc(mRgba.getNativeObjAddr(),touch_x, touch_y);
//		   Log.e("yulinghan","yulinghan onCameraFrame touch_x="+touch_x);	
		}

		/*
		if(run_frame_flag){
		   //    public static void rectangle(Mat img, Point pt1, Point pt2, Scalar color, int thickness, int lineType, int shift)
		   //Core.rectangle(mGray, new Point(touch_x, touch_y),new Point(touch_x+64, touch_y+64),new Scalar(255,0,0), 3, 8, 0);
			//add_frame = grayProc_0(mGray.getNativeObjAddr(), subMat.getNativeObjAddr());
			//mGray = new Mat(add_frame);
			
			add_frame = CarPlateDetection.matchTemplate(mGray.getNativeObjAddr(), subMat.getNativeObjAddr());
			//mGray = new Mat(add_frame);
			
		}
		*/
		/*
		if(run_frame_flag){
        	if(frameFlag){
            	System.out.println("        doSelectRect         ");
            	frameFlag = false;
            	//mCamShift = new Mat();
                CarPlateDetection.doSelectRect(mRgba.rows(), mRgba.cols());
            }else{
            	frame_addr = CarPlateDetection.doCamshift(mRgba.getNativeObjAddr());
            	
            	if(frame_addr==0 || frame_addr==1){
            		return mRgba;
            	}
            	mRgba = new Mat(frame_addr);
            }
        }
		*/
		
		if(run_frame_flag){
		   //frame_addr = CarPlateDetection.doHaarClassifier(mRgba.getNativeObjAddr()); // Haar classifier
           frame_addr = CarPlateDetection.doLKOpenCV0(mRgba.getNativeObjAddr()); // optical flow Lucas-Kanade
           mRgba = new Mat(frame_addr);
        }
		
	    return mRgba;
	}
	@Override
	public boolean onTouch(View arg0, MotionEvent arg1) {
		// TODO Auto-generated method stub
		touch_x = (int)arg1.getX();
		touch_y = (int)arg1.getY();
		Log.e("yulinghan","yulinghan onTouch touch_x="+touch_x+", "+touch_y);
		
		if(mGray!=null){
		   //subMat = new Mat();
		   //subMat = mGray.submat(new Rect(touch_x, touch_y, 100, 100));
		   //run_frame_flag = true;
		}
		run_frame_flag = true;
		frameFlag=true;
		   
		return false;
	}
	
	public static native void grayProc(long k,int draw_x,int draw_y);
	public static native long grayProc_0(long imageAddr, long tempAddr);
}
