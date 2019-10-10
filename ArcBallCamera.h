//
// Created by zsmeton on 10/8/19.
//

#ifndef MP_ARCBALLCAMERA_H
#define MP_ARCBALLCAMERA_H

#include "CameraBase.h"


class ArcBallCamera : public CameraBase{
public:
    const float CAM_SPEED = FPS_ADJUSTMENT*0.2f;                       // camera's rate of position change
    const float CAM_ANGULAR_VEL = FPS_ADJUSTMENT*0.05f;                // camera's rate of direction change

    void update() override {
        position = _computePosition();
    }

    void keyPress(bool W, bool S) override {

    }

    void mouseMovement(float xDelta, float yDelta, bool ctrl) override {
        if (ctrl) {
            setDistance(distance + CAM_SPEED * yDelta);
        }else {
            setTheta(theta - CAM_ANGULAR_VEL * xDelta);
            setPhi(phi + CAM_ANGULAR_VEL * yDelta);
        }
    }

    void setPhi(float phi) override {
        this->phi = restrictVariable(phi, static_cast<float>(M_PI / 2), static_cast<float>(M_PI-0.001));
    }

    glm::mat4 getLookAt() const override {
        return glm::lookAt(position + *target,        // camera is located at (10, 10, 10)
                           *target,        // camera is looking at (0, 0, 0,)
                           glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
    }

    glm::vec3 *getTarget() const {
        return target;
    }

    void setTarget(glm::vec3* target){
        this->target = target;
    }

    void setDistance(float distance) {
        distance = restrictVariable(distance, 2.0f, 15.0f);
        this->distance = distance;
    }

private:
    float distance{};
    glm::vec3* target{};

    /*!
     * This function updates the camera's direction in cartesian coordinates based
     * on its position in spherical coordinates. Should be called every time
     * cameraTheta or cameraPhi is updated.
     * @return
     */
    glm::vec3 _computePosition() {
        GLfloat x = distance * sin(theta) * sin(phi);
        GLfloat y = -distance * cos(phi);
        GLfloat z = -distance * cos(theta) * sin(phi);
        return glm::vec3(x, y, z);
    }

};

#endif //MP_ARCBALLCAMERA_H
