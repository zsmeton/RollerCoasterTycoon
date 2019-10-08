//
// Created by zsmeton on 10/8/19.
//

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

const float CAM_SPEED = FPS_ADJUSTMENT*0.2f;                       // camera's rate of position change
const float CAM_ANGULAR_VEL = FPS_ADJUSTMENT*0.05f;                // camera's rate of direction change

class Camera {
public:
    enum Model{FreeCam = 1, ArcBall = 2, FirstPerson = 3};

    void update(){
        camPos = _computePosition();
    }

    void mouseMovement(float xDelta, float yDelta, bool ctrl){
        if(ctrl) {
            setDistance(distance + CAM_SPEED * yDelta);

        }
        setTheta(theta - CAM_ANGULAR_VEL * xDelta);

        setPhi(phi + CAM_ANGULAR_VEL * yDelta);
    }

    float getTheta() const {
        return theta;
    }

    void setTheta(float theta) {
        this->theta = theta;
    }

    float getPhi() const {
        return phi;
    }

    void setPhi(float phi) {
        phi = restrictVariable(phi, static_cast<float>(M_PI / 2), static_cast<float>(M_PI));
        this->phi = phi;
    }

    float getDistance() const {
        return distance;
    }

    void setDistance(float distance) {
        distance = restrictVariable(distance, 2.0f, 15.0f);
        this->distance = distance;
    }

    glm::vec3 *getTarget() const {
        return target;
    }

    void setTarget(glm::vec3* target){
        this->target = target;
    }

    Model getModel() const {
        return model;
    }

    void setModel(Model m){
        model = m;
    }

    const glm::vec3 &getPos() const {
        return camPos;
    }

    void setPos(const glm::vec3 &camPos) {
        this->camPos = camPos;
    }

    glm::mat4 getLookAt() const{
        return glm::lookAt(camPos + *target,        // camera is located at (10, 10, 10)
                    *target,        // camera is looking at (0, 0, 0,)
                    glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
    }

private:
    float theta;
    float phi;
    float distance;
    glm::vec3* target;
    Model model;
    glm::vec3 camPos;       // camera position in cartesian coordinates

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


#endif //MP_CAMERA_H
