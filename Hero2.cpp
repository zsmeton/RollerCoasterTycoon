//
// Created by linds on 9/29/2019.
//



#include "Hero2.h"

using namespace std;
//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.





Snake::Snake() {
    recomputeSnakeOrientation();
}

//*************************************************************************************
//
// Helper Functions
void Snake::recomputeSnakeOrientation() {

    snakeDir.x = cos(snakeTheta);
    snakeDir.y = 0;
    snakeDir.z = sin(snakeTheta);
    float magnitude = sqrt(pow(snakeDir.x, 2) + pow(snakeDir.y, 2) + pow(snakeDir.z, 2));
    // and NORMALIZE this directional vector!!!
    snakeDir.x /= magnitude;
    snakeDir.y /= magnitude;
    snakeDir.z /= magnitude;
}


void Snake::drawSnakeEye() {
    glColor3f(0.0f, 0.0f, 0.0f); //black
    CSCI441::drawSolidSphere(0.25, 30, 30);
}

void Snake::flickingTongue() {
    if (flickingTongueDir == 15 || flickingTongueDir == 25) {
        tongueChange *= -1;
    }
    flickingTongueDir += tongueChange;
}

void Snake::drawSnakeTongue() {
    glColor3f(0.9f, 0.0f, 0.4f); //red for tongue

    glm::mat4 snakeTongueTrans1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.15, 10, 0));
    glMultMatrixf(&snakeTongueTrans1[0][0]);
    {
        CSCI441::drawSolidCone(0.25, flickingTongueDir, 10, 10);
    };
    glMultMatrixf(&(glm::inverse(snakeTongueTrans1))[0][0]);

    glm::mat4 snakeTongueTrans2 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.15, 10, 0));
    glMultMatrixf(&snakeTongueTrans2[0][0]);
    {
        CSCI441::drawSolidCone(0.25, flickingTongueDir, 10, 10);
    };
    glMultMatrixf(&(glm::inverse(snakeTongueTrans2))[0][0]);
}

void Snake::drawSnakeHead() {
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeHeadTrans1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 15, 0));
    glMultMatrixf(&snakeHeadTrans1[0][0]);
    {
        CSCI441::drawSolidCylinder(1.25, 2, 3, 30, 30);
    };
    glMultMatrixf(&(glm::inverse(snakeHeadTrans1))[0][0]);

    glm::mat4 snakeHeadTrans2 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 18, 0));
    glMultMatrixf(&snakeHeadTrans2[0][0]);
    {
        CSCI441::drawSolidCylinder(2, 1, 2, 30, 30);
    };
    glMultMatrixf(&(glm::inverse(snakeHeadTrans2))[0][0]);

    glm::mat4 snakeHeadTrans3 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 19.7, 0));
    glMultMatrixf(&snakeHeadTrans3[0][0]);
    {
        CSCI441::drawSolidSphere(1.0, 30, 30);
    };
    glMultMatrixf(&(glm::inverse(snakeHeadTrans3))[0][0]);

    glm::mat4 snakeEyeTrans1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1, 19.7, 0.75));
    glMultMatrixf(&snakeEyeTrans1[0][0]);
    {
        drawSnakeEye();
    };
    glMultMatrixf(&(glm::inverse(snakeEyeTrans1))[0][0]);

    glm::mat4 snakeEyeTrans2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.75, 19.7, 1));
    glMultMatrixf(&snakeEyeTrans2[0][0]);
    {
        drawSnakeEye();
    };
    glMultMatrixf(&(glm::inverse(snakeEyeTrans2))[0][0]);
}

void Snake::drawSnakeBody() {
    double snakeLength = 15.0;
    double snakeRad = 1.5;
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeBodyTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glMultMatrixf(&snakeBodyTrans[0][0]);
    {
        CSCI441::drawSolidCylinder(snakeRad, snakeRad - 0.25, snakeLength, 30, 30);
    };
    glMultMatrixf(&(glm::inverse(snakeBodyTrans))[0][0]);


    glColor3f(0.9f, 0.0f, 0.4f); //red for scales
    glm::mat4 snakeBodyWireTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glMultMatrixf(&snakeBodyWireTrans[0][0]);
    {
        glm::mat4 snakeBodyScale = glm::scale(glm::mat4(1.0f), glm::vec3(1.04f, 1.0, 1.04f));
        glMultMatrixf(&snakeBodyScale[0][0]);
        {
            CSCI441::drawWireCylinder(snakeRad, snakeRad - 0.25, snakeLength, 15, 15);
        };
        glMultMatrixf(&(glm::inverse(snakeBodyScale))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(snakeBodyWireTrans))[0][0]);
}

void Snake::drawSnakeTail() {
    double snakeTailLength = 12.0;
    double snakeTailRad = 1.5;
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeBodyTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glMultMatrixf(&snakeBodyTrans[0][0]);
    {
        glm::mat4 snakeBodyRot = glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&snakeBodyRot[0][0]);
        {
            CSCI441::drawSolidCone(snakeTailRad, snakeTailLength, 30, 30);
        };
        glMultMatrixf(&(glm::inverse(snakeBodyRot))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(snakeBodyTrans))[0][0]);


    glColor3f(0.9f, 0.0f, 0.4f); //red for scales
    glm::mat4 snakeBodyWireTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glMultMatrixf(&snakeBodyWireTrans[0][0]);
    {
        glm::mat4 snakeBodyWireRot = glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0.0f, 0.0f, 1.0f));
        glMultMatrixf(&snakeBodyWireRot[0][0]);
        {
            glm::mat4 snakeBodyWireScale = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f, 1.0, 1.05f));
            glMultMatrixf(&snakeBodyWireScale[0][0]);
            {
                CSCI441::drawSolidCone(snakeTailRad, snakeTailLength, 7, 7);
            };
            glMultMatrixf(&(glm::inverse(snakeBodyWireScale))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(snakeBodyWireRot))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(snakeBodyWireTrans))[0][0]);
}

