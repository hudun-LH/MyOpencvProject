package org.opencv.samples.tutorial2;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;

import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;


public class Tutorial2Activity extends Activity implements OnTouchListener,CvCameraViewListener2{
    private Mat mRgba;
    private Mat mGray;
    
	private CameraBridgeViewBase mOpenCvCameraView;
	public static int touch_x,touch_y;
	
	private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
				case LoaderCallbackInterface.SUCCESS:
					System.loadLibrary("hogsvm");
					mOpenCvCameraView.setOnTouchListener(Tutorial2Activity.this);
			        mOpenCvCameraView.enableView();
					break;
				default:
				{
					super.onManagerConnected(status);
				} break;
			}
		}
	};
	
	ImageView image_view;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		setContentView(R.layout.activity_main);
		
		//image_view =  (ImageView) findViewById(R.id.image_view);
		 
		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.color_blob_detection_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
	}
	
	@Override
	public void onResume(){
	        super.onResume();
	        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
	}
	
	int pic_width;
	int pic_height;
	
	@Override
	public void onCameraViewStarted(int width, int height) {
		// TODO Auto-generated method stub
		//mRgba = new Mat(height, width, CvType.CV_8UC4);
		mGray = new Mat();
	    mRgba = new Mat();
	    pic_width = width/2;
	    pic_height = height/2;
	}

	@Override
	public void onCameraViewStopped() {
		// TODO Auto-generated method stub
		 mRgba.release();
	}

	boolean trainFlag = false;
	Mat mGray0;
	Size dsize;	
	Size dsize_r;
	double scale=0.25; //设置缩放倍数
	Bitmap bt3;
	
	boolean run_frame_flag = false;
	int run_frame;
	Mat subMat = new Mat();
	
	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// TODO Auto-generated method stub
		//mRgba = inputFrame.rgba();
		mGray = inputFrame.gray();
		//CvResize(mRgba,mRgba0,new Size(pic_width,pic_height));//CV_INTER_LINEAR
		//public static void resize(Mat src, Mat dst, Size dsize, double fx, double fy, int interpolation)
		
		//缩放
		//Utils.bitmapToMat(bmps, sceneMat);
	    //Size dsize = new Size(mGray.cols()/4, mGray.rows()/4);
	    //mGray0 = new Mat(dsize, CvType.CV_8UC1);
	    //Mat mGray0 = new Mat();
	    //Imgproc.resize(mGray, mGray0, dsize, 0, 0, Imgproc.INTER_LINEAR);//LINEAR 
	    //Bitmap btt = Bitmap.createBitmap(mGray.cols()/4, mGray.rows()/4, Config.RGB_565);    
	    //Utils.matToBitmap(mGray0, btt);
	    //image_view.setImageBitmap(btt);
	    
	    //Mat mGray1 = new Mat();
	    //Imgproc.resize(mGray0, mGray1, new Size(mGray.cols(), mGray.rows()), 0, 0, Imgproc.INTER_LINEAR);//LINEAR 
	    
		run_frame++;
		if(run_frame>4){
		   run_frame  = 0;
		   //PreviceGray.grayProc(mRgba.getNativeObjAddr(),touch_x, touch_y);
//		   Log.e("yulinghan","yulinghan onCameraFrame touch_x="+touch_x);	
		}
		
		if(dsize==null){
    		dsize = new Size(mGray.cols()/2, mGray.rows()/2);
    		dsize_r = new Size(mGray.cols(), mGray.rows());
    	    mGray0 = new Mat();
    	}
    	
    	Imgproc.resize(mGray, mGray0, dsize, 0, 0, Imgproc.INTER_NEAREST);//LINEAR 
    	HogFeatures_0(mGray.getNativeObjAddr());
	    Imgproc.resize(mGray0, mGray, dsize_r, 0, 0, Imgproc.INTER_NEAREST);//LINEAR INTER_LINEAR 
		
		/*
		if(!trainFlag){
		    HogSvmTrain();
		    trainFlag = true;
	    }else{
	    	if(dsize==null){
	    		dsize = new Size(mGray.cols()/2, mGray.rows()/2);
	    		dsize_r = new Size(mGray.cols(), mGray.rows());
	    	    mGray0 = new Mat();
	    	}
	    	
	    	Imgproc.resize(mGray, mGray0, dsize, 0, 0, Imgproc.INTER_NEAREST);//LINEAR 
		    HogFeatures(mGray0.getNativeObjAddr()); //调用Native接口函数
		    Imgproc.resize(mGray0, mGray, dsize_r, 0, 0, Imgproc.INTER_NEAREST);//LINEAR INTER_LINEAR 
		    
		    //Rect rec = new Rect(0, 0, 64, 64);
			//Mat(int rows, int cols, int type,const Scalar& s)
			//Mat mat1Sub = new Mat(mGray.cols(), mGray.rows(), CvType.CV_8UC1, new Scalar(0,255,0));//mGray.submat(rec);//0, mat2.rows(), 0, mat2.cols());
			//Mat mat2Sub = mat1Sub.submat(rec);
			//Core.addWeighted(mat2Sub, 1, mGray, 0.7, 0.5, mGray);
	    }
	    */
	    
	    run_frame++;
		if(run_frame>4){
		   run_frame  = 0;
		   //PreviceGray.grayProc(mRgba.getNativeObjAddr(),touch_x, touch_y);
//		   Log.e("yulinghan","yulinghan onCameraFrame touch_x="+touch_x);	
		}

		if(run_frame_flag){
		   //    public static void rectangle(Mat img, Point pt1, Point pt2, Scalar color, int thickness, int lineType, int shift)
		   //Core.rectangle(mGray, new Point(touch_x, touch_y),new Point(touch_x+64, touch_y+64),new Scalar(255,0,0), 3, 8, 0);
		   // PreviceGray.grayProc_0(mGray.getNativeObjAddr(), subMat.getNativeObjAddr());
		}
		
	    return mGray;
	}
	@Override
	public boolean onTouch(View arg0, MotionEvent arg1) {
		// TODO Auto-generated method stub
		touch_x = (int)arg1.getX();
		touch_y = (int)arg1.getY();
		Log.e("yulinghan","yulinghan onTouch touch_x="+touch_x);
		
		if(mGray!=null){
			   subMat = mGray.submat(new Rect(touch_x, touch_y, touch_x+64, touch_y+64));
			   run_frame_flag = true;
		}
		return false;
	}
	
	public native void HogSvmTrain();
	public native void HogFeatures(long matAddrGr);
	public native void HogFeatures_0(long matAddrGr);
	public native void grayProc_0(long imageAddr, long tempAddr);
}
