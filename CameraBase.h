//
// Created by zsmeton on 10/8/19.
//

#ifndef MP_CAMERABASE_H
#define MP_CAMERABASE_H

#ifndef MP_CAMERA_H
#define MP_CAMERA_H

#ifdef __APPLE__                    // if compiling on Mac OS
#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS

#include <GL/gl.h>

#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "GlobalConsts.h"

class CameraBase {

    virtual void update() = 0;

    virtual void keyPress(bool W, bool S) = 0;

    virtual void mouseMovement(float xDelta, float yDelta, bool ctrl) = 0;

    float getTheta() const {
        return theta;
    }

    void setTheta(float theta) {
        this->theta = theta;
    }

    float getPhi() const {
        return phi;
    }

    virtual void setPhi(float phi) = 0;

    const glm::vec3 &getPos() const {
        return position;
    }

    void setPos(const glm::vec3 &camPos) {
        this->position = camPos;
    }

    virtual glm::mat4 getLookAt() const = 0;

private:
    float theta;
    float phi;
    glm::vec3 position;       // camera position in cartesian coordinates
};

#endif //MP_CAMERABASE_H
