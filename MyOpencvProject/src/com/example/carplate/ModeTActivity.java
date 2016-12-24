package com.example.carplate;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

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
import android.app.IntentService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class ModeTActivity extends Activity {
	private static ImageView imageView = null;  
	private static ImageView imageViews = null;  
	private Bitmap bmp = null;  
	private static Bitmap bmps = null; 
    Bitmap bmp0 = null; 
    Bitmap bmp_one = null;  
	Bitmap bmp_two = null;
		
	Mat objMat;
	Mat sceneMat;
	Mat objMatch;
	   
    public static final String UPLOAD_RESULT = "UPLOAD_RESULT";
	private LinearLayout mLyTaskContainer;
	
	private static TextView m_text = null;
	private String path = null; //SDCARD 根目录
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.activity_main0);
		imageViews = (ImageView) findViewById(R.id.image_view);  
		imageView = (ImageView) findViewById(R.id.image_view0);  

	    //将汽车完整图像加载程序中并进行显示
	    bmps = BitmapFactory.decodeResource(getResources(), R.drawable.test);  
	    //imageViews.setImageBitmap(bmps);
	     
	    bmp = BitmapFactory.decodeResource(getResources(), R.drawable.test00);  
	    //imageView0.setImageBitmap(bmp);
	     
	    bmp0 = BitmapFactory.decodeResource(getResources(), R.drawable.people);   
	    
	    mLyTaskContainer = (LinearLayout) findViewById(R.id.id_ll_taskcontainer);

        registerReceiver();
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
			          break;
		        case 1:
		        	  m_text.setText(result);
		        	  System.out.println(" m_text.setText = "+result);
		        	  break;
		        case 2:
		        	  //objMatch
		        	  System.out.println("    2!   ");
		        	  break;
		        case 3:
		        	  //objMatch
		        	  break;
		        case 4:
		        	  System.out.println("              myHandler -- 4            ");
			          break;
		        default:
		        	  break;
		   }
	   }
   };
   
   public void click0(View view){
	   //ExecutorService executorService = Executors.newSingleThreadExecutor();
       /*
	   ExecutorService executorService = Executors.newSingleThreadExecutor();
       for (int i = 0; i < 20; i++) {
           Runnable syncRunnable = new Runnable() {
               @Override
               public void run() {
                   Log.e("Executors", Thread.currentThread().getName());
               }
           };
           executorService.execute(syncRunnable);
       }
       ExecutorService executorService = Executors.newFixedThreadPool(5);
       for (int i = 0; i < 20; i++) {
            Runnable syncRunnable = new Runnable() {
                @Override
                public void run() {
                    Log.e(TAG, Thread.currentThread().getName());
                }
            };
       }
       executorService.execute(syncRunnable);
            
       ScheduledExecutorService executorService = Executors.newScheduledThreadPool(5);
       Runnable syncRunnable = new Runnable() {
            @Override
            public void run() {
                Log.e(TAG, Thread.currentThread().getName());
            }
       };
       executorService.schedule(syncRunnable, 5000, TimeUnit.MILLISECONDS);
       executorService.scheduleAtFixedRate(syncRunnable, 5000, 3000, TimeUnit.MILLISECONDS);
       executorService.scheduleWithFixedDelay(syncRunnable, 5000, 3000, TimeUnit.MILLISECONDS); 
       */
       ExecutorService executorService = Executors.newCachedThreadPool();
       for (int i = 0; i < 100; i++) {
           Runnable syncRunnable = new Runnable() {
               @Override
               public void run() {
                   Log.e("Executors", Thread.currentThread().getName());
               }
           };
           executorService.execute(syncRunnable);
       }
       
	   Future future = executorService.submit(new Runnable() {
		    public void run() {
		        System.out.println("Asynchronous task");
		    }
	   });
	   //如果任务结束执行则返回 null
	   try {
		   System.out.println("Runnable future.get()=" + future.get());
	   } catch (InterruptedException e3) {
		   // TODO Auto-generated catch block
		   e3.printStackTrace();
	   } catch (ExecutionException e3) {
		   // TODO Auto-generated catch block
		   e3.printStackTrace();
	   }
		
	   Future future0 = executorService.submit(new Callable(){
		    public Object call() throws Exception {
		        System.out.println("Asynchronous Callable");
		        return "Callable Result";
		    }
		});

		try {
			System.out.println("Callable future.get() = " + future0.get());
		} catch (InterruptedException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		} catch (ExecutionException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
		
	   Set<Callable<String>> callables = new HashSet<Callable<String>>();

	   callables.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 1";
	       }
	   });
	   callables.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 2";
	       }
	   });
	   callables.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 3";
	       }
	   });

	   String result = null;
	   try {
		   result = executorService.invokeAny(callables);
	   } catch (InterruptedException e1) {
		  // TODO Auto-generated catch block
		  e1.printStackTrace();
	  } catch (ExecutionException e1) {
		  // TODO Auto-generated catch block
		  e1.printStackTrace();
	   }

	   System.out.println("result = " + result);
	   executorService.shutdown();
	   executorService = null;
	   
	   // System.out.println(Environment.getExternalStorageState());
	   ExecutorService executorService0 = Executors.newSingleThreadExecutor();

	   Set<Callable<String>> callables0 = new HashSet<Callable<String>>();

	   callables0.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 1";
	       }
	   });
	   callables0.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 2";
	       }
	   });
	   callables0.add(new Callable<String>() {
	       public String call() throws Exception {
	           return "Task 3";
	       }
	   });

	   List<Future<String>> futures0 = null;
	   try {
		  futures0 = executorService0.invokeAll(callables0);
	   } catch (InterruptedException e) {
		  // TODO Auto-generated catch block
		  e.printStackTrace();
	   }

	   for(Future<String> future1 : futures0){
	       try {
			  System.out.println("future.get = " + future1.get());
		   } catch (InterruptedException e) {
			  // TODO Auto-generated catch block
			  e.printStackTrace();
		   } catch (ExecutionException e) {
			  // TODO Auto-generated catch block
			  e.printStackTrace();
		  }
	   }
	   executorService0.shutdown();
	   executorService0 = null;
    }
   
   /*
   public abstract class Volkswagen{
       public abstract void drive();
       public abstract String getName();
   }
   */
   
   //抽象产品类
   public interface Volkswagen{
       void drive();     // == public abstract void
       String getName(); // == public static final String
   } 
   //抽象产品类1
   public abstract class ShangHaiVolkswagen implements Volkswagen{
   }
   //抽象产品类2
   public abstract class FAWVolkswagen implements Volkswagen {
       public abstract void brake();
   }

   public class Passat extends ShangHaiVolkswagen {
       public static final int ID = 0;
       @Override
       public void drive() {
           System.out.println("Passat开出去咯，测试成功");  
       }
       @Override
       public String getName() {
           return "Passat";
       }
   }
   public class Polo extends ShangHaiVolkswagen{
       public static final int ID = 1;
       @Override
       public void drive() {
           System.out.println("Polo开出去咯，测试成功");
       }
       @Override
       public String getName() {
           return "Polo";
       }
   }

   public class Sagitar extends FAWVolkswagen {
       public static final int ID = 2;
       @Override
       public void drive() {
           System.out.println("Sagitar 开出去了，测试成功了");
       }
       @Override
       public String getName() {
           return "Sagitar";
       }
       @Override
       public void brake(){
           System.out.println("Sagitar 刹车挺好的，测试通过了");
       }
   }

   public class Magotan extends FAWVolkswagen{
       public static final int ID = 3;
       @Override
       public void drive() {
           System.out.println("Magotan 开出去咯，测试成功了");
       }
       @Override
       public String getName() {
           return "Magotan";
       }
       @Override
       public void brake(){
           System.out.println("Magotan 刹车挺好的，测试通过");
       }
   }

   //Insurance类
   public abstract class Insurance {
       public abstract String getName();
   }
   public class OneLevellnsurance extends Insurance{
       @Override
       public String getName() {
           return "一级保险";
       }
   }
   public class TwoLevelInsurance extends Insurance {
       @Override
       public String getName() {
           return "二级保险";
       }
   }

   //抽象工厂类
   public abstract class VolkswagenFactory {
       public abstract Volkswagen createVolkswagen(int productID);
       public abstract Insurance bindInsurance();

       public final Volkswagen createVolkswagen(Class <? extends Volkswagen> clazz){
           Volkswagen volkswagen = null;
           try {
               volkswagen = clazz.newInstance();
           } catch (InstantiationException e) {
               e.printStackTrace();
           } catch (IllegalAccessException e) {
               e.printStackTrace();
           }
           return volkswagen;
       };
   }

   //工厂类1
   public class ShangHaiVolkswagenFactory extends VolkswagenFactory {

       public ShangHaiVolkswagen createVolkswagen(int id){
       //子类实现public abstract Volkswagen createVolkswagen(int productID)函数

           ShangHaiVolkswagen volkswagen = null;
           switch(id){
               case Passat.ID:
                   volkswagen = new Passat();
                   break;
               case Polo.ID:
                   volkswagen = new Polo();
                   break;
               default:
                   volkswagen = null;
           }
           return volkswagen;
       }
       @Override
       public Insurance bindInsurance() {  
           return new OneLevellnsurance();
       }
   }
   //工厂类2
   public class FAWVolkswagenFactory extends VolkswagenFactory {

       @Override
       public FAWVolkswagen createVolkswagen(int productID) {
       //子类实现public abstract Volkswagen createVolkswagen(int productID)函数
           FAWVolkswagen volkswagen = null;
           switch(productID){
               case Magotan.ID:
                   volkswagen = new Magotan();
                   break;
               case Sagitar.ID:
                   volkswagen = new Sagitar();
                   break;
               default:
                   volkswagen = null;
           }
           return volkswagen;
       }
       @Override
       public Insurance bindInsurance() {
           return new TwoLevelInsurance();
       }
   }
   /*
   public class Client {
       public static void main(String[] args) {
           System.out.println("开始测试上海大众的车辆");
           ShangHaiVolkswagenFactory factory = new ShangHaiVolkswagenFactory();
           ShangHaiVolkswagen passat = factory.createVolkswagen(Passat.ID);
           passat.drive();

           ShangHaiVolkswagen polo = factory.createVolkswagen(Polo.ID);
           polo.drive();

           System.out.println("开始测试一汽大众的车辆");
           FAWVolkswagenFactory fawFactory = new FAWVolkswagenFactory();

           FAWVolkswagen magotan = fawFactory.createVolkswagen(Magotan.ID);
           magotan.drive();
           magotan.brake();
           FAWVolkswagen sagitar = fawFactory.createVolkswagen(Sagitar.ID);
           sagitar.drive();
           sagitar.brake();
       }
   }
   */
   //public class Client {
   public  void test() 
   {
           System.out.println("开始测试上海大众的车辆");
           ShangHaiVolkswagenFactory factory = new ShangHaiVolkswagenFactory();
           ShangHaiVolkswagen passat = factory.createVolkswagen(Passat.ID);
           passat.drive();

           ShangHaiVolkswagen polo = factory.createVolkswagen(Polo.ID);
           polo.drive();
           Insurance shanghaiInsurance = factory.bindInsurance();
           System.out.println(shanghaiInsurance.getName());

           System.out.println("开始测试一汽大众的车辆");
           FAWVolkswagenFactory fawFactory = new FAWVolkswagenFactory();

           FAWVolkswagen magotan = fawFactory.createVolkswagen(Magotan.ID);
           magotan.drive();
           magotan.brake();
           FAWVolkswagen sagitar = fawFactory.createVolkswagen(Sagitar.ID);
           sagitar.drive();
           sagitar.brake();
           Insurance fawInsurance = fawFactory.bindInsurance();
           System.out.println(fawInsurance.getName());
    }
   
    public void click1(View view){
	    test();
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
	
	public void click2(View view)
    {
        //模拟路径
		int i = 0;
        String path = "/sdcard/imgs/" + (++i) + ".png";
        UploadImgService.startUploadImg(this, path);

        TextView tv = new TextView(this);
        mLyTaskContainer.addView(tv);
        tv.setText(path + " is uploading ...");
        tv.setTag(path);
    }
	
	public static class UploadImgService extends IntentService
	{
	    private static final String ACTION_UPLOAD_IMG = "UPLOAD_IMAGE";
	    public static final String EXTRA_IMG_PATH = "IMG_PATH";

	    public static void startUploadImg(Context context, String path)
	    {
	        Intent intent = new Intent(context, UploadImgService.class);
	        intent.setAction(ACTION_UPLOAD_IMG);
	        intent.putExtra(EXTRA_IMG_PATH, path);
	        context.startService(intent);
	    }

	    public UploadImgService(){
	    	super("UploadImgService");
	    }

	    @Override
	    protected void onHandleIntent(Intent intent){
	        if (intent != null){
	            final String action = intent.getAction();
	            if (ACTION_UPLOAD_IMG.equals(action)){
	                final String path = intent.getStringExtra(EXTRA_IMG_PATH);
	                handleUploadImg(path);
	            }
	        }
	    }

	    private void handleUploadImg(String path)
	    {
	        try{
	            //模拟上传耗时
	            Thread.sleep(1500);
	            Intent intent = new Intent(ModeTActivity.UPLOAD_RESULT);
	            intent.putExtra(EXTRA_IMG_PATH, path);
	            sendBroadcast(intent);
	        } catch (InterruptedException e)
	        {
	            e.printStackTrace();
	        }
	    }

	    @Override
	    public void onCreate()
	    {
	        super.onCreate();
	        Log.e("UploadImgService","onCreate");
	    }

	    @Override
	    public void onDestroy()
	    {
	        super.onDestroy();
	        Log.e("UploadImgService","onDestroy");
	    }
	}
	

	private void handleResult(String path)
    {
        TextView tv = (TextView) mLyTaskContainer.findViewWithTag(path);
        tv.setText(path + " upload success ~~~ ");
    }
	
	private BroadcastReceiver uploadImgReceiver = new BroadcastReceiver()
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            if (intent.getAction() == UPLOAD_RESULT){
                String path = intent.getStringExtra(UploadImgService.EXTRA_IMG_PATH);
                handleResult(path);
            }
        }
    };
    
    private void registerReceiver(){
	     IntentFilter filter = new IntentFilter();
	     filter.addAction(UPLOAD_RESULT);
	     registerReceiver(uploadImgReceiver, filter);
    }
	 
	@Override
	protected void onDestroy(){
		super.onDestroy();
		unregisterReceiver(uploadImgReceiver);
		    
	}
}
