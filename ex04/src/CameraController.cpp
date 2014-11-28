#include "CameraController.h"

#include <glm/gtx/transform.hpp>

CameraController::CameraController(float theta, float phi, float dist) {
    reset(theta, phi, dist);
}

CameraController::~CameraController() {}

void CameraController::updateMousePos(int x, int y) {
    switch (mState) {
    case LEFT_BTN : {
        int dx = mX - x, dy = mY -y;
        
        mX = x;
        mY = y;
        
        mTheta += dx / 2.;
        mPhi += dy / 2.;
        
        mTheta -= (mTheta > 360) ? 360 : 0;
        mPhi -= (mPhi > 360) ? 360 : 0;
        
        mTheta += (mTheta < 0) ? 360 : 0;
        mPhi += (mPhi  < 0) ? 360 : 0;

        break;
    }
    case RIGHT_BTN : {
        // not used yet //
        break;
    }
    default : break;
    }
}

void CameraController::updateMouseBtn(MouseState state, int x, int y) {
    switch (state) {
    case NO_BTN : {
        mLastTheta = mTheta;
        mLastPhi = mPhi;
        break;
    }
    case LEFT_BTN : {
        mX = x;
        mY = y;
        break;
    }
    case RIGHT_BTN : {
        // not used yet //
        break;
    }
    default : break;
    }
    mState = state;
}

void CameraController::move(Motion motion) {
    // init direction multiplicator (forward/backward, left/right are SYMMETRIC!) //
    //int dir = 1;
    glm::vec3 center = getCenter();
    glm::vec3 lookDir = glm::normalize(center - mCameraPosition) / 10.f;
    glm::vec3 rightDir = glm::cross(lookDir, {0, 1, 0});
    glm::vec3 upDir = glm::cross(rightDir, lookDir);
    
    switch (motion) {
    case MOVE_FORWARD:
        mCameraPosition += lookDir;
        break;
    case MOVE_BACKWARD:
        mCameraPosition -= lookDir;
        break;
    case MOVE_RIGHT:
        mCameraPosition += rightDir;
        break;
    case MOVE_LEFT:
        mCameraPosition -= rightDir;
        break;
    case MOVE_UP:
        mCameraPosition += upDir;
        break;
    case MOVE_DOWN:
        mCameraPosition -= upDir;
        break;
    // TODO: move camera along or perpendicular to its viewing direction or along y according to motion state //
    //       motion state is one of: (MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN)
    default : break;
    }
}

glm::mat4 CameraController::getProjectionMat(void) {
    // TODO:  Return perspective matrix describing the camera intrinsics.
    //        The perspective matrix has been derived in the lecture.
    glm::mat4 projectionMat = glm::perspective(mOpenAngle, mAspect, mNear, mFar);
    return projectionMat;
}

glm::mat4 CameraController::getModelViewMat(void) {
    glm::mat4 modelViewMat = glm::lookAt(mCameraPosition, getCenter(), {0, 1, 0});
    return modelViewMat;
}

void CameraController::reset(float theta, float phi, float dist) {
    // reset everything //
    resetOrientation(theta, phi, dist);
    resetIntrinsics();
    mX = 0;
    mY = 0;
    mState = NO_BTN;
}

void CameraController::resetOrientation(float theta, float phi, float dist) {
    // reset camera extrisics //
    mPhi = phi;
    mLastPhi = mPhi;
    mTheta = theta;
    mLastTheta = mTheta;
    // move camera about 'dist' along opposite of rotated view vector //
    mCameraPosition = glm::vec3(sin(mTheta) * cos(mPhi) * dist,
                                sin(mPhi) * dist,
                                cos(mTheta) * cos(mPhi) * dist);
    //auto center = getCenter();
    //center = center;
    // lookAt position is now (0, 0, 0) //
}

void CameraController::resetIntrinsics(float angle, float aspect, float near, float far) {
    // reset intrinsic parameters //
    setOpeningAngle(angle);
    setAspect(aspect);
    setNear(near);
    setFar(far);
}

void CameraController::setNear(float near) {
    mNear = near;
}

float CameraController::getNear(void) {
    return mNear;
}

void CameraController::setFar(float far) {
    mFar = far;
}

float CameraController::getFar(void) {
    return mFar;
}

void CameraController::setOpeningAngle(float angle) {
    mOpenAngle = angle;
}

float CameraController::getOpeningAngle(void) {
    return mOpenAngle;
}

void CameraController::setAspect(float ratio) {
    mAspect = ratio;
}

float CameraController::getAspect(void) {
    return mAspect;
}

glm::vec3 CameraController::getCameraPosition(void) {
    return mCameraPosition;
}

glm::vec3 CameraController::getCenter()
{
    glm::vec3 center = mCameraPosition + glm::vec3(glm::rotate(mTheta, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 0));
    center.y += glm::sin(glm::radians(mPhi));
    
    return center;
}
