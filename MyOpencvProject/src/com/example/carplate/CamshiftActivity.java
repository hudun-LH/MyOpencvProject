package com.example.carplate;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import org.opencv.imgproc.Imgproc;

public class CamshiftActivity extends Activity implements SurfaceHolder.Callback, PreviewCallback {

	//private int width=320, height=240;
	private int width=640, height=480;
	
	private byte[] buf;
	private Surface surface;
	private SurfaceView surfaceView;
	SurfaceHolder mSurfaceHolder;
	Camera camera = null;
	MediaCodec mediaCodec, mediaCodecd;
	
	byte keyFrame;
	boolean mp4fFlag;
	private byte[] m_info = null; 
	private byte[] yuv420 = new byte[width*height*3/2];
	private byte[] h264 = new byte[width*height*3/2]; 
	
	private BufferedOutputStream outputStream;
	
	BufferedOutputStream  h264outputStream;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		/*
		surfaceView = new SurfaceView(this);
		SurfaceHolder mSurfaceHolder = surfaceView.getHolder();
		//mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);//translucent閿熸枻鎷烽�忛敓鏂ゆ嫹 transparent閫忛敓鏂ゆ嫹  
		mSurfaceHolder.addCallback(this);
		surface = surfaceView.getHolder().getSurface();
		//mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		//getActionBar().hide();
		setContentView(surfaceView);
        */
		
