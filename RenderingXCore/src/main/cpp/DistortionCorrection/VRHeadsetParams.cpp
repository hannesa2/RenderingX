//
// Created by Constantin on 1/9/2020.
//

#include <string>
#include <Helper/MDebug.hpp>
#include <MatrixHelper.h>
#include "VRHeadsetParams.h"
#include "DistortionManager.h"

void VRHeadsetParams::setGvrApi(gvr::GvrApi *gvrApi) {
    this->gvr_api=gvrApi;
}


void VRHeadsetParams::updateHeadsetParams(const MDeviceParams &mDP,int screenWidthP,int screenHeightP) {
    this->screenWidthP=screenWidthP;
    this->screenHeightP=screenHeightP;
    LOGD("%s",MLensDistortion::MDeviceParamsAsString(mDP).c_str());
    auto polynomialRadialDistortion=MPolynomialRadialDistortion(mDP.radial_distortion_params);
    //auto polynomialRadialDistortion=MPolynomialRadialDistortion({0.441, 0.156});

    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      polynomialRadialDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=MLensDistortion::reverseFOV(fovLeft);


    MLensDistortion::CalculateViewportParameters_NDC(0, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovLeft,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[0], &texture_params[0]);
    MLensDistortion::CalculateViewportParameters_NDC(1, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovRight,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[1], &texture_params[1]);

    //The daydream headset v2 distortion function is the only function where we cannot achieve a proper maxRadSq value
    //with a deviation of less than 0.001f

    MAX_RAD_SQ=1.0f;
    bool done=false;
    while(MAX_RAD_SQ<2.0f && !done){
        const auto& inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        LOGD("Max Rad Sq%f",MAX_RAD_SQ);
        const float stepSize=0.01f;
        for(float r=0;r<MAX_RAD_SQ;r+=stepSize) {
            const float deviation = MPolynomialRadialDistortion::calculateDeviation(r,polynomialRadialDistortion,inverse);
            LOGD("r %f | Deviation %f",r,deviation);
            if (deviation > 0.001f) {
                done = true;
                MAX_RAD_SQ-=stepSize;
                break;
            }
        }
        MAX_RAD_SQ+=0.01f;
    }
    //MAX_RAD_SQ=2.0f;
    LOGD("Max Rad Sq%f",MAX_RAD_SQ);
    mInverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    eyeFromHead[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    eyeFromHead[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));
}


void VRHeadsetParams::updateDistortionManager(DistortionManager &distortionManager) {
    distortionManager.updateDistortion(mInverse,MAX_RAD_SQ,screen_params,texture_params);
}

void VRHeadsetParams::updateLatestHeadSpaceFromStartSpaceRotation() {
    latestHeadSpaceFromStartSpaceRotation_=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    latestHeadSpaceFromStartSpaceRotation=toGLM(latestHeadSpaceFromStartSpaceRotation_);
}

glm::mat4 VRHeadsetParams::GetLatestHeadSpaceFromStartSpaceRotation() {
    return latestHeadSpaceFromStartSpaceRotation;
}

gvr::Mat4f VRHeadsetParams::GetLatestHeadSpaceFromStartSpaceRotation_() {
    return latestHeadSpaceFromStartSpaceRotation_;
}

glm::mat4 VRHeadsetParams::GetEyeFromHeadMatrix(gvr::Eye eye) {
    return eyeFromHead[eye];
}

glm::mat4 VRHeadsetParams::GetProjectionMatrix(gvr::Eye eye) {
    return mProjectionM[eye];
}

void VRHeadsetParams::setOpenGLViewport(gvr::Eye eye) {
    const int ViewPortW=(int)(screenWidthP/2.0f);
    const int ViewPortH=screenHeightP;
    if(eye==0){
        glViewport(0,0,ViewPortW,ViewPortH);
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    }
}