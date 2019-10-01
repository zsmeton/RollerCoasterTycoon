//
// Created by zsmeton on 10/1/19.
//

#ifndef MP_FAERYHERO_H
#define MP_FAERYHERO_H
#include <vector>
#include <glm/glm.hpp>
using namespace std;

class FaeryHero{
public:
    virtual void setFaeryPath(const vector<glm::vec3>& controlPoints) = 0;
};

#endif //MP_FAERYHERO_H