		// requesting to turn the title OFF
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        // making it full screen
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.activity_streamer);
		
        surfaceView = (SurfaceView) findViewById(R.id.mySurfaceView);
        mSurfaceHolder = surfaceView.getHolder();
		//mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);//translucent閸楀﹪锟藉繑妲� transparent闁繑妲�  
        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		mSurfaceHolder.addCallback(this);
		surface = surfaceView.getHolder().getSurface();
		
        Button showButton = (Button) findViewById(R.id.btnShow);
        showButton.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                    	Log.i("Encoder", "--------------showButton--------------");
                    	/*
                    	Message message = new Message();   
                        message.what = 0;     
                        myHandler.sendMessage(message); 
                        */
                    }
                }
        );
	}
	
	public DisplayMetrics getDM(Context context) {
		WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
		Display display = windowManager.getDefaultDisplay();
		DisplayMetrics outMetrics = new DisplayMetrics();
		display.getMetrics(outMetrics);
		return outMetrics;
	}
	
	boolean yuv_update = true;
	Handler myHandler = new Handler() {  
        public void handleMessage(Message msg) {   
             switch (msg.what) {   
                  case 0:   
                	  Log.i("Encoder", "--------------myHandler--------------");
                      //preview1.addView(surfaceView);
                      break;   
                  case 1:
                	  break;
                  case 2:
                	  openCamera(mSurfaceHolder);
                	  break;
             }   
             super.handleMessage(msg);   
        }   
    }; 
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i("Encoder", "--------------surfaceCreated--------------");
		MediaCodecEncodeInit();
		//MediaCodecDecodeInit();
		
		openCamera(holder);

		 /*
		 new Thread() {
	        public void run() {
					try {
						Thread.sleep(2000);
						//Mp4FpgDCE();
						Message message = new Message();   
		                message.what = 2;     
		                myHandler.sendMessage(message);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
	        }
	     }.start();
	     */
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		releaseCamera();
		close();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
	    if (keyCode == KeyEvent.KEYCODE_BACK) {
	      //do something here
	      finish();   
          //System.exit(0); 
	      return true;
	    }
	    return super.onKeyDown(keyCode, event);
	} 
	
	private Camera getCamera(int cameraType) {
	    Camera camera = null;
	    try {
	    	Log.i("Encoder", "--------------getCamera start--------------");
	        camera = Camera.open(Camera.getNumberOfCameras()-1);
	        Log.i("Encoder", "--------------getCamera end--------------");	        
	        //printSupportPreviewSize(camera.getParameters());
	    } catch (Exception e) {
	        e.printStackTrace();
	    }
	    return camera; // returns null if camera is unavailable
	}
	
	@SuppressWarnings("deprecation")
	private void openCamera(SurfaceHolder holder) {
	    releaseCamera();
	    try {
	            camera = getCamera(Camera.CameraInfo.CAMERA_FACING_BACK); 
	        } catch (Exception e) {
	            camera = null;
	            e.printStackTrace();
	        }
	    if(camera != null){
	        try {
				camera.setPreviewDisplay(holder);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			camera.setDisplayOrientation(90); 			
			Camera.Parameters parameters = camera.getParameters();
			
			parameters.setPreviewSize(width, height);
			//parameters.getSupportedPreviewSizes();
			parameters.setFlashMode("off");
			parameters.setWhiteBalance(Camera.Parameters.WHITE_BALANCE_AUTO); 
			parameters.setPreviewFormat(ImageFormat.YV12); // YV12
			parameters.setSceneMode(Camera.Parameters.SCENE_MODE_AUTO);  
			//parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO); 
			//parameters.set("orientation", "portrait");
			//parameters.set("orientation", "landscape");
			camera.setParameters(parameters);
         
			
            /*
			 camera.setPreviewDisplay(holder);  
			 Camera.Parameters parameters = camera.getParameters();  
			 parameters.setPreviewSize(width, height);  
			 // parameters.setPictureSize(width, height);  
			 parameters.setPreviewFormat(ImageFormat.YV12);  
			 camera.setParameters(parameters);   
			 camera.setPreviewCallback(this);  
			 camera.startPreview();  
         */
			
			buf = new byte[width*height*3/2];
			camera.addCallbackBuffer(buf);
			camera.setPreviewCallbackWithBuffer(this);
			
			List<int[]> fpsRange = parameters.getSupportedPreviewFpsRange();
			for (int[] temp3 : fpsRange) {
			     System.out.println(Arrays.toString(temp3));
			}
			
			//parameters.setPreviewFpsRange(10000, 30000);    
			parameters.setPreviewFpsRange(15000, 15000);
			//parameters.setPreviewFpsRange(4000,60000);//this one results fast playback when I use the FRONT CAMERA 
			
			camera.startPreview();
			
			Log.i("Encoder", "width = "+width+", height = "+height);
			
			Log.i("Encoder", "--------------openCamera--------------");
	    }
	}
	
	private synchronized void releaseCamera() {
	    if (camera != null) {
	        try {
	            camera.setPreviewCallback(null);
	            camera.stopPreview();
	            camera.release();
	            camera = null;
	            Log.i("Encoder", "--------------releaseCamera--------------");
	        } catch (Exception e) {
	            e.printStackTrace();
	        }
	    }
	}

    /**
     * Returns a color format that is supported by the codec and by this test code.  If no
     * match is found, this throws a test failure -- the set of formats known to the test
     * should be expanded for new platforms.
     */
    private static int selectColorFormat(MediaCodecInfo codecInfo, String mimeType) {
        MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType(mimeType);
        for (int i = 0; i < capabilities.colorFormats.length; i++) {
            int colorFormat = capabilities.colorFormats[i];
            if (isRecognizedFormat(colorFormat)) {
            	Log.i("Encoder", "selectColorFormat = "+colorFormat);
                return colorFormat;
            }
        }
        Log.e("Encoder","couldn't find a good color format for " + codecInfo.getName() + " / " + mimeType);
        return 0;   // not reached
    }

    /**
     * Returns true if this is a color format that this test code understands (i.e. we know how
     * to read and generate frames in this format).
     */
    private static boolean isRecognizedFormat(int colorFormat) {
        switch (colorFormat) {
            // these are the formats we know how to handle for this test
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar:
                return true;
            default:
                return false;
        }
    }
    
    /**
     * Returns the first codec capable of encoding the specified MIME type, or null if no
     * match was found.
     */
	private static MediaCodecInfo selectCodec(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);

            if (!codecInfo.isEncoder()) {
                continue;
            }

            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                	Log.i("Encoder", "selectCodec = "+codecInfo.getName());
                    return codecInfo;
                }
            }
        }
        return null;
    }
    
	public void MediaCodecEncodeInit(){
		String type = "video/avc";
		
		File f = new File(Environment.getExternalStorageDirectory(), "mediacodec_r0.264");
	    if(!f.exists()){	
	    	try {
				f.createNewFile();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	    }
	    try {
	        outputStream = new BufferedOutputStream(new FileOutputStream(f));
	        Log.i("Encoder", "outputStream initialized");
	    } catch (Exception e){ 
	        e.printStackTrace();
	    }
	    
	    int colorFormat = selectColorFormat(selectCodec("video/avc"), "video/avc");
		try {
			mediaCodec = MediaCodec.createEncoderByType(type);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}  
		MediaFormat mediaFormat = MediaFormat.createVideoFormat(type, width, height);  
		mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 250000);//125kbps  
		mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 15);  
		mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, colorFormat);
		//mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, 
		//		MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);  //COLOR_FormatYUV420Planar
		mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5); 
		mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);  
		mediaCodec.start();	
		Log.i("Encoder", "--------------MediaCodecEncodeInit--------------");
	}
	
	
	public void MediaCodecDecodeInit(){
		String type = "video/avc";
		try {
			mediaCodecd = MediaCodec.createDecoderByType(type);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}  
		MediaFormat mediaFormat = MediaFormat.createVideoFormat(type, width, height);  
		mediaCodecd.configure(mediaFormat, surface, null, 0);  
		mediaCodecd.start(); 
	}	
	
	
	public void close() {
	    try {
	        mediaCodec.stop();
	        mediaCodec.release();
	        outputStream.flush();
	        outputStream.close();
	        Log.i("Encoder", "--------------close--------------");
	    } catch (Exception e){ 
	        e.printStackTrace();
	    }
	}

	 //yv12 杞� yuv420p  yvu -> yuv  
    private void swapYV12toI420(byte[] yv12bytes, byte[] i420bytes, int width, int height)   
    {        
        System.arraycopy(yv12bytes, 0, i420bytes, 0,width*height);  
        System.arraycopy(yv12bytes, width*height+width*height/4, i420bytes, width*height,width*height/4);  
        System.arraycopy(yv12bytes, width*height, i420bytes, width*height+width*height/4,width*height/4);    
    } 
    
    public int offerEncoder(byte[] input, byte[] output)   
    {     
        int pos = 0;
        keyFrame = 0;
        swapYV12toI420(input, output, width, height);  
        //YV12toYUV420PackedSemiPlanar_00(input, output, width, height);
        
        try {  
            ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();  
            ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();  
            int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);  
            if (inputBufferIndex >= 0)   
            {  
                ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];  
                inputBuffer.clear();  
                inputBuffer.put(output);  //yuv420
                mediaCodec.queueInputBuffer(inputBufferIndex, 0, output.length, 0, 0); // yuv420.length 
            }  
  
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();  
            int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo,0);  
             
            while (outputBufferIndex >= 0)   
            {  
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];  
                byte[] outData = new byte[bufferInfo.size];  
                outputBuffer.get(outData);  
                  
                if(m_info != null)  
                {                 
                    System.arraycopy(outData, 0,  output, pos, outData.length);  
                    pos += outData.length;  
                      
                }  
                  
                else {  
                     ByteBuffer spsPpsBuffer = ByteBuffer.wrap(outData);    
                     if (spsPpsBuffer.getInt() == 0x00000001)   
                     {   
                    	 Log.i("Encoder", "--------- pps sps found = "+outData.length+"---------");
                         m_info = new byte[outData.length];  
                         System.arraycopy(outData, 0, m_info, 0, outData.length); 
                         
                         int length = outData.length;
                         for (int ix = 0; ix < length; ++ix) {
                 			System.out.printf("%02x ", outData[ix]);
                 		 }
                 		 System.out.println("\n----------");
                 		 pos += outData.length;  
                     }   
                     else   
                     {    
                    	 Log.i("Encoder", "--------- no pps sps detect---------");
                         return -1;  
                     }        
                }  
                  
                mediaCodec.releaseOutputBuffer(outputBufferIndex, false);  
                outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);  
            }  
  
            if(output[4] == 0x65)
            {  
            	Log.i("Encoder", "-----------idr frame: "+output[4]+"-----------");
                System.arraycopy(output, 0,  yuv420, 0, pos);  
                System.arraycopy(m_info, 0,  output, 0, m_info.length);  
                System.arraycopy(yuv420, 0,  output, m_info.length, pos);  
                pos += m_info.length;  
                
                keyFrame = 1;
            }
            
              
        } catch (Throwable t) {  
            t.printStackTrace();  
        }  
  
        return pos;  
    }  
    
	// encode
	 public void onFrame(byte[] buf, int offset, int length, int flag) {	
		 
		    //YV12toYUV420PackedSemiPlanar_00(buf, h264, width, height);
		    swapYV12toI420(buf, h264, width, height); 
		    
		    //Mp4CE(h264, 0);
		    /*
		    if(isPlaying){	
 		    	if(onFraming>50){
 	 		       onFraming = 0;
 	 		       Log.i("Encoder", "--------------onFrame go--------------");
 	 		    }
 		    	copyFrom(h264,width,height);
 		    	Message message = new Message();   
                message.what = 1;     
                myHandler.sendMessage(message);
 		    }else{
 		    	onFraming++;
 	 		    if(onFraming>50){
 	 		       onFraming = 0;
 	 		       Log.i("Encoder", "--------------onFrame--------------");
 	 		    }
 		    }
		    */
		    
		    ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
		    ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
		    int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);
		    if (inputBufferIndex >= 0) {
		        ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
		        inputBuffer.clear();
		        //inputBuffer.put(buf, offset, length);
		        inputBuffer.put(h264, offset, length);
		        mediaCodec.queueInputBuffer(inputBufferIndex, 0, length, 0, 0);
		    }
		    MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
		    int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo,0);
		    while (outputBufferIndex >= 0) {
		        ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];

	            byte[] outData = new byte[bufferInfo.size];
	            outputBuffer.get(outData);
	            
	            /*
	            Log.i("Encoder", "onFrame   outData.length = "+outData.length);
	            if(outData.length==20){
	               for (int ix = 0; ix < 20; ++ix) {
             		  System.out.printf("%02x ", outData[ix]);
             		  //10-11 09:11:36.670: I/System.out(24316): 00 00 00 01 67 42 80 0c e9 02 83 f2 00 00 00 01 68 ce 06 e2 
             	   }
             	   System.out.println("\n----------");
	            }
	            */
	            keyFrame = 0;
	            //mp4fFlag = false;
	            if(mp4fFlag){
	            	  if(outData.length==20){ //21
		    	    	   Log.i("Encoder", " pps sps set = "+outData.length);
		    	    	   //int length = outData.length;
	                       for (int ix = 0; ix < 20; ++ix) {
	                 			System.out.printf("%02x ", outData[ix]);
	                 	   }
	                 	   System.out.println("\n----------");
                            //00 00 00 01 67 42 80 1e e9 01 40 7b 20 00 00 00 01 68 ce 06 e2 
	                 		 
		    	    	   //byte[] outData0 = new byte[13]; 
		    	    	   //byte[] outData1 = new byte[8]; 
	                 	   //System.arraycopy(outData, 0,  outData0, 0, 13);  
		                   //System.arraycopy(outData, 13, outData1, 0, 8); 
		    	    	   byte[] outData0 = new byte[12]; 
		    	    	   byte[] outData1 = new byte[8]; 
		                   System.arraycopy(outData, 0,  outData0, 0, 12);  
		                   System.arraycopy(outData, 12, outData1, 0, 8); 
			               //mp4packVideo(outData0, 13, keyFrame);
			               //mp4packVideo(outData1, 8, keyFrame); 
		                   
		                   //Mp4PackV(outData0, 13, keyFrame);
		                   //Mp4PackV(outData1, 8, keyFrame);
		                   
		                   //Mp4FpgDCE0(outData0, 12);
		                   //Mp4FpgDCE0(outData1, 8);
		    	      }else{
		    	    	   if(outData[4] == 0x65) //key frame   閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熺即鍏崇》鎷峰抚鏃跺彧閿熸枻鎷� 00 00 00 01 65 娌￠敓鏂ゆ嫹pps sps閿熸枻鎷� 瑕侀敓鏂ゆ嫹閿熸枻鎷�  
		    	           { 
		    	    		   keyFrame = 1;
		    	    		   // Log.i("Encoder", "--------- key frame---------");
		    	           }
		    	    	   //Mp4PackV(outData, outData.length, keyFrame);
		    	    	   
		    	    	   //Mp4FpgDCE0(outData, outData.length);
	                       //mp4packVideo(outData, outData.length, keyFrame);
		    	      }
	            }
	            /*
	            try {
					outputStream.write(outData, 0, outData.length);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				*/ 
				// write into h264 file
	            //Log.i("Encoder", outData.length + " bytes written");
                
	            
		        //if (frameListener != null)//鍙栧帇閿熸枻鎷烽敓鐭鎷烽敓鏂ゆ嫹閿熸枻鎷峰杺閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
		        //    frameListener.onFrame(outputBuffer, 0, length, flag);
		        
	            //onFrame0(outData, 0, outData.length, flag);
	            
		        //onFrame0(outputBuffer.array(), 0, bufferInfo.size, flag);
		        
		        mediaCodec.releaseOutputBuffer(outputBufferIndex, false);
		        outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
		    }
	 }  
	 
	 private int mCount;
	 private final static int FRAME_RATE = 15;
	 // decoder
	 public void onFrame0(byte[] buf, int offset, int length, int flag) {  
	        ByteBuffer[] inputBuffers = mediaCodecd.getInputBuffers();  
	        int inputBufferIndex = mediaCodecd.dequeueInputBuffer(-1);  
	        if (inputBufferIndex >= 0) {  
	            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];  
	            inputBuffer.clear();  
	            inputBuffer.put(buf, offset, length);  
	            mediaCodecd.queueInputBuffer(inputBufferIndex, 0, length,
	            		mCount * 1000000 / FRAME_RATE, 0);  
	            mCount++;  
	        }  
	  
	       MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();  
	       int outputBufferIndex = mediaCodecd.dequeueOutputBuffer(bufferInfo,0);  
	       while (outputBufferIndex >= 0) {  
	    	   /*
	    	   ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];  
	    	   byte[] outData = new byte[bufferInfo.size + 3];  
	    		        outputBuffer.get(outData, 3, bufferInfo.size);  
	    	   if (frameListener != null) {  
	    		     if ((outData[3]==0 && outData[4]==0 && outData[5]==1)  
	    		     || (outData[3]==0 && outData[4]==0 && outData[5]==0 && outData[6]==1))  
	    		     {  
	    		         frameListener.onFrame(outData, 3, outData.length-3, bufferInfo.flags);  
	    		     }  
	    		     else  
	    		     {  
	    		      outData[0] = 0;  
	    		      outData[1] = 0;  
	    		      outData[2] = 1;  
	    		         frameListener.onFrame(outData, 0, outData.length, bufferInfo.flags);  
	    		     }  
	    		} 
	    		 */
	           mediaCodecd.releaseOutputBuffer(outputBufferIndex, true);  
	           outputBufferIndex = mediaCodecd.dequeueOutputBuffer(bufferInfo, 0);  
	       }  
	}
	
	boolean firstFlag = true;
	int yuv_size = width*height*3/2;
			
	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		// TODO Auto-generated method stub
		 //if (frameListener != null) {  
		 //       frameListener.onFrame(data, 0, data.length, 0);  
		 //} 
		 /*
		 int ret = offerEncoder(data,h264);  
         
	     if(ret > 0)  
	     {   	 
	    	 //try {
	    	    if(mp4fFlag){
	    	       if(firstFlag){
	    	    	  firstFlag = false;
	    	    	  Log.i("Encoder", "first frame = "+ret);
	    	       }
	    	       if(ret==21){
	    	    	   Log.i("Encoder", "--------- pps sps set---------");
	    	    	   byte[] outData0 = new byte[13]; 
	    	    	   byte[] outData1 = new byte[8]; 
	                   System.arraycopy(h264, 0,  outData0, 0, 13);  
	                   System.arraycopy(h264, 13, outData1, 0, 8); 
		               mp4packVideo(outData0, 13, keyFrame);
		               mp4packVideo(outData1, 8, keyFrame); 
	    	       }else{
	    	    	   mp4packVideo(h264, ret, keyFrame); 
	    	       }
		        }
	    	    
	    	    
				//outputStream.write(h264, 0, ret);
			 } catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			 } // write into h264 file
	         //Log.i("Encoder", ret + " bytes written");
	          
	     }
	     */
		 swapYV12toI420(data, h264, width, height); 
		 //h264outputStream.write(h264, 0, yuv_size);
		 //onFrame(data, 0, data.length, 0); 
	     
		 camera.addCallbackBuffer(buf);	
		 //Log.i("Encoder", "--------------------onPreviewFrame-----------------"+data.length);
	}

	
}
