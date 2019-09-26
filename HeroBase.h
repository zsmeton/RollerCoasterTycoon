//
// Created by zsmeton on 9/25/19.
//

#ifndef MP_HEROBASE_H
#define MP_HEROBASE_H
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <CSCI441/objects.hpp> // for our 3D objects
#include "HelperFunctions.h"
const float FPS_ADJUSTMENT = 0.2;

class HeroBase{
public:
    virtual void draw()=0;
    virtual void update()=0;

    const float &getX() const{
        return pos.x;
    }

    void setX(const float& x){
        pos.x = x;
    }

    const float &getY() const{
        return pos.y;
    }

    void setY(const float& y){
        pos.y = y;
    }

    const float &getZ() const{
        return pos.z;
    }

    void setZ(const float& z){
        pos.z = z;
    }

    const glm::vec3 &getPos() const {
        return pos;
    }

    void setPos(const glm::vec3 &pos) {
        HeroBase::pos = pos;
    }

private:
    glm::vec3 pos;
};

#endif //MP_HEROBASE_H
