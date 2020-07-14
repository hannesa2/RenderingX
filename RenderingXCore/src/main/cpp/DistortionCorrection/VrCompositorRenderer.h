//
// Created by geier on 22/06/2020.
//

#ifndef FPV_VR_OS_VRCOMPOSITORRENDERER_H
#define FPV_VR_OS_VRCOMPOSITORRENDERER_H

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GLProgramTexture.h>
#include <GLProgramVC.h>
#include <GLBuffer.hpp>
#include <TexturedGeometry.hpp>
#include <variant>
#include "MatrixHelper.h"

#define PRECALCULATE_STATIC_LAYER

class VrCompositorRenderer {
public:
    /**
     * Can be constructed without a OpenGL context bound. Don't forget to call initializeGL once context becomes available.
     * @param gvr_api The gvr_api is used for head tracking only
     * @param ENABLE_VDDC if V.D.D.C is not enabled, the VR layers are rendered without distortion correction
     * @param occlusionMeshColor1 Use a custom color for the occlusion mesh for Debugging
     */
    VrCompositorRenderer(gvr::GvrApi *gvr_api,const bool ENABLE_VDDC,const TrueColor occlusionMeshColor1=TrueColor2::BLACK);
    /**
     *  Call this once the OpenGL context is available
     */
    void initializeGL();
// Head Tracking begin   ---
private:
    gvr::GvrApi *gvr_api;
    //translation matrix representing half inter-eye-distance
    glm::mat4 eyeFromHead[2]{};
    //projection matrix created using the fov of the headset
    glm::mat4 mProjectionM[2]{};
    glm::mat4 latestHeadSpaceFromStartSpaceRotation=glm::mat4(1.0f);
public:
    // we do not want the view (rotation) to change during rendering of one frame/eye
    // else we could end up with multiple elements rendered in different perspectives
    void updateLatestHeadSpaceFromStartSpaceRotation();
    // returns the latest 'cached' head rotation
    glm::mat4 GetLatestHeadSpaceFromStartSpaceRotation()const;
// Head tracking end ---
// V.D.D.C begin ---
public:
    //These values must match the surface that is used for rendering VR content
    //E.g. must be created as full screen surface
    int SCREEN_WIDTH_PX=1920;
    int SCREEN_HEIGHT_PX=1080;
    int EYE_VIEWPORT_W=SCREEN_WIDTH_PX/2;
    int EYE_VIEWPORT_H=SCREEN_HEIGHT_PX;
    // Min and Max clip distance
    static constexpr float MIN_Z_DISTANCE=0.1f;
    static constexpr float MAX_Z_DISTANCE=100.0f;
private:
    std::array<MLensDistortion::ViewportParamsHSNDC,2> screen_params{};
    std::array<MLensDistortion::ViewportParamsHSNDC,2> texture_params{};
    PolynomialRadialDistortion mDistortion{};
    PolynomialRadialInverse mInverse{};
    VDDC::DataUnDistortion mDataUnDistortion=VDDC::DataUnDistortion::identity();
public:
    // update with vr headset params
    void updateHeadsetParams(const MVrHeadsetParams& mDP);
// V.D.D.C end ---
private:
    static constexpr bool ENABLE_OCCLUSION_MESH=true;
    const TrueColor occlusionMeshColor;
    //One for left and right eye each
    std::array<GLProgramVC::ColoredGLMeshBuffer,2> mOcclusionMesh;
    const bool ENABLE_VDDC;
    //this one is for drawing the occlusion mesh only, no V.D.D.C, source mesh holds NDC
    std::unique_ptr<GLProgramVC2D> mGLProgramVC2D;
    // Use NDC (normalized device coordinates), both for normal and ext texture
    std::unique_ptr<GLProgramTexture> mGLProgramTexture2D;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt2D;
    // Apply V.D.D.C to the 3d coordinates, both for normal and ext texture
    std::unique_ptr<GLProgramTexture> mGLProgramTextureVDDC;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExtVDDC;
public:
    // NONE == position is fixed
    // TODO: Pre-calculate vertices for fixed position (Since position relative to head does not change, I can calculate the un-distorted Vertices once
    // TODO: instead of calculating them dynamically in the Vertex shader
    enum HEAD_TRACKING{
        NONE,
        FULL
    };
    // https://developer.oculus.com/documentation/unity/unity-ovroverlay/
    struct VRLayer{
        HEAD_TRACKING headTracking;
        // If head tracking is disabled for this layer we can pre-calculate the undistorted vertices
        // for both the left and right eye. Else, the vertex shader does the un-distortion and
        // we do not touch the mesh data.
        std::unique_ptr<GLProgramTexture::TexturedGLMeshBuffer> meshLeftAndRightEye=nullptr;
        std::unique_ptr<GLProgramTexture::TexturedGLMeshBuffer> optionalLeftEyeDistortedMesh=nullptr;
        std::unique_ptr<GLProgramTexture::TexturedGLMeshBuffer> optionalRightEyeDistortedMesh=nullptr;
        GLuint textureId;
        bool isExternalTexture;
    };
    // List of layer descriptions
    std::vector<VRLayer> mVrLayerList;
    void addLayer(const GLProgramTexture::TexturedMeshData& meshData, GLuint textureId, bool isExternalTexture=false, HEAD_TRACKING headTracking=FULL);

