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
        switch(model){
            case FreeCam:
                direction.x = sinf(theta) * sinf(phi);
                direction.z = -cosf(theta) * sinf(phi);
                direction.y = -cosf(phi);
                //and normalize this directional vector!
                direction = glm::normalize(direction );
                break;
            case ArcBall:
                position = _computePosition();
                break;
            case FirstPerson:
                break;
        }


    }

    void keyPress(bool W, bool S){
        if(model == Model::FreeCam){
            if(W) {
                position += 4*CAM_SPEED * direction;
            }else if(S){
                position -= 4*CAM_SPEED * direction;
            }
        }
    }

    void mouseMovement(float xDelta, float yDelta, bool ctrl){
        switch(model){
            case FreeCam:
                setTheta(theta + CAM_ANGULAR_VEL * xDelta);
                setPhi(phi - CAM_ANGULAR_VEL * yDelta);
                break;
            case ArcBall:
                if (ctrl) {
                    setDistance(distance + CAM_SPEED * yDelta);
                }
                setTheta(theta - CAM_ANGULAR_VEL * xDelta);
                setPhi(phi + CAM_ANGULAR_VEL * yDelta);
                break;
            case FirstPerson:
                break;
        }
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
        switch(model){
            case FreeCam:
                phi = restrictVariable(phi, static_cast<float>(-M_PI+0.001), static_cast<float>(M_PI-0.001));
                break;
            case ArcBall:
                phi = restrictVariable(phi, static_cast<float>(M_PI / 2), static_cast<float>(M_PI-0.001));
                break;
            case FirstPerson:
                break;
        }

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
        if(model == ArcBall && m == FreeCam){
            setPhi(phi-M_PI);
            setTheta(theta-M_PI);
            position = position + *target;
        }if(model != ArcBall && m == ArcBall){
            setPhi(phi+M_PI/4);
            setTheta(theta+M_PI);
        }
        model = m;
        update();
    }

    const glm::vec3 &getPos() const {
        return position;
    }

    void setPos(const glm::vec3 &camPos) {
        this->position = camPos;
    }

    glm::mat4 getLookAt() const{
        switch(model){
            case FreeCam:
                return glm::lookAt(position,        // camera is located at (10, 10, 10)
                                   position+direction,        // camera is looking at (0, 0, 0,)
                                   glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
                break;
            case ArcBall:
                return glm::lookAt(position + *target,        // camera is located at (10, 10, 10)
                                   *target,        // camera is looking at (0, 0, 0,)
                                   glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
                break;
            case FirstPerson:
                break;
        }

    }

private:
    float theta;
    float phi;
    float distance;
    glm::vec3* target;
    Model model;
    glm::vec3 position;       // camera position in cartesian coordinates
    glm::vec3 direction;
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
