
#ifndef HERO2_BP_H
#define HERO2_BP_H

// include the OpenGL library header
#ifdef __APPLE__					// if compiling on Mac OS
#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>	// include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality

#include <iostream>
#include <sstream>

#include <fstream>			// for file I/O
#include <vector>				// for vector
#include "HeroBase.h"
#include "BezierCurve.h"
#include "FaeryHero.h"

class Snake: public HeroBase, public FaeryHero{

public:
    constexpr static const float DT = FPS_ADJUSTMENT * 0.01f;
    constexpr static const float DR = -FPS_ADJUSTMENT * 1.0f;


    Snake();                            //Constructor
    void draw() override;
    void update() override;

    // Getters and setters
    void setFaeryPath(const vector<glm::vec3>& controlPoints) override;

    const vector<glm::vec3> &getControlPoints() const;

    void setControlPoints(const vector<glm::vec3> &controlPoints);

private:
    int flickingTongueDir = 20;
    int tongueChange = 1;
    vector<glm::vec3> controlPoints;
    vector<glm::vec3> faeryMovement;
    int faeryIndex = 0;
    float sparkleRotTheta = 0;
    float minT = 0;
    float maxT;
    float t=0;
    float theta= 0;

    float PI = 3.141592;
    glm::vec3 UP = glm::vec3(0.0f,0.0f,1.0f);


    void recomputeSnakeOrientation();
    void restrictBezierCurvePos();
    void drawSnakeEye();
    void flickingTongue();
    void drawSnakeTongue();
    void drawSnakeHead();
    void drawSnakeBody();
    void drawSnakeTail();
    void drawSnake();
    void drawConeUp();
    void drawConeDown();
    void drawConeLeft();
    void drawConeRight();
    void drawSparkle();
    void drawFaery();


};

#endif //HERO2_BP_H