package com.example.carplate;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Size;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.calib3d.Calib3d;
import org.opencv.core.*;
import org.opencv.features2d.DescriptorExtractor;
import org.opencv.features2d.DescriptorMatcher;
import org.opencv.features2d.FeatureDetector;
import org.opencv.features2d.DMatch;
import org.opencv.features2d.Features2d;
import org.opencv.features2d.KeyPoint;
import org.opencv.imgproc.Imgproc;
import org.opencv.core.Point;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends Activity {
	private static ImageView imageView = null;  
	private static ImageView imageViews = null;  
	private Bitmap bmp = null;  
	private static Bitmap bmps = null; 
    Bitmap bmp0 = null; 
    Bitmap bmp_one = null;  
	Bitmap bmp_two = null;
			
	private static TextView m_text = null;
	private String path = null; //SDCARD 根目录
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.activity_main);
		imageViews = (ImageView) findViewById(R.id.image_view);  
		imageView = (ImageView) findViewById(R.id.image_view0);  
		m_text = (TextView) findViewById(R.id.myshow);
	    //将汽车完整图像加载程序中并进行显示
		 bmps = BitmapFactory.decodeResource(getResources(), R.drawable.test);  
	     //imageViews.setImageBitmap(bmps);
	     
	     bmp = BitmapFactory.decodeResource(getResources(), R.drawable.test00);  
	     //imageView0.setImageBitmap(bmp);
	     
	     bmp0 = BitmapFactory.decodeResource(getResources(), R.drawable.people);  
	     
	     bmp_one = BitmapFactory.decodeResource(getResources(), R.drawable.two);  
	     bmp_two = BitmapFactory.decodeResource(getResources(), R.drawable.one);  
	     
	     path = Environment.getExternalStorageDirectory().getAbsolutePath();//获取跟目录 
	     System.out.println(path);
	     
	     File f0 = new File(Environment.getExternalStorageDirectory(), "SVM.xml");
		 try {
			if(!f0.exists()){
				f0.createNewFile();
			}else{
				Log.i("Encoder", " SVM.xml exists ");  
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		 
		Log.i("Encoder", "f0.getPath() = "+f0.getPath()+", f0.getAbsolutePath() = "+f0.getAbsolutePath());  
	    Log.i("Encoder", "f0.getPath() = "+f0.getPath()+", f0.getAbsolutePath() = "+f0.getAbsolutePath());  
	}

	//OpenCV类库加载并初始化成功后的回调函数，在此我们不进行任何操作  
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {  
       @Override  
       public void onManagerConnected(int status) {  
           switch (status) {  
               case LoaderCallbackInterface.SUCCESS:{  
                   System.loadLibrary("imageproc");  
                   objMat = new Mat();
          		   sceneMat = new Mat();
          		   objMatch = new Mat();
               } break;  
               default:{  
                   super.onManagerConnected(status);  
               } break;  
           }  
       }  
   };  
   
   static String result=null;
   
   static Handler myHandler = new Handler(){
	   public void handleMessage(Message msg){
		   switch(msg.what){
		        case 0:
		        	  System.out.println("             00 myHandler -- 0            ");
		        	  Utils.bitmapToMat(bmps, sceneMat);
		        	  System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
		     	      Size dsize = new Size(sceneMat.cols()/4, sceneMat.rows()/4);
		     	      Mat mGray0 = new Mat(dsize, sceneMat.type());//CvType.CV_8UC1);
		     	      //Mat mGray0 = new Mat();
		     	      //Imgproc.resize(sceneMat, mGray0, dsize, 0, 0, Imgproc.INTER_LINEAR);//LINEAR 
		     	      //cvPyrDown( src, result1,CV_GAUSSIAN_5x5);  
		     	      Imgproc.pyrDown(sceneMat, mGray0);//
		     	      //Imgproc.pyrDown(sceneMat, mGray0, dsize);//
		     	      System.out.println("             11 myHandler -- 0            ");
		     	      Bitmap btt = Bitmap.createBitmap(mGray0.cols(), mGray0.rows(), Config.RGB_565);    
		     	      Utils.matToBitmap(mGray0, btt);
		     	      imageViews.setImageBitmap(btt);
			          break;
		        case 1:
		        	  m_text.setText(result);
		        	  System.out.println(" m_text.setText = "+result);
		        	  break;
		        case 2:
		        	  //objMatch
		        	  System.out.println("    2!   ");
		        	  //Bitmap bt3 = Bitmap.createBitmap(objMatch.cols(), objMatch.rows(), Config.RGB_565);
					  //Utils.matToBitmap(objMatch, bt3);
		        	  m_text.setText(str_detect);
		        	  imageView.setImageBitmap(bt2);
		        	  imageViews.setImageBitmap(bt3);
		        	  break;
		        case 3:
		        	  //objMatch
		        	  System.out.println("    3!   ");
		        	  imageView.setVisibility(View.GONE);
		        	  imageViews.setImageBitmap(bt3);
		        	  break;
		        case 4:
		        	  System.out.println("              myHandler -- 4            ");
		        	  imageView.setVisibility(View.GONE);
		     	      Bitmap btt0 = Bitmap.createBitmap(sceneMat.cols(), sceneMat.rows(), Config.RGB_565); 
		     	      System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
		     	     
		     	      Utils.matToBitmap(sceneMat, btt0);
		     	      imageViews.setImageBitmap(btt0);
			          break;
		        default:
		        	  break;
		   }
	   }
   };
   
   public void click(View view){
	   System.out.println("entering the jni");
	   final int w = bmps.getWidth();
	   final int h = bmps.getHeight();
	   final int[] pixels = new int[w * h];
	   
	   bmps.getPixels(pixels, 0, w, 0, 0, w, h);
	   imageViews.setImageBitmap(bmps);
	   /*
	   new Thread(){
		   public void run(){
	          try {
				 Thread.sleep(3000);
			  } catch (InterruptedException e) {
				 // TODO Auto-generated catch block
			 	 e.printStackTrace();
			  }
	          Message message = new Message();   
              message.what = 0;     
              myHandler.sendMessage(message);
		   }
	   }.start();
	  */
	   
	   // System.out.println(Environment.getExternalStorageState());
	   new Thread(){
		   public void run(){
	          result=CarPlateDetection.ImageProc(pixels, w, h,path);
	          System.out.println(result);
	          Message message = new Message();   
              message.what = 1;     
              myHandler.sendMessage(message);
		   }
	   }.start();
   }
   
   public static void updateMat(long addr) {
	    System.out.println("          update        ");
	    System.out.println("          update        ");
	    
	    sceneMat = new Mat(addr);
	    System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());

        Message message = new Message();   
        message.what = 4;     
        myHandler.sendMessage(message);
   }

   int  bleadFlag = 0;
   
   public void click6(View view){
	   System.out.println("              click6             ");
	   System.out.println("              click6             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   bleadFlag += 1;
	   if(bleadFlag>3){
		  bleadFlag = 1;
	   }
	   //bleadFlag = 2;
	   if(bleadFlag==1){
		   imageViews.setImageBitmap(bmp_two);
		   imageView.setImageBitmap(bmp_one);
		   
		   Utils.bitmapToMat(bmp_two, objMat);
		   Utils.bitmapToMat(bmp_one, sceneMat);
	       new Thread(){
		       public void run(){
			      try {
					 Thread.sleep(1500);
					 long addr = CarPlateDetection.doStitching(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 if(addr >0){
					    sceneMat = new Mat(addr);
					    System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
					 }
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(address); 
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		      }
	       }.start();
	   }else if(bleadFlag==2){
		   imageViews.setImageBitmap(bmp_two);
		   imageView.setImageBitmap(bmp_one);
		   
		   Utils.bitmapToMat(bmp_two, objMat);
		   Utils.bitmapToMat(bmp_one, sceneMat);
           
	       new Thread(){
		       public void run(){
			      try {
					 Thread.sleep(1500);
					 //CarPlateDetection.doBlending0
					 long addr = CarPlateDetection.doBlending0(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 if(addr >0){
					    sceneMat = new Mat(addr);
					    System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
					 }
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(address); 
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		      }
	       }.start();
	       
	   }else{
		   imageViews.setImageBitmap(bmp_two);
		   imageView.setImageBitmap(bmp_one);
		   
		   Utils.bitmapToMat(bmp_two, objMat);
		   Utils.bitmapToMat(bmp_one, sceneMat);
		   
		   System.out.println("     doStitching start   ");
	       new Thread(){
		       public void run(){
			      try {
					 Thread.sleep(1500);
					 //doStitching
					 long addr = CarPlateDetection.doStitchingDetailed(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 if(addr >0){
					    sceneMat = new Mat(addr);
					    //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_GRAY2BGR); 
					    System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
					 }
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(address); 
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		      }
	       }.start();
	   }
	   
   }
   
   boolean click1_item = false;
   
   boolean change_item = false;
   public void click1(View view){
	   System.out.println("              click1             ");
	   System.out.println("              click1             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   imageViews.setImageBitmap(bmps);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   
	   click1_item = !click1_item;
	   if(click1_item){
		   new Thread(){
			   public void run(){
				      try {
						 Thread.sleep(1500);
						 change_item = !change_item;
						 if(change_item){
						    long addr = CarPlateDetection.matchTemplate(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
						    sceneMat = new Mat(addr);
						    System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
						 }else{
						    long addr = CarPlateDetection.matchTemplate11(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
						 }
						 //long address = generateHistogram(bitmapMat.nativeObj);
						 //Mat histogram = new Mat(address);
						 
					  } catch (InterruptedException e) {
						 // TODO Auto-generated catch block  
					 	 e.printStackTrace();
					  }
			          Message message = new Message();   
		              message.what = 4;     
		              myHandler.sendMessage(message);
			   }
		   }.start();
	   }else{
		   new Thread(){
		       public void run(){
			      try {
					 Thread.sleep(1500);
					 long addr = CarPlateDetection.doBlending(objMat.getNativeObjAddr(), sceneMat.getNativeObjAddr());
					 sceneMat = new Mat(addr);
					 System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(address); 
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		      }
	       }.start();
	   }
   }
   
   boolean click2_flag = false;
   public void click2(View view){
	   System.out.println("              click2             ");
	   System.out.println("              click2             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   imageViews.setImageBitmap(bmps);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   
	   new Thread(){
		   public void run(){
			      try {
					 Thread.sleep(1500);
					 click2_flag = !click2_flag;
					 long addr;
					 if(click2_flag){
					    addr = CarPlateDetection.contourProc(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 }else{
						addr = CarPlateDetection.contourProc0(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr()); 
					 }
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(addr);
					 sceneMat = new Mat(addr);
					 System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		   }
	   }.start();
   }
   
   public void click3(View view){
	   System.out.println("              click3             ");
	   System.out.println("              click3             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   imageViews.setImageBitmap(bmps);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   
	   new Thread(){
		   public void run(){
			      try {
					 Thread.sleep(1500);
					 //long addr = CarPlateDetection.contourProc0(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(addr);
					 //sceneMat = new Mat(addr);
					 System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		   }
	   }.start();
   }
   
   boolean contourFlag=false;
   
   public void click4(View view){
	   System.out.println("              click4             ");
	   System.out.println("              click4             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   imageViews.setImageBitmap(bmps);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   
	   new Thread(){
		   public void run(){
			      try {
					 Thread.sleep(1500);
					 long addr;
					 contourFlag = !contourFlag;
					 if(contourFlag){
					     addr = CarPlateDetection.contourProc1(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 }else{
						 addr = CarPlateDetection.contourProc2(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr()); 
					 }
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(addr);
					 sceneMat = new Mat(addr);
					 System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		   }
	   }.start();
   }
   
   public void click5(View view){
	   System.out.println("              click5             ");
	   System.out.println("              click5             ");
	   
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);
	   
	   imageViews.setImageBitmap(bmps);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   //Imgproc.cvtColor(sceneMat, sceneMat, Imgproc.COLOR_RGBA2GRAY); 
	   System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	   
	   new Thread(){
		   public void run(){
			      try {
					 Thread.sleep(1500);
					 long addr = CarPlateDetection.OpticalFlow(sceneMat.getNativeObjAddr(), objMat.getNativeObjAddr());
					 //long address = generateHistogram(bitmapMat.nativeObj);
					 //Mat histogram = new Mat(addr);
					 sceneMat = new Mat(addr);
				  } catch (InterruptedException e) {
					 // TODO Auto-generated catch block  
				 	 e.printStackTrace();
				  }
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
		   }
	   }.start();
   }
   
   Mat objMatch;
   Mat objMat;
   static Mat sceneMat;
   Mat img_object;
   Mat img_scene;
   
   boolean testFeature = false;
   int detect_type=3;
   static Bitmap bt2;
   static Bitmap bt3;
   static String str_detect;
   
   public int FeatureT()
   {
	   int length, length0, i;
	   
	   img_object = objMat;
	   img_scene = sceneMat;
	      
	   Imgproc.cvtColor(img_object, img_object, Imgproc.COLOR_RGBA2GRAY);
	   Imgproc.cvtColor(img_scene, img_scene, Imgproc.COLOR_RGBA2GRAY);
	   
	   bt2 = Bitmap.createBitmap(img_object.cols(), img_object.rows(), Config.RGB_565);
       Utils.matToBitmap(img_object, bt2);
       
	   bt3 = Bitmap.createBitmap(img_scene.cols(), img_scene.rows(), Config.RGB_565);
       Utils.matToBitmap(img_scene, bt3);
       //imageViews.setImageBitmap(bt3);
       
       Message message = new Message();   
       message.what = 2;     
       myHandler.sendMessage(message);
       
       try {
		   Thread.currentThread();
		   Thread.sleep(4000);
	   } catch (InterruptedException e) {
		   // TODO Auto-generated catch block
		   e.printStackTrace();
	   }
       
	   //-- Step 1: Detect the keypoints using SURF Detector
	   FeatureDetector detector;
	   detect_type = 5;
	   Log.i("Feature", "  detect_type = "+detect_type);
	   /*
	   switch(detect_type){
	        case 1:
		         detector = FeatureDetector.create(FeatureDetector.FAST); // ok
		         str_detect = "FAST";
	             break;
	        case 2:
	        	 detector = FeatureDetector.create(FeatureDetector.STAR); // ok
	             str_detect = "STAR";
	             break;
	        case 3:
		         detector = FeatureDetector.create(FeatureDetector.SIFT);
		         str_detect = "SIFT";
	             break;
	        case 4:
		         detector = FeatureDetector.create(FeatureDetector.SURF);
		         str_detect = "SURF";
	             break;
	        case 5:
	        	 detector = FeatureDetector.create(FeatureDetector.ORB); // ok
	        	 str_detect = "ORB";
	             break;
	        case 6:
		         detector = FeatureDetector.create(FeatureDetector.MSER);
		         str_detect = "MSER";
	             break;
	        case 7:
		         detector = FeatureDetector.create(FeatureDetector.GFTT);
		         str_detect = "GFTT";
	             break;
	        case 8:
	        	 detector = FeatureDetector.create(FeatureDetector.HARRIS); // ok
	        	 str_detect = "HARRIS";
	             break;
	        case 9:
		         detector = FeatureDetector.create(FeatureDetector.SIMPLEBLOB);
		         str_detect = "SIMPLEBLOB";
	             break;
	        case 10:
		         detector = FeatureDetector.create(FeatureDetector.DENSE);
		         str_detect = "DENSE";
	             break;
	        case 11:
	        	 detector = FeatureDetector.create(FeatureDetector.BRISK);
	        	 str_detect = "BRISK";
	             break;
	        case 12:
		         detector = FeatureDetector.create(FeatureDetector.GRIDRETECTOR);
		         str_detect = "GRIDRETECTOR";
	             break;
	        default:
	        	 detector = FeatureDetector.create(FeatureDetector.FAST);
	        	 str_detect = "FAST";
	             break;
	   }
	   */
	   detector = FeatureDetector.create(FeatureDetector.ORB);
  	   str_detect = "ORB";
	   
	   MatOfKeyPoint keypoints_object = new MatOfKeyPoint();
	   MatOfKeyPoint keypoints_scene  = new MatOfKeyPoint();

	   detector.detect(img_object, keypoints_object);
	   detector.detect(img_scene, keypoints_scene);

	   KeyPoint[] obj_item = keypoints_object.toArray();
	   KeyPoint[] obj_item0 = keypoints_scene.toArray();
	   
	   length  = obj_item.length;
	   length0 = obj_item0.length;
	   
	   Log.i("Feature", "  keypoints_object = "+obj_item.length+", keypoints_scene = "+obj_item0.length); 
	   
	   Mat img_object0 = img_object;
	   Mat img_scene0  = img_scene;
	   
	   for(i = 0; i < length; i++ ) {
	       //-- Get the keypoints from the good matches
	       Point pts = obj_item[i].pt;
	       Core.rectangle(img_object0, 
	    		   new Point(pts.x-5, pts.y-5),
	    		   new Point(pts.x+5, pts.y+5),
	    		   new Scalar(0, 255, detect_type*20),  
	    		   2);
	       //scene.add(obj_item0[dgood_matches.get(i).trainIdx].pt);
	   }
	   
	   bt2 = Bitmap.createBitmap(img_object0.cols(), img_object0.rows(), Config.RGB_565);
       Utils.matToBitmap(img_object0, bt2);
       //imageView.setImageBitmap(bt2);
       
	   for(i = 0; i < length0; i++ ) {
	       //-- Get the keypoints from the good matches
	       Point pts = obj_item0[i].pt;
	       Core.rectangle(img_scene0, 
	    		   new Point(pts.x-5, pts.y-5),
	    		   new Point(pts.x+5, pts.y+5),
	    		   new Scalar(0, 255, detect_type*20),  
	    		   2);
	       //scene.add(obj_item0[dgood_matches.get(i).trainIdx].pt);
	   }
	   
	   bt3 = Bitmap.createBitmap(img_scene0.cols(), img_scene0.rows(), Config.RGB_565);
       Utils.matToBitmap(img_scene0, bt3);
       //imageViews.setImageBitmap(bt3);
       
       Message message00 = new Message();   
       message00.what = 2;     
       myHandler.sendMessage(message00);
       
       try {
		   Thread.currentThread();
		   Thread.sleep(4000);
	   } catch (InterruptedException e) {
		   // TODO Auto-generated catch block
		   e.printStackTrace();
	   }
       
       //-- Step 2: Calculate descriptors (feature vectors)
       DescriptorExtractor extractor;
	   extractor = DescriptorExtractor.create(DescriptorExtractor.ORB);//ORB  BRIEF SURF SIFT FREAK
	   
	   Mat descriptors_object = new Mat();
	   Mat descriptors_scene = new Mat();

	   extractor.compute(img_object, keypoints_object, descriptors_object); 
	   extractor.compute(img_scene, keypoints_scene, descriptors_scene);

	   Log.w("Feature","     DescriptorExtractor    \n");
	   
	   bt2 = Bitmap.createBitmap(descriptors_object.cols(), descriptors_object.rows(), Config.RGB_565);
       Utils.matToBitmap(descriptors_object, bt2);
       
       bt3 = Bitmap.createBitmap(descriptors_scene.cols(), descriptors_scene.rows(), Config.RGB_565);
       Utils.matToBitmap(descriptors_scene, bt3);
       
       Message message0 = new Message();   
       message0.what = 2;     
       myHandler.sendMessage(message0);
       
       try {
		   Thread.currentThread();
		   Thread.sleep(4000);
	   } catch (InterruptedException e) {
		   // TODO Auto-generated catch block
		   e.printStackTrace();
	   }
       
       //-- Step 3: Matching descriptor vectors using FLANN matcher
	   DescriptorMatcher matcher;
	   matcher = DescriptorMatcher.create(DescriptorMatcher.BRUTEFORCE);//BRUTEFORCE  FLANNBASED  BRUTEFORCE_HAMMING  BRUTEFORCE_HAMMINGLUT
	   
	   MatOfDMatch matches = new MatOfDMatch();
	   matcher.match(descriptors_object, descriptors_scene, matches);

	   Log.w("Feature","     DescriptorMatcher    \n");
	   /*
	   Mat matches_object = (Mat)matches;
	   bt3 = Bitmap.createBitmap(matches_object.cols(), matches_object.rows(), Config.RGB_565);
       Utils.matToBitmap(matches_object, bt3);
       
       Message message1 = new Message();   
       message1.what = 3;     
       myHandler.sendMessage(message1);
       */
	   double max_dist = 0; 
	   double min_dist = 100;

	   DMatch[] dmatch;
	   dmatch = matches.toArray();
	   
	   length = dmatch.length;
	   Log.w("Feature","     DescriptorMatcher  =  "+dmatch.length+",  "+matches.height());
	   
	   //-- Quick calculation of max and min distances between keypoints
	   //for( int i = 0; i < descriptors_object.rows(); i++ )//rows 
	   for(i=0; i<length; i++){
		   double dist = dmatch[i].distance; //.distance()
	       if( dist < min_dist ) min_dist = dist;
	       if( dist > max_dist ) max_dist = dist;
	   }

	   Log.w("Feature","-- Max dist : "+max_dist );
	   Log.w("Feature","-- Min dist : "+min_dist );

	   //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	   MatOfDMatch good_matches = new MatOfDMatch();
	   List<DMatch>  dgood_matches = new ArrayList<DMatch>();
	   
	   min_dist = 8*min_dist;
	   
	   for(i=0; i <length; i++){  
		  if( dmatch[i].distance < min_dist ){ 
		     dgood_matches.add(dmatch[i]); 
		  }
	   }
	   
	   good_matches.fromList(dgood_matches);
	   Log.i("Feature", " good_matches "+good_matches.height()+", "+dgood_matches.size()); 
	   
	   
	   Mat img_matches = new Mat();
	   //public static void drawMatches(Mat img1, MatOfKeyPoint keypoints1, 
	   //          Mat img2, MatOfKeyPoint keypoints2, MatOfDMatch matches1to2, 
	   //          Mat outImg, Scalar matchColor, Scalar singlePointColor, 
	   //          MatOfByte matchesMask, int flags)
	   
	   //public static void drawMatches(Mat img1, MatOfKeyPoint keypoints1, Mat img2, 
	   //                               MatOfKeyPoint keypoints2, MatOfDMatch matches1to2, Mat outImg)
	   Features2d.drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
               good_matches, img_matches);
	   /*
	   Features2d.drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
	                good_matches, img_matches, Scalar.all(-1), Scalar.all(-1),
	                null, Features2d.NOT_DRAW_SINGLE_POINTS); //new MatOfByte()  NOT_DRAW_SINGLE_POINTS = 2, DRAW_RICH_KEYPOINTS = 4
	   */
	   
	   Log.i("Feature", " drawMatches "+length+", "+good_matches.height()); 
	   
	   //Imgproc.cvtColor(img_matches, img_matches, Imgproc.COLOR_RGBA2GRAY);
	   
	   bt3 = Bitmap.createBitmap(img_matches.cols(), img_matches.rows(), Config.RGB_565);
       Utils.matToBitmap(img_matches, bt3);
       
       Message message1 = new Message();   
       message1.what = 3;     
       myHandler.sendMessage(message1);
	   
       try {
		   Thread.currentThread();
		   Thread.sleep(4000);
	   } catch (InterruptedException e) {
		   // TODO Auto-generated catch block
		   e.printStackTrace();
	   }
       
	   List<Point> obj = new ArrayList<Point>();
	   List<Point> scene = new ArrayList<Point>();

	   length = good_matches.height();
	   //length = dgood_matches.size();
	   
	   //KeyPoint[] obj_item = keypoints_object.toArray();
	   //KeyPoint[] obj_item0 = keypoints_scene.toArray();
	   
	   for(i = 0; i < length; i++ ) {
	       //-- Get the keypoints from the good matches
	       obj.add(obj_item[dgood_matches.get(i).queryIdx].pt);
	       scene.add(obj_item0[dgood_matches.get(i).trainIdx].pt);
	   }

	   //-- Localize the object
	   MatOfPoint2f objs = new MatOfPoint2f();
	   MatOfPoint2f scenes = new MatOfPoint2f();
	   
	   objs.fromList(obj);
	   scenes.fromList(scene);
	   Log.i("Feature", " findHomography  before! ");
	   Mat H = Calib3d.findHomography(objs, scenes);//, Calib3d.CV_RANSAC );
	   Log.i("Feature", " findHomography "); 
	   
	   if(H != null)
	   {
	   Log.i("Feature", " perspectiveTransform before "); 
	   /*
	   Core.perspectiveTransform(img_matches, img_matches, H);
	   
	   bt3 = Bitmap.createBitmap(img_matches.cols(), img_matches.rows(), Config.ARGB_8888);
       Utils.matToBitmap(img_matches, bt3);
       
       Message message2 = new Message();   
       message2.what = 3;     
       myHandler.sendMessage(message2);
       */
	   }
	   
	   Log.i("Feature", " perspectiveTransform "); 
       
	   Log.w("Feature","     FeatureDetector    \n"); 
	   return 1;
   }
   
   public int FeatureT1()
   {
	   int length;
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmps, sceneMat);
	   
	   img_object = objMat;
	   img_scene = sceneMat;
	   /*
	            FAST = 1,
	            STAR = 2,
	            SIFT = 3,
	            SURF = 4,
	            ORB = 5,
	            MSER = 6,
	            GFTT = 7,
	            HARRIS = 8,
	            SIMPLEBLOB = 9,
	            DENSE = 10,
	            BRISK = 11,
	            GRIDRETECTOR = 1000,
	  */          
	   //-- Step 1: Detect the keypoints using SURF Detector
	   FeatureDetector detector;
	   detect_type+= 1;
	   if(detect_type>=13){
	      detect_type = 1;
	   }
	   Log.i("Feature", "  detect_type = "+detect_type);
	   switch(detect_type){
	        case 1:
		         detector = FeatureDetector.create(FeatureDetector.FAST); // ok
		         str_detect = "FAST";
	             break;
	        case 2:
	        	 detector = FeatureDetector.create(FeatureDetector.STAR); // ok
	             str_detect = "STAR";
	             break;
	        case 3:
		         detector = FeatureDetector.create(FeatureDetector.SIFT);
		         str_detect = "SIFT";
	             break;
	        case 4:
		         detector = FeatureDetector.create(FeatureDetector.SURF);
		         str_detect = "SURF";
	             break;
	        case 5:
	        	 detector = FeatureDetector.create(FeatureDetector.ORB); // ok
	        	 str_detect = "ORB";
	             break;
	        case 6:
		         detector = FeatureDetector.create(FeatureDetector.MSER);
		         str_detect = "MSER";
	             break;
	        case 7:
		         detector = FeatureDetector.create(FeatureDetector.GFTT);
		         str_detect = "GFTT";
	             break;
	        case 8:
	        	 detector = FeatureDetector.create(FeatureDetector.HARRIS); // ok
	        	 str_detect = "HARRIS";
	             break;
	        case 9:
		         detector = FeatureDetector.create(FeatureDetector.SIMPLEBLOB);
		         str_detect = "SIMPLEBLOB";
	             break;
	        case 10:
		         detector = FeatureDetector.create(FeatureDetector.DENSE);
		         str_detect = "DENSE";
	             break;
	        case 11:
	        	 detector = FeatureDetector.create(FeatureDetector.BRISK);
	        	 str_detect = "BRISK";
	             break;
	        case 12:
		         detector = FeatureDetector.create(FeatureDetector.GRIDRETECTOR);
		         str_detect = "GRIDRETECTOR";
	             break;
	        default:
	        	 detector = FeatureDetector.create(FeatureDetector.FAST);
	        	 str_detect = "FAST";
	             break;
	   }
	   

	   MatOfKeyPoint keypoints_object = new MatOfKeyPoint();
	   MatOfKeyPoint keypoints_scene  = new MatOfKeyPoint();

	   detector.detect(img_object, keypoints_object);
	   detector.detect(img_scene, keypoints_scene);

	   KeyPoint[] obj_item = keypoints_object.toArray();
	   KeyPoint[] obj_item0 = keypoints_scene.toArray();
	   
	   length = obj_item0.length;
	   
	   Log.i("Feature", "  obj_item.length = "+length+", "+obj_item0.length); 
	   
	   for( int i = 0; i < length; i++ ) {
	       //-- Get the keypoints from the good matches
	       Point pts = obj_item0[i].pt;
	       Core.rectangle(img_scene, 
	    		   new Point(pts.x-5, pts.y-5),
	    		   new Point(pts.x+5, pts.y+5),
	    		   new Scalar(0, 255, detect_type*20),  
	    		   2);
	       //scene.add(obj_item0[dgood_matches.get(i).trainIdx].pt);
	   }
	   
	   bt3 = Bitmap.createBitmap(img_scene.cols(), img_scene.rows(), Config.RGB_565);
       Utils.matToBitmap(img_scene, bt3);
       //imageView0.setImageBitmap(bt3);

       
	   Log.w("Feature","     FeatureDetector    \n"); 
	   return 1;
   }
   
   public void FeatureT0()
   {
	   int length;
	   //Utils.bitmapToMat(bmp, objMat);
	   //Utils.bitmapToMat(bmps, sceneMat);
	   
	   Mat img_object = objMat;
	   Mat img_scene = sceneMat;

	   //-- Step 1: Detect the keypoints using SURF Detector
	   FeatureDetector detector;
	   detector = FeatureDetector.create(FeatureDetector.ORB);

	   MatOfKeyPoint keypoints_object = new MatOfKeyPoint();
	   MatOfKeyPoint keypoints_scene  = new MatOfKeyPoint();

	   detector.detect(img_object, keypoints_object);
	   detector.detect(img_scene, keypoints_scene);

	   Log.w("Feature","     FeatureDetector    \n");
	   //-- Step 2: Calculate descriptors (feature vectors)
	   
	   DescriptorExtractor extractor;
	   extractor = DescriptorExtractor.create(DescriptorExtractor.ORB);
	   
	   Mat descriptors_object = new Mat();
	   Mat descriptors_scene = new Mat();

	   extractor.compute(img_object, keypoints_object, descriptors_object); 
	   extractor.compute(img_scene, keypoints_scene, descriptors_scene);

	   Log.w("Feature","     DescriptorExtractor    \n");

	   //-- Step 3: Matching descriptor vectors using FLANN matcher
	   DescriptorMatcher matcher;
	   matcher = DescriptorMatcher.create(DescriptorMatcher.BRUTEFORCE);
	   
	   MatOfDMatch matches = new MatOfDMatch();
	   matcher.match(descriptors_object, descriptors_scene, matches);

	   Log.w("Feature","     DescriptorMatcher    \n");
	   double max_dist = 0; double min_dist = 100;

	   DMatch[] dmatch;
	   dmatch = matches.toArray();
	   
	   Log.w("Feature","     DescriptorMatcher  =  "+dmatch.length+",  "+matches.height());
	   
	   //-- Quick calculation of max and min distances between keypoints
	   for( int i = 0; i < descriptors_object.rows(); i++ )//rows 
	   { 
		   double dist = dmatch[i].distance; //.distance()
	       if( dist < min_dist ) min_dist = dist;
	       if( dist > max_dist ) max_dist = dist;
	   }

	   Log.w("Feature","-- Max dist : "+max_dist );
	   Log.w("Feature","-- Min dist : "+min_dist );

	   //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	   MatOfDMatch good_matches = new MatOfDMatch();
	   List<DMatch>  dgood_matches = new ArrayList<DMatch>();
	   
	   length = 0;
	   for(int i=0; i < descriptors_object.rows(); i++){  
		  if( dmatch[i].distance < 3*min_dist ){ 
		     dgood_matches.add(dmatch[i]); 
		     length++;
		  }
	   }
	   good_matches.fromList(dgood_matches);
	   Log.i("Feature", " good_matches "+good_matches.height()+", "+dgood_matches.size()); 
	   
	   
	   Mat img_matches = new Mat();
	   //public static void drawMatches(Mat img1, MatOfKeyPoint keypoints1, 
	   //          Mat img2, MatOfKeyPoint keypoints2, MatOfDMatch matches1to2, 
	   //          Mat outImg, Scalar matchColor, Scalar singlePointColor, 
	   //          MatOfByte matchesMask, int flags)
	   
	   Features2d.drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
	                good_matches, img_matches, Scalar.all(-1), Scalar.all(-1),
	                null, Features2d.NOT_DRAW_SINGLE_POINTS); //new MatOfByte()  NOT_DRAW_SINGLE_POINTS = 2,
	   
	   Log.i("Feature", " drawMatches "+length+", "+good_matches.height()); 

	   KeyPoint[] obj_item = keypoints_object.toArray();
	   KeyPoint[] obj_item0 = keypoints_scene.toArray();
	   
	   List<Point> obj = new ArrayList<Point>();
	   List<Point> scene = new ArrayList<Point>();

	   length = dgood_matches.size();
	   
	   Log.i("Feature", " drawMatches 00 = "+length+", "+obj_item.length+", "+obj_item0.length); 
	   
	   for( int i = 0; i < length; i++ ) {
	       //-- Get the keypoints from the good matches
		   Log.i("Feature", " drawMatches  = "+i+", queryIdx = "+dgood_matches.get(i).queryIdx+", trainIdx = "+dgood_matches.get(i).trainIdx); 
		   
	       obj.add(obj_item[dgood_matches.get(i).queryIdx].pt);
	       scene.add(obj_item[dgood_matches.get(i).queryIdx].pt);
	       
	       Core.rectangle(img_matches, 
	    		   new Point(obj_item[dgood_matches.get(i).queryIdx].pt.x-5, obj_item[dgood_matches.get(i).queryIdx].pt.y-5),
	    		   new Point(obj_item[dgood_matches.get(i).queryIdx].pt.x+5, obj_item[dgood_matches.get(i).queryIdx].pt.y+5),
	    		   new Scalar(0, 255, 0),  
	    		   3);
	       //scene.add(obj_item0[dgood_matches.get(i).trainIdx].pt);
	   }

	   /*
	   //-- Localize the object
	   MatOfPoint2f objs = new MatOfPoint2f();
	   MatOfPoint2f scenes = new MatOfPoint2f();
	   
	   objs.fromList(obj);
	   objs.fromList(scene);
	   Log.i("Feature", " findHomography  before! ");
	   Mat H = Calib3d.findHomography(objs, scenes);//, Calib3d.CV_RANSAC );

	   Log.i("Feature", " findHomography "); 
	   
	   Core.perspectiveTransform(img_matches, img_matches, H);
	   Log.i("Feature", " perspectiveTransform "); 
	   */
	   
	   /*
	   //-- Draw lines between the corners (the mapped object in the scene - image_2 )
	   line( img_matches, 
			   scene_corners[0] + Point2f( img_object.cols(), 0), 
			   scene_corners[1] + Point2f( img_object.cols(), 0), 
			   new Scalar(0, 255, 0), 
			   4 );
	   line( img_matches, 
			   scene_corners[1] + Point2f( img_object.cols(), 0), 
			   scene_corners[2] + Point2f( img_object.cols(), 0), 
			   Scalar( 0, 255, 0), 
			   4 );
	   line( img_matches, 
			   scene_corners[2] + Point2f( img_object.cols(), 0), 
			   scene_corners[3] + Point2f( img_object.cols(), 0),
			   Scalar( 0, 255, 0), 
			   4 );
	   line( img_matches, 
			   scene_corners[3] + Point2f( img_object.cols(), 0), 
			   scene_corners[0] + Point2f( img_object.cols(), 0), 
			   Scalar( 0, 255, 0), 
			   4 );
       */
	   Bitmap bt3 = Bitmap.createBitmap(img_matches.cols(), img_matches.rows(), Config.RGB_565);
       Utils.matToBitmap(img_matches, bt3);
		  
       imageView.setImageBitmap(bt3);
	   //objMatch = img_matches;
   }
   
   boolean click0Swap=false;
   
   public void click0(View view){
	   System.out.println("entering the jni");
	   
	   // 转换数据
	   imageView.setVisibility(View.VISIBLE);
	   imageViews.setVisibility(View.VISIBLE);

	   /*
	   new Thread(){
		   public void run(){
	          int ret =  FeatureT();
	          if(ret==1){
	             Message message = new Message();   
                 message.what = 5;     
                 myHandler.sendMessage(message);
	          }
		   }
	   }.start();
	   */
	   
	   click0Swap = !click0Swap;
	   
	   if(click0Swap){
		   imageViews.setImageBitmap(bmps);
		   imageView.setImageBitmap(bmp);
		   
		   Utils.bitmapToMat(bmp, objMat);
		   Utils.bitmapToMat(bmps, sceneMat);
		   
		   // System.out.println(Environment.getExternalStorageState());
		   new Thread(){
			   public void run(){
		          //long addr = CarPlateDetection.featureTest(objMat.getNativeObjAddr(), sceneMat.getNativeObjAddr());
		          //sceneMat = new Mat(addr);
				  FeatureT();
				  System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
			   }
		   }.start();
	   
	   }else{
		   imageViews.setImageBitmap(bmp0);
		   imageView.setImageBitmap(bmp);
		   
		   Utils.bitmapToMat(bmp, objMat);
		   Utils.bitmapToMat(bmp0, sceneMat);
		   
		   // System.out.println(Environment.getExternalStorageState());
		   new Thread(){
			   public void run(){
		          long addr = CarPlateDetection.doHaarClassifier(sceneMat.getNativeObjAddr());
		          sceneMat = new Mat(addr);
				  System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
		          Message message = new Message();   
	              message.what = 4;     
	              myHandler.sendMessage(message);
			   }
		   }.start();
	   }
	   /*
	   imageViews.setImageBitmap(bmp0);
	   imageView.setImageBitmap(bmp);
	   
	   Utils.bitmapToMat(bmp, objMat);
	   Utils.bitmapToMat(bmp0, sceneMat);
	   
	   // System.out.println(Environment.getExternalStorageState());
	   new Thread(){
		   public void run(){
	          long addr = CarPlateDetection.doImageShow(sceneMat.getNativeObjAddr());
	          sceneMat = new Mat(addr);
			  System.out.println("      sceneMat  "+sceneMat.cols()+", "+sceneMat.rows());
	          Message message = new Message();   
              message.what = 4;     
              myHandler.sendMessage(message);
		   }
	   }.start();
	   */
   }
   
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		  //通过OpenCV引擎服务加载并初始化OpenCV类库，所谓OpenCV引擎服务即是  
       //OpenCV_2.4.3.2_Manager_2.4_*.apk程序包，存在于OpenCV安装包的apk目录中  
       OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);  
	}
	
	@Override
	protected void onStart(){
		 super.onStart();
	}
}
