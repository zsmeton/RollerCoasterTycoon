//
// Created by zsmeton on 10/8/19.
//

#ifndef MP_FREECAMERA_H
#define MP_FREECAMERA_H

#include "CameraBase.h"

class FreeCamera : public CameraBase{
public:
    const float CAM_SPEED = FPS_ADJUSTMENT*0.5f;                       // camera's rate of position change
    const float CAM_ANGULAR_VEL = FPS_ADJUSTMENT*0.05f;                // camera's rate of direction change

    void update() override {
        direction.x = sinf(theta) * sinf(phi);
        direction.z = -cosf(theta) * sinf(phi);
        direction.y = -cosf(phi);
        //and normalize this directional vector!
        direction = glm::normalize(direction );
    }

    void keyPress(bool W, bool S) override {
        if(W) {
            position += 4*CAM_SPEED * direction;
        }else if(S){
            position -= 4*CAM_SPEED * direction;
        }
    }

    void mouseMovement(float xDelta, float yDelta, bool ctrl) override {
        setTheta(theta + CAM_ANGULAR_VEL * xDelta);
        setPhi(phi - CAM_ANGULAR_VEL * yDelta);
    }

    void setPhi(float phi) override {
        this->phi = restrictVariable(phi, static_cast<float>(0.0001), static_cast<float>(M_PI-0.001));
    }

    glm::mat4 getLookAt() const override {
        return glm::lookAt(position,        // camera is located at (10, 10, 10)
                           position+direction,        // camera is looking at (0, 0, 0,)
                           glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
    }

private:
    glm::vec3 direction;
};

#endif //MP_FREECAMERA_H
