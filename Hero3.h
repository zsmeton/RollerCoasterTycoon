//
// Created by zsmeton on 10/9/19.
//

#ifndef MP_HERO3_H
#define MP_HERO3_H

#include <GL/gl.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "HelperFunctions.h"
#include "HeroBase.h"
#include "BezierCurve.h"

class Coaster:public HeroBase{
public:
    // Drawing Constants
    constexpr static const float CART_WIDTH = 1.0f;
    constexpr static const float CART_THICKNESS = 0.1f;
    constexpr static const float CART_WHEEL_RADIUS = 0.3;
    constexpr static const float HORSE_WIDTH = (CART_WIDTH - CART_THICKNESS * 2) / 2;
    constexpr static const float HORSE_LENGTH = 1.0f;
    constexpr static const float HORSE_LEG_ANGLE = 4 * M_PI / 3;
    constexpr static const float HORSE_LEG_SIZE = 0.2;
    constexpr static const float HORSE_KNECK_ANGLE = 3 * M_PI / 4;
    constexpr static const float HORSE_KNECK_LENGTH = 0.4;

    // Animation Constants
    constexpr static const float WHL_VEL = FPS_ADJUSTMENT * 0.5f;
    constexpr static const float LEG_THETA_RATE = FPS_ADJUSTMENT * 0.35;

    // Movement Constants
    constexpr static const float DL = FPS_ADJUSTMENT * 1.0f;
    constexpr static const int BEZIER_CURVE_TABLE_RES = 10000; // the dt step size between points in lookup table




    Coaster() {
        restrictBezierPosition();
    }

    void draw() override {
        glm::vec3 coasterHeading = computeRotationBezierCurve(controlPoints, float(time));
        glm::vec3 rotAxis =glm::cross(UP, coasterHeading);
        float rad = acos(glm::dot( glm::normalize(coasterHeading), glm::normalize(UP)));
        // Translate to the position
        worldMtx = glm::translate(glm::mat4(1.0f), getPos());
        // Rotate to match the terrain
        worldMtx = glm::rotate(worldMtx, rad, rotAxis);
        // Translate upwards to keep wheels out of ground
        worldMtx = glm::translate(worldMtx, glm::vec3(0.0f,CART_WHEEL_RADIUS+0.25f, 0.0f));
        // Orient along direction
        //worldMtx = glm::rotate(worldMtx, theta, glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(&worldMtx[0][0]);
        {
            drawCartAndHorse();
        }
        glMultMatrixf(&(glm::inverse(worldMtx))[0][0]);
    }

    void update() override {
        moveLegs();
        leftWhlTheta += WHL_VEL;
        rightWhlTheta += WHL_VEL;
        distance += DL;
        restrictBezierPosition();
        setPos(calculatePosition());
    }

    void setControlPoints(const vector<glm::vec3> &controlPoints) {
        Coaster::controlPoints = controlPoints;
        calculateLookupTable();
    }

private:
    /*!
     * Calculate position based on lookup table
     */
     glm::vec3 calculatePosition(){
        return computePositionBezierCurve(controlPoints, convertDistToTime());
     }



    /*!
    * Find the t value for the desired distance
    */
    float convertDistToTime(){
        // If last index and neighbours did not work, run binary search
        int low = 0;
        int high = bezCurveLookupTableD.size()-1;
        int counter = 0;
        while(!(bezCurveLookupTableD.at(lastUsedIndex - 1) <= distance && bezCurveLookupTableD.at(lastUsedIndex) >= distance)){
            counter++;
            if(counter > log2(BEZIER_CURVE_TABLE_RES)+100){
                fprintf(stderr, "Could not find position on bezier curve with distance: %f, infinite loop\n", distance);
            }
            if(high < low){
                fprintf(stderr, "Cannot find position on bezier curve with distance: %f\n", distance);
                exit(-30);
            }
            lastUsedIndex = (high-low)/2+low;
            if(bezCurveLookupTableD[lastUsedIndex] < distance){
                low = lastUsedIndex+1;
            }else{
                high = lastUsedIndex-1;
            }
        }
        // use lerp for to find time based on surrounding distances
        float distance01 = variableMapping<float>(distance, bezCurveLookupTableD[lastUsedIndex - 1], bezCurveLookupTableD[lastUsedIndex], 0.0f,1.0f);
        time = lerp<float>(bezCurveLookupTableT[lastUsedIndex-1], bezCurveLookupTableT[lastUsedIndex], distance01);
        return time;
    }

