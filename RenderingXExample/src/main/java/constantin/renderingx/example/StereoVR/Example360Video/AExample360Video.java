package constantin.renderingx.example.StereoVR.Example360Video;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyGLSurfaceView;
import constantin.renderingx.core.MyVRLayout;
import constantin.video.core.VideoPlayerSurfaceTexture;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

//See native code (renderer) for documentation
public class AExample360Video extends AppCompatActivity {
    private static final String TAG="AExampleVRRendering";
    public static final int SPHERE_MODE_GVR_EQUIRECTANGULAR=0;
    public static final int SPHERE_MODE_INSTA360_TEST=1;
    public static final int SPHERE_MODE_INSTA360_TEST2=2;
    //Use one of both, either GvrLayout or MyVRLayout
    private static final boolean USE_GVR_LAYOUT=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;
    //Default mode is 0 (test VDDC)
    public static final String KEY_SPHERE_MODE ="KEY_SPHERE_MODE";
    public static final String KEY_VIDEO_FILENAME="KEY_VIDEO_FILENAME";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final GvrApi gvrApi;
        if(USE_GVR_LAYOUT){
            gvrLayout=new GvrLayout(this);
            gvrApi =gvrLayout.getGvrApi();
        }else{
            myVRLayout=new MyVRLayout(this);
            gvrApi=myVRLayout.getGvrApi();
        }
        final Bundle bundle=getIntent().getExtras();
        final int SPHERE_MODE=bundle.getInt(KEY_SPHERE_MODE,0);
        final String VIDEO_FILENAME=bundle.getString(KEY_VIDEO_FILENAME);
        //start initialization
        final MyGLSurfaceView gLView = new MyGLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        //Use one of both ! Default to the player from VideoCore
        final VideoPlayerSurfaceTexture mVideoPlayer=new VideoPlayerSurfaceTexture(this,null,VIDEO_FILENAME);
        //final XVideoPlayerSurfaceTexture mVideoPlayer=new XVideoPlayerSurfaceTexture(this,VIDEO_FILENAME);
        final Renderer360Video renderer =new Renderer360Video(this,mVideoPlayer, gvrApi,false,
                true,SPHERE_MODE);

        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gLView.setPreserveEGLContextOnPause(true);
        if(USE_GVR_LAYOUT){
            setContentView(gvrLayout);
            gvrLayout.setPresentationView(gLView);
        }else{
            setContentView(myVRLayout);
            myVRLayout.setPresentationView(gLView);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(gvrLayout!=null)gvrLayout.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        if(gvrLayout!=null)gvrLayout.onPause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(gvrLayout!=null)gvrLayout.shutdown();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            FullscreenHelper.setImmersiveSticky(this);
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        //Some VR headsets use a clamp to hold the phone in place. This clamp may press against the volume up/down buttons.
        //Here we effectively disable these 2 buttons
        if(event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN || event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){
            return true;
        }
        //if(event.getKeyCode()==KeyEvent.KEYCODE_BACK){
        //    return true;
        //}
        return super.dispatchKeyEvent(event);
    }

}