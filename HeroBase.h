//
// Created by zsmeton on 9/25/19.
//

#ifndef MP_HEROBASE_H
#define MP_HEROBASE_H
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <CSCI441/objects.hpp> // for our 3D objects
#include "HelperFunctions.h"
#include "GlobalConsts.h"
#include "FirstPersonCamera.h"

class HeroBase{
public:
    HeroBase(){
        fpv.setMtx(worldMtx);
    }

    virtual void draw()=0;
    virtual void update()=0;

    const float &getX() const{
        return pos.x;
    }

    virtual void setX(const float& x){
        pos.x = x;
    }

    const float &getY() const{
        return pos.y;
    }

    virtual void setY(const float& y){
        pos.y = y;
    }

    const float &getZ() const{
        return pos.z;
    }

    virtual void setZ(const float& z){
        pos.z = z;
    }

    glm::vec3 &getPos(){
        return pos;
    }

    virtual void setPos(const glm::vec3 &pos) {
        HeroBase::pos = pos;
    }

    FirstPersonCamera& getFPVCam(){
        return fpv;
    }

private:
    glm::vec3 pos{};
    FirstPersonCamera fpv{};

protected:
    glm::mat4 worldMtx{};
};

#endif //MP_HEROBASE_H
