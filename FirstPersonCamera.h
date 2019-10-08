//
// Created by zsmeton on 10/8/19.
//

#ifndef MP_FIRSTPERSONCAMERA_H
#define MP_FIRSTPERSONCAMERA_H

#include "CameraBase.h"

class FirstPersonCamera{

public:
    void setMtx(glm::mat4& mtx){
        characterMtx = &mtx;
    }

    glm::mat4 getLookAt() const {
        glm::vec4 dir = *characterMtx * glm::vec4(0.0f,1.0f,1.0f, 1.0f);
        glm::vec3 dir_ = glm::vec3(dir.x,dir.y,dir.z);
        glm::vec4 pos = *characterMtx * glm::vec4(0.0f,1.0f,0.0f, 1.0f);
        glm::vec3 pos_ = glm::vec3(pos.x,pos.y,pos.z);
        return glm::lookAt(pos_,        // camera is located at thet target position
                         dir_,        // camera is looking at the target direction
                            glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
    }


private:
    glm::mat4* characterMtx;
};

#endif //MP_FIRSTPERSONCAMERA_H