    /*!
     * Calculates the distance lookup table
     */
    void calculateLookupTable(){
        float dist = 0.0f;
        glm::vec3 lastPoint = computePositionBezierCurve(controlPoints, 0.0);
        bezCurveLookupTableD.push_back(dist);
        bezCurveLookupTableT.push_back(0.0f);
        for(int i = 1; i < BEZIER_CURVE_TABLE_RES; i += 1){
            float index = (float(i)/float(BEZIER_CURVE_TABLE_RES)) * (controlPoints.size()/3);
            glm::vec3 current = computePositionBezierCurve(controlPoints, index);
            dist += glm::distance(lastPoint, current);
            bezCurveLookupTableD.push_back(dist);
            bezCurveLookupTableT.push_back(index);
            lastPoint = current;
        }
        auto minmaxiumum = minmax_element(bezCurveLookupTableD.begin(), bezCurveLookupTableD.end());
        minDist = *minmaxiumum.first;
        maxDist = *minmaxiumum.second;
    }



    /*!
     * This function keeps the bezier position between 0 and 1
     */
    void restrictBezierPosition() {
        if(minDist == maxDist && maxDist == -1) return;
        distance = fmod(distance,maxDist);
        distance = max(distance, minDist);
    }

    /*!
     * Updates the angle of the legs for a running like motion
     * @param negative move legs backwards
     */
    void moveLegs() {
        float rate = legRate;
        legTheta1 -= rate;
        legTheta2 += rate;

        if (legTheta1 > M_PI / 4 || legTheta2 < -M_PI / 4) {
            legRate = LEG_THETA_RATE;
        } else if (legTheta2 > M_PI / 4 || legTheta1 < -M_PI / 4) {
            legRate = -LEG_THETA_RATE;
        }
    }

