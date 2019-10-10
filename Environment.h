//
// Created by zsmeton on 10/8/19.
//

#ifndef MP_ENVIRONMENT_H
#define MP_ENVIRONMENT_H

#ifdef __APPLE__                    // if compiling on Mac OS
#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS

#include <GL/gl.h>

#endif

#include <GLFW/glfw3.h>    // include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "HelperFunctions.h"


class EnvironmentObject{
public:
    EnvironmentObject(float x, float y, float z) : x(x), y(y), z(z) {};
    virtual void render() = 0;
protected:
    float x,y,z;
};

class Ball : public EnvironmentObject{
    void render() override{
        float randVal = getRand();
        float randScale = rand() % 2 + 1;
        int randScaleInt = rand() % 2 + 1;
        glm::mat4 randBall = glm::translate( glm::mat4(1.0f), glm::vec3( x, y+ randScale/2.0f, z ) );
        glMultMatrixf( &randBall[0][0] );{
            glm::mat4 scaleBall = glm::scale(glm::mat4(1.0f), glm::vec3(randScale* 1.0f, randScale*1.0, randScale*1.0f));
            glMultMatrixf(&scaleBall[0][0]);{
                glColor3f(getRand(), getRand(), getRand()); //random color
                if(randScaleInt % 2 == 0) {
                    CSCI441::drawSolidSphere(getRand(), 30, 30);
                }else{
                    CSCI441::drawWireSphere(getRand(), 15, 15);
                }
            }; glMultMatrixf(&(glm::inverse(scaleBall))[0][0]);
        }; glMultMatrixf( &(glm::inverse( randBall ))[0][0] );

    }

public:
    Ball(float x, float y, float z) : EnvironmentObject(x, y, z) {
    }
};

class Bush : public EnvironmentObject{
    void render() override{
        // Choose random color
        float green = getRand();
        for (int i = 0; i < int(getRand()*25)+5; i++) {
            glColor3f(0.0f, green, 0.0f);
            GLfloat color[4] = {0.0f,green,0.0f, 1};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
            // Choose random height
            float bushHeight = getRand() * 2;
            glm::mat4 bushMtx = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(x, y+bushHeight / 2, z));
            bushMtx = glm::rotate(bushMtx, float(getRand() * M_PI), glm::vec3(getRand(), getRand(), getRand()));
            bushMtx = glm::scale(bushMtx, glm::vec3(bushHeight, bushHeight, bushHeight));
            glMultMatrixf(&bushMtx[0][0]);
            {
                CSCI441::drawSolidCube(1.0f);
            }
            glMultMatrixf(&(glm::inverse(bushMtx))[0][0]);
        }
    }

public:
    Bush(float d, float d1, float d2) : EnvironmentObject(d, d1, d2) {

    }
};

class Rock : public EnvironmentObject{
    void render() override {
        float grey = getRand() * 0.5f;
        glColor3f(grey, grey, grey);
        for (int i = 0; i < int(getRand() * 25)+5; i++) {
            GLfloat color[4] = {grey, grey, grey, 1};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
            // Choose random size
            float rockHeight = getRand() * 1;
            glm::mat4 rockMtx = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(x, y+rockHeight / 2.0f, z));
            rockMtx = glm::rotate(rockMtx, float(getRand() * M_PI), glm::vec3(getRand(), getRand(), getRand()));
            rockMtx = glm::scale(rockMtx, glm::vec3(rockHeight, rockHeight, rockHeight));
            glMultMatrixf(&rockMtx[0][0]);
            {
                CSCI441::drawSolidCube(1.0f);
            }
            glMultMatrixf(&(glm::inverse(rockMtx))[0][0]);
        }
    }

public:
    Rock(float d, float d1, float d2) : EnvironmentObject(d, d1, d2) {

    }
};

#endif //MP_ENVIRONMENT_H