    void removeLayers();
    void drawLayers(gvr::Eye eye);

    // Add a 2D layer at position (0,0,Z) and (width,height) in VR 3D space.
    void addLayer2DCanvas(float z,float width,float height,GLuint textureId, bool isExternalTexture=false);
    // Add a 360° video sphere
    void addLayerEquirectangularMonoscopic360(float radius,GLuint textureId, bool isExternalTexture=false);
public:
    // NOT VR
    //void drawLayersMono(glm::mat4 ViewM, glm::mat4 ProjM);
    //void updateHeadsetParams(const MVrHeadsetParams &mDP);
    //
private:
    // Set the viewport to exactly half framebuffer size
    // where framebuffer size==screen size
    void setOpenGLViewport(gvr::Eye eye)const;
public:
    //This one does not use the inverse and is therefore (relatively) slow compared to when
    //using the approximate inverse
    glm::vec2 UndistortedNDCForDistortedNDC(const glm::vec2& in_ndc,int eye)const{
        const auto ret= MLensDistortion::UndistortedNDCForDistortedNDC(mDistortion,mDataUnDistortion.screen_params[eye],mDataUnDistortion.texture_params[eye],{in_ndc.x,in_ndc.y},false);
        return glm::vec2(ret[0],ret[1]);
    }
    static std::array<float,4> reverseFOV(const std::array<float,4>& fov){
        return {fov[1],fov[0],fov[2],fov[3]};
    }
    glm::vec3 UndistortedNDCFor3DPoint(const gvr::Eye eye,const glm::vec3 point,const glm::mat4 headSpaceFromStartSPaceRotation=glm::mat4(1.0f)){
        const int EYE_IDX=eye==GVR_LEFT_EYE ? 0 : 1;
        const auto MVMatrix=eyeFromHead[EYE_IDX]*headSpaceFromStartSPaceRotation;
        const glm::vec4 pos_view= MVMatrix * glm::vec4(point, 1.0f);
        const glm::vec4 pos_clip=mProjectionM[EYE_IDX]*pos_view;
        const glm::vec3 ndc=glm::vec3(pos_clip)/pos_clip.w;
        const glm::vec2 dist_p=UndistortedNDCForDistortedNDC({ndc.x,ndc.y},EYE_IDX);
        //const glm::vec4 gl_Position=glm::vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);
        const glm::vec4 lola=glm::vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);
        return glm::vec4(glm::vec3(lola)/lola.w,1.0);
        //MLOGD<<"w value"<<gl_Position.w;
        //return glm::vec2(gl_Position.x,gl_Position.y)/gl_Position.w;
        //return gl_Position;
    }
    // Distort the mesh for the selected perspective from either the left or right eye perspective
    TexturedMeshData distortMesh(const gvr::Eye eye,const TexturedMeshData& input){
        auto tmp=input;
        //if(input.hasIndices()){
        //    MLOGD<<"Merging indices into vertices";
        //    tmp.mergeIndicesIntoVertices();
        //}
        const int EYE_IDX=eye==GVR_LEFT_EYE ? 0 : 1;
        for(auto& vertex : tmp.vertices){
            const glm::vec3 pos=glm::vec3(vertex.x,vertex.y,vertex.z);
            const glm::vec3 newPos=UndistortedNDCFor3DPoint(eye,pos);
            //const glm::vec3 newPos=VDDC::CalculateVertexPosition(mDataUnDistortion.radialDistortionCoefficients,
            //        mDataUnDistortion.screen_params[EYE_IDX],mDataUnDistortion.texture_params[EYE_IDX],eyeFromHead[EYE_IDX]*glm::mat4(1.0f),
            //        mProjectionM[EYE_IDX],glm::vec4(pos,1.0f));
            vertex.x=newPos.x;
            vertex.y=newPos.y;
            vertex.z=newPos.z;
        }
        return tmp;
    }
};


#endif //FPV_VR_OS_VRCOMPOSITORRENDERER_H