void Snake::drawSnake() {
    drawSnakeHead();
    drawSnakeBody();
    drawSnakeTail();
}


void Snake::drawConeUp() {
    glm::mat4 coneTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glMultMatrixf(&coneTrans[0][0]);
    CSCI441::drawSolidCone(1.0, 4.0, 20, 60);
    glMultMatrixf(&(glm::inverse(coneTrans))[0][0]);
}

void Snake::drawConeDown() {
    glm::mat4 coneTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0));
    glMultMatrixf(&coneTrans[0][0]);
    glm::mat4 coneRot = glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf(&coneRot[0][0]);
    CSCI441::drawSolidCone(1.0, 4.0, 20, 60);
    glMultMatrixf(&(glm::inverse(coneRot))[0][0]);
    glMultMatrixf(&(glm::inverse(coneTrans))[0][0]);
}

void Snake::drawConeLeft() {
    glm::mat4 coneTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0));
    glMultMatrixf(&coneTrans[0][0]);
    glm::mat4 coneRot = glm::rotate(glm::mat4(1.0f), -PI / 2, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf(&coneRot[0][0]);
    CSCI441::drawSolidCone(1.0, 4.0, 20, 60);
    glMultMatrixf(&(glm::inverse(coneRot))[0][0]);
    glMultMatrixf(&(glm::inverse(coneTrans))[0][0]);
}

void Snake::drawConeRight() {
    glm::mat4 coneTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0));
    glMultMatrixf(&coneTrans[0][0]);
    glm::mat4 coneRot = glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf(&coneRot[0][0]);
    CSCI441::drawSolidCone(1.0, 4.0, 20, 60);
    glMultMatrixf(&(glm::inverse(coneRot))[0][0]);
    glMultMatrixf(&(glm::inverse(coneTrans))[0][0]);
}

void Snake::drawSparkle() {

    sparkleRotTheta += 0.1f;
    if (sparkleRotTheta >= 2 * PI) {
        sparkleRotTheta -= (2 * PI);
    }

    glm::mat4 sparkleRot = glm::rotate(glm::mat4(1.0f), sparkleRotTheta, glm::vec3(0.0f, 1.0f, 0.0));
    glMultMatrixf(&sparkleRot[0][0]);
    drawConeRight();
    drawConeLeft();
    drawConeUp();
    drawConeDown();
    glMultMatrixf(&(glm::inverse(sparkleRot))[0][0]);
}

void Snake::drawFaery() {

    glm::mat4 faeryTrans = glm::translate(glm::mat4(1.0f), faeryMovement[faeryIndex]);
    glMultMatrixf(&faeryTrans[0][0]);
    {
        glm::mat4 faeryScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
        glMultMatrixf(&faeryScale[0][0]);
        {
            glColor3f(1.0f, 1.0f, 1.0); //white
            drawSparkle();
        };
        glMultMatrixf(&(glm::inverse(faeryScale))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(faeryTrans))[0][0]);

}

void Snake::draw() {
    worldMtx = glm::translate(glm::mat4(1.0f), getPos());
    worldMtx = glm::rotate(worldMtx, -snakeTheta, glm::vec3(0.0f, 1.0, 0.0f));
    glMultMatrixf(&worldMtx[0][0]);{
        if (faeryMovement.size() != 0) {
            drawFaery();
        }
    }glMultMatrixf(&(glm::inverse(worldMtx))[0][0]);
    worldMtx = glm::rotate(worldMtx, PI / 2, glm::vec3(0.0f, 0.0, 1.0f));
    worldMtx = glm::rotate(worldMtx, PI/2, glm::vec3(0.0f,1.0f,0.0f));
    worldMtx = glm::scale(worldMtx, glm::vec3(0.15f, 0.15f, 0.15f));
    glMultMatrixf(&worldMtx[0][0]);{
                drawSnakeTongue();
                drawSnake();
    }glMultMatrixf(&(glm::inverse(worldMtx))[0][0]);
    // Position the camera correctly
    worldMtx = glm::scale(worldMtx, glm::vec3(6.67f, 6.67f, 6.67f));
    worldMtx = glm::rotate(worldMtx, PI/2, glm::vec3(0.0f,1.0f,0.0f));
    worldMtx = glm::translate(worldMtx, glm::vec3(0.0f, -6.67f, 0.0f));
    worldMtx = glm::rotate(worldMtx, -PI/2, glm::vec3(1.0f,0.0f,0.0f));
    worldMtx = glm::translate(worldMtx, glm::vec3(-1.0f, -1.0, 10.0f));
}

void Snake::update() {
    if(faeryMovement.size() != 0) {
        faeryIndex = (faeryIndex + 1) % faeryMovement.size();
    }
}

void Snake::setFaeryPath(const vector<glm::vec3>& controlPoints){
    int max_i = controlPoints.size()-4;
    if(max_i < 0) return;
    if(max_i > 0){
        max_i += (controlPoints.size()-4)/3;
    }
    faeryMovement.clear();
    for(float i = 0; i < max_i; i += FPS_ADJUSTMENT*0.01){
        faeryMovement.emplace_back(computePositionBezierCurve(controlPoints, i));
    }
}


