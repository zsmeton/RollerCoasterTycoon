//
// Created by zsmeton on 10/9/19.
//

#ifndef MP_LOOKABLE_H
#define MP_LOOKABLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Lookable{
public:
    virtual glm::mat4 getLookAt() const = 0;
};

#endif //MP_LOOKABLE_H