    void drawHorseBody() const {
        // Calculate horse Width
        glm::mat4 horseMtx = glm::scale(glm::mat4(1.0f), glm::vec3(HORSE_WIDTH, HORSE_WIDTH, HORSE_LENGTH));
        glMultMatrixf(&horseMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(horseMtx))[0][0]);
    }

    void drawHorseLeg() const {
        // Draw upper leg
        glm::mat4 upperLegMtx = glm::rotate(glm::mat4(1.0f), HORSE_LEG_ANGLE, glm::vec3(1.0f, 0.0f, 0.0f));
        upperLegMtx = glm::translate(upperLegMtx, glm::vec3(0.0f, 0.0f, HORSE_LEG_SIZE / 2));
        upperLegMtx = glm::scale(upperLegMtx, glm::vec3(CART_THICKNESS, CART_THICKNESS, HORSE_LEG_SIZE));
        glMultMatrixf(&upperLegMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(upperLegMtx))[0][0]);
        // Draw lower leg
        glm::mat4 lowerLegMtx = upperLegMtx * glm::scale(upperLegMtx,
                                                         glm::vec3(1 / CART_THICKNESS, 1 / CART_THICKNESS,
                                                                   1 / HORSE_LEG_SIZE));
        lowerLegMtx = glm::translate(lowerLegMtx, glm::vec3(0.0f, 0.0f, HORSE_LEG_SIZE / 2));
        lowerLegMtx = glm::rotate(glm::mat4(1.0f), float(M_PI / 2), glm::vec3(1.0f, 0.0f, 0.0f));
        lowerLegMtx = glm::translate(lowerLegMtx, glm::vec3(0.0f, 0.0f, HORSE_LEG_SIZE / 2));
        lowerLegMtx = glm::scale(lowerLegMtx, glm::vec3(CART_THICKNESS, CART_THICKNESS, HORSE_LEG_SIZE));
        glMultMatrixf(&lowerLegMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(lowerLegMtx))[0][0]);
    }

    void drawHorseHeadAndKneck() const {
        // Draw the kneck
        glm::mat4 kneckMtx = glm::rotate(glm::mat4(1.0f), HORSE_KNECK_ANGLE, glm::vec3(1.0f, 0.0f, 0.0f));
        kneckMtx = glm::translate(kneckMtx, glm::vec3(0.0f, 0.0f, HORSE_KNECK_LENGTH / 2));
        kneckMtx = glm::scale(kneckMtx, glm::vec3(2 * HORSE_WIDTH / 5, 2 * HORSE_WIDTH / 5, HORSE_KNECK_LENGTH));
        glMultMatrixf(&kneckMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(kneckMtx))[0][0]);
        // Draw the head
        glm::mat4 headMtx = kneckMtx * glm::scale(kneckMtx, glm::vec3(1 / CART_THICKNESS, 1 / CART_THICKNESS,
                                                                      1 / HORSE_KNECK_LENGTH));
        headMtx = glm::translate(headMtx, glm::vec3(0.0f, 0.0f, HORSE_KNECK_LENGTH / 2));
        headMtx = glm::rotate(glm::mat4(1.0f), float(M_PI / 4), glm::vec3(1.0f, 0.0f, 0.0f));
        headMtx = glm::translate(headMtx, glm::vec3(0.0f, 0.0f, HORSE_KNECK_LENGTH / 2));
        headMtx = glm::scale(headMtx, glm::vec3(HORSE_WIDTH / 2, 2 * HORSE_WIDTH / 5, HORSE_KNECK_LENGTH));
        glMultMatrixf(&headMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(headMtx))[0][0]);
    }

    void drawHorse() const {
        glColor3f(0.3569f, 0.3569f, 0.3569f);
        drawHorseBody();
        // Draw left front leg
        glm::mat4 lFLegMtx = glm::translate(glm::mat4(1.0f),
                                            glm::vec3(HORSE_WIDTH / 2 + CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                      2 * HORSE_LENGTH / 5));
        lFLegMtx = glm::rotate(lFLegMtx, legTheta1, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&lFLegMtx[0][0]);
        {
            drawHorseLeg();
        }
        glMultMatrixf(&(glm::inverse(lFLegMtx))[0][0]);
        // Draw right front leg
        glm::mat4 rFLegMtx = glm::translate(glm::mat4(1.0f),
                                            glm::vec3(-HORSE_WIDTH / 2 - CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                      2 * HORSE_LENGTH / 5));
        rFLegMtx = glm::rotate(rFLegMtx, legTheta2, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&rFLegMtx[0][0]);
        {
            drawHorseLeg();
        }
        glMultMatrixf(&(glm::inverse(rFLegMtx))[0][0]);
        // Draw left back leg
        glm::mat4 lBLegMtx = glm::translate(glm::mat4(1.0f),
                                            glm::vec3(HORSE_WIDTH / 2 + CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                      -HORSE_LENGTH / 4));
        lBLegMtx = glm::rotate(lBLegMtx, legTheta2, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&lBLegMtx[0][0]);
        {
            drawHorseLeg();
        }
        glMultMatrixf(&(glm::inverse(lBLegMtx))[0][0]);
        // draw right back leg
        glm::mat4 rBLegMtx = glm::translate(glm::mat4(1.0f),
                                            glm::vec3(-HORSE_WIDTH / 2 - CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                      -HORSE_LENGTH / 4));
        rBLegMtx = glm::rotate(rBLegMtx, legTheta1, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&rBLegMtx[0][0]);
        {
            drawHorseLeg();
        }
        glMultMatrixf(&(glm::inverse(rBLegMtx))[0][0]);

        // draw the head
        glm::mat4 headKneckMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, HORSE_WIDTH, 2 * HORSE_LENGTH / 3));
        glMultMatrixf(&headKneckMtx[0][0]);
        {
            drawHorseHeadAndKneck();
        }
        glMultMatrixf(&(glm::inverse(headKneckMtx))[0][0]);


    }

    void drawCartBody() const {
        // draw bottom
        glColor3f(0.353f, 0.27f, 0.0f);
        glm::mat4 botttomMtx = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, CART_THICKNESS, 1.0f));
        glMultMatrixf(&botttomMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(botttomMtx))[0][0]);
        // draw back panel
        glm::mat4 backMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.375f, -0.5f + CART_THICKNESS / 2));
        backMtx = glm::scale(backMtx, glm::vec3(1.0f, 0.75f, CART_THICKNESS));
        glMultMatrixf(&backMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(backMtx))[0][0]);
        // draw left panel
        glm::mat4 leftPanelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f + CART_THICKNESS / 2, 0.375f, 0.0f));
        leftPanelMtx = glm::scale(leftPanelMtx, glm::vec3(CART_THICKNESS, 0.75f, 1.0f));
        glMultMatrixf(&leftPanelMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(leftPanelMtx))[0][0]);
        // draw right panel
        glm::mat4 rightPanelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f - CART_THICKNESS / 2, 0.375f, 0.0f));
        rightPanelMtx = glm::scale(rightPanelMtx, glm::vec3(CART_THICKNESS, 0.75f, 1.0f));
        glMultMatrixf(&rightPanelMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(rightPanelMtx))[0][0]);
        // draw left handle
        glm::mat4 leftHandleMtx = glm::translate(glm::mat4(1.0f),
                                                 glm::vec3(-0.5f + CART_THICKNESS / 2, CART_THICKNESS / 2, 1.0f));
        leftHandleMtx = glm::scale(leftHandleMtx, glm::vec3(CART_THICKNESS, CART_THICKNESS, 1.0f));
        glMultMatrixf(&leftHandleMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(leftHandleMtx))[0][0]);
        // draw right handle
        glm::mat4 rightHandleMtx = glm::translate(glm::mat4(1.0f),
                                                  glm::vec3(0.5f - CART_THICKNESS / 2, CART_THICKNESS / 2, 1.0f));
        rightHandleMtx = glm::scale(rightHandleMtx, glm::vec3(CART_THICKNESS, CART_THICKNESS, 1.0f));
        glMultMatrixf(&rightHandleMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(rightHandleMtx))[0][0]);

    }

    void drawWheel() const {
        glColor3f(0.353f, 0.27f, 0.0f);
        // Draw the spokes
        for (float rad = 0; rad < M_PI; rad += M_PI / 3) {
            glm::mat4 spokeMtx = glm::rotate(glm::mat4(1.0f), rad, glm::vec3(1.0f, 0.0f, 0.0f));
            spokeMtx = glm::scale(spokeMtx, glm::vec3(CART_THICKNESS / 2, CART_WHEEL_RADIUS * 2 - CART_THICKNESS,
                                                      CART_THICKNESS / 2));
            glMultMatrixf(&spokeMtx[0][0]);
            {
                CSCI441::drawSolidCube(1.0f);
            }
            glMultMatrixf(&(glm::inverse(spokeMtx))[0][0]);
        }

        glColor3f(0.3059f, 0.3059f, 0.3059f);
        // Draw the tire
        glm::mat4 tireMtx = glm::rotate(glm::mat4(1.0f), GLfloat(M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(&tireMtx[0][0]);
        {
            CSCI441::drawSolidTorus(CART_THICKNESS, CART_WHEEL_RADIUS - CART_THICKNESS / 2, 20, 20);
        }
        glMultMatrixf(&(glm::inverse(tireMtx))[0][0]);
    }

    void drawCart() const {
        // Draw body
        drawCartBody();
        // Draw left wheel
        glm::mat4 leftWhlMtx = glm::translate(glm::mat4(1.0f),
                                              glm::vec3(CART_WIDTH / 2 + CART_THICKNESS, 0.0f, 0.0f));
        leftWhlMtx = glm::rotate(leftWhlMtx, leftWhlTheta, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&leftWhlMtx[0][0]);
        {
            drawWheel();
        }
        glMultMatrixf(&(glm::inverse(leftWhlMtx))[0][0]);
        // Draw right wheel
        glm::mat4 rightWhlMtx = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(-CART_WIDTH / 2 - CART_THICKNESS, 0.0f, 0.0f));
        rightWhlMtx = glm::rotate(rightWhlMtx, rightWhlTheta, glm::vec3(1.0f, 0.0f, 0.0f));
        glMultMatrixf(&rightWhlMtx[0][0]);
        {
            drawWheel();
        }
        glMultMatrixf(&(glm::inverse(rightWhlMtx))[0][0]);

    }

    void drawCartAndHorse() const {
        glm::mat4 cartMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f / 2));
        glMultMatrixf(&cartMtx[0][0]);
        {
            drawCart();
        }
        glMultMatrixf(&(glm::inverse(cartMtx))[0][0]);
        glm::mat4 horseMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, HORSE_LEG_SIZE / 2, 1.5f / 2));
        glMultMatrixf(&horseMtx[0][0]);
        {
            drawHorse();
        }
        glMultMatrixf(&(glm::inverse(horseMtx))[0][0]);

    }

    // Animation variables
    float legTheta1 = 0.0f;
    float legTheta2 = M_PI / 4;
    float legRate = LEG_THETA_RATE;
    float leftWhlTheta = 0.0f;
    float rightWhlTheta = 0.0f;

    // Orientation variables
    glm::vec3 orientation = glm::vec3(0.0f, 1.0f, 0.0f);
    float theta = 0.0f;

    // Location variables
    vector<glm::vec3> controlPoints;
    vector<float> bezCurveLookupTableD;
    vector<float> bezCurveLookupTableT;
    int lastUsedIndex = 1;
    float time;
    float distance = 0.0f;
    float minDist = -1;
    float maxDist = -1;
    glm::vec3 UP = glm::vec3(0.0f,0.0f,1.0f);

};

#endif //MP_HERO3_H
