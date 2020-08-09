package constantin.renderingx.core.views;

import android.content.Context;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.OnLifecycleEvent;

import java.util.Objects;

import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.renderingx.core.xglview.XSurfaceParams;

/**
 * Wrapper around GvrLayout/GLSurfaceView that includes its own rendering interface, created for SuperSync
 * Provides a convenient IRendererSuperSync
 * SuperSync allows developers to build their own Time Warp /other similar latency reducing techniques without
 * Specifying the device as 'Daydream ready'
 */

public class ViewSuperSync extends VRLayout implements XGLSurfaceView.FullscreenRendererWithSurfaceTexture{
    private static final String TAG="ViewSuperSync";
    private final XGLSurfaceView mGLSurfaceView;
    private IRendererSuperSync mRenderer;

    private final long choreographerVsyncOffsetNS;
    private Context context;

    public ViewSuperSync(Context context,SurfaceTextureHolder.ISurfaceTextureAvailable iSurfaceTextureAvailable){
        super(context);
        this.context=context;
        //getUiLayout().setTransitionViewEnabled(false);
        //setAsyncReprojectionEnabled(false);
        mGLSurfaceView =new XGLSurfaceView(context);
        mGLSurfaceView.setEGLConfigPrams(new XSurfaceParams(0,0,true));
        mGLSurfaceView.setRenderer(this,iSurfaceTextureAvailable);
        mGLSurfaceView.DO_SUPERSYNC_MODS=true;

        setPresentationView(mGLSurfaceView);
        //
        final Display d=((WindowManager) Objects.requireNonNull(context.getSystemService(Context.WINDOW_SERVICE))).getDefaultDisplay();
        choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();
        //System.out.println("refreshRate:"+d.getRefreshRate());
        //System.out.println("app Vsync offset"+choreographerVsyncOffsetNS/1000000.0f);
    }

    public void setRenderer(final IRendererSuperSync mRenderer){
        this.mRenderer=mRenderer;
    }


    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void onResume(){
        //FullscreenHelper.setImmersiveSticky(this);
        //FullscreenHelper.enableAndroidVRModeIfPossible(this);
        resumeX();
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void onPause(){
        pauseX();
        //FullscreenHelper.disableAndroidVRModeIfEnabled(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        destroyX();
    }



    private static void debug(final String s){
        Log.d(TAG,s);
    }


    @Override
    public void onContextCreated(int width, int height, SurfaceTextureHolder surfaceTextureHolder) {
        mRenderer.onContextCreated(width,height,surfaceTextureHolder);
    }

    @Override
    public void onDrawFrame() {
        mRenderer.onDrawFrame();
    }


    public interface IRendererSuperSync {
        void onContextCreated(int width, int height, SurfaceTextureHolder surfaceTextureHolder);
        void onDrawFrame();
    }

}