//
// Created by zsmeton on 9/25/19.
//

#ifndef MP_HERO1_H
#define MP_HERO1_H

#include <GL/gl.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "HelperFunctions.h"
#include "HeroBase.h"
#include "BezierCurve.h"
#include "FaeryHero.h"

using namespace std;

class Cart : public HeroBase, public FaeryHero{
public:
    constexpr static const float CART_WIDTH = 1.0f;
    constexpr static const float CART_THICKNESS = 0.1f;
    constexpr static const float CART_WHEEL_RADIUS = 0.3;
    constexpr static const float HORSE_WIDTH = (CART_WIDTH - CART_THICKNESS * 2) / 2;
    constexpr static const float HORSE_LENGTH = 1.0f;
    constexpr static const float HORSE_LEG_ANGLE = 4 * M_PI / 3;
    constexpr static const float HORSE_LEG_SIZE = 0.2;
    constexpr static const float HORSE_KNECK_ANGLE = 3 * M_PI / 4;
    constexpr static const float HORSE_KNECK_LENGTH = 0.4;

    constexpr static const float VEL = FPS_ADJUSTMENT * 0.01f;
    constexpr static const float ANGULAR_VEL = FPS_ADJUSTMENT * 0.2f;
    constexpr static const float WHL_VEL = FPS_ADJUSTMENT * 0.5f;
    constexpr static const float WHL_TURN_VEL = WHL_VEL / 2;
    constexpr static const float LEG_THETA_RATE = FPS_ADJUSTMENT * 0.35;

    constexpr static const float FAERY_VEL = FPS_ADJUSTMENT * 0.05;
    constexpr static const float F_MAX = std::numeric_limits<float>::max();
    constexpr static const float FAERY_RAD_MAX = 0.6;

    glm::vec3 UP = glm::vec3(0.0f,1.0f,0.0f);

    Cart() {
        recomputeDirection();
        restrictBezierPosition();
    }

    void draw() override {
        glm::vec3 rotAxis =glm::cross(UP, orientation);
        float rad = acos(glm::dot( glm::normalize(orientation), glm::normalize(UP)));
        // Translate to the position
        worldMtx = glm::translate(glm::mat4(1.0f), getPos());
        // Rotate to match the terrain
        worldMtx = glm::rotate(worldMtx, rad, rotAxis);
        // Translate upwards to keep wheels out of ground
        worldMtx = glm::translate(worldMtx, glm::vec3(0.0f,CART_WHEEL_RADIUS+CART_THICKNESS/10, 0.0f));
        // Orient along direction
        worldMtx = glm::rotate(worldMtx, theta, glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(&worldMtx[0][0]);
        {
            drawCharacterAndFaery();
        }
        glMultMatrixf(&(glm::inverse(worldMtx))[0][0]);

    }

    void update() override {
        if(controlPoints.size() !=0) {
            updateFaery();
        }
    }

    void moveForward() {
        bezierPosition += direction * VEL;
        leftWhlTheta += WHL_VEL;
        rightWhlTheta += WHL_VEL;
        moveLegs();
        restrictBezierPosition();
    }

    void moveBackward() {
        bezierPosition -= direction * VEL;
        leftWhlTheta += WHL_VEL;
        rightWhlTheta += WHL_VEL;
        moveLegs(true);
        restrictBezierPosition();
    }

    void turnLeft() {
        theta += ANGULAR_VEL;
        leftWhlTheta -= WHL_TURN_VEL;
        rightWhlTheta += WHL_TURN_VEL;
        recomputeDirection();
    }

    void turnRight() {
        theta -= ANGULAR_VEL;
        leftWhlTheta += WHL_TURN_VEL;
        rightWhlTheta -= WHL_TURN_VEL;
        recomputeDirection();
    }

    const glm::vec3 &getBezierPosition() const {
        return bezierPosition;
    }

    const vector<glm::vec3> &getControlPoints() const {
        return controlPoints;
    }

    void setControlPoints(const vector<glm::vec3> &controlPoints) {
        this->controlPoints = controlPoints;
    }

    glm::vec3 &getDirection(){
        return direction;
    }

    void setDirection(const glm::vec3 &direction) {
        this->direction = direction;
    }

    void setMaxX(float maxX) {
        max_x = maxX;
    }

    void setMaxZ(float maxZ) {
        max_z = maxZ;
    }

    const glm::vec3 &getOrientation() const {
        return orientation;
    }

    void setOrientation(const glm::vec3 &orientation) {
        this->orientation = orientation;
    }

    void setFaeryPath(const vector<glm::vec3>& controlPoints) override{
        this->controlPoints = controlPoints;
    }

private:
    /*!
     * This function updates the vehicles's velocity in cartesian coordinates based
     * on its direction in spherical coordinates. Should be called every time
     * vehicleTheta is changed.
     */
    void recomputeDirection() {
        // Convert spherical coordinates into a cartesian vector
        GLfloat x = cos(theta);
        GLfloat z = sin(theta);
        GLfloat y = 0.0;
        direction = glm::vec3(x, y, z);
        // and NORMALIZE this directional vector!!!
        direction = glm::normalize(direction);
    }

    /*!
     * This function keeps the bezier position between 0 and 1
     */
    void restrictBezierPosition() {
        bezierPosition.x = restrictVariable<float>(bezierPosition.x, 0, max_x);
        bezierPosition.z = restrictVariable<float>(bezierPosition.z, 0, max_z);
    }

    void updateFaery() {
        faeryDt += FAERY_VEL;
        faeryTheta += faeryThetaRate;
        if (faeryDt >= F_MAX - 2 * FAERY_VEL) {
            faeryDt = fmod(faeryDt, F_MAX - 2 * FAERY_VEL);
            cout << "Well its been about 6 septillion years!!!" << endl;
        }
        if (faeryTheta >= FAERY_RAD_MAX || faeryTheta <= -FAERY_RAD_MAX) {
            faeryThetaRate = -faeryThetaRate;
        }

        glm::vec3 faeryHeading = computeRotationBezierCurve(controlPoints, faeryDt);
        faeryDir = glm::atan(faeryHeading.z, faeryHeading.x);
    }

    /*!
     * Updates the angle of the legs for a running like motion
     * @param negative move legs backwards
     */
    void moveLegs(bool negative = false) {
        float rate = legRate;
        if (negative) {
            legTheta1 += rate;
            legTheta2 -= rate;
        } else {
            legTheta1 -= rate;
            legTheta2 += rate;
        }
        if (legTheta1 > M_PI / 4 || legTheta2 < -M_PI / 4) {
            legRate = (negative) ? -LEG_THETA_RATE : LEG_THETA_RATE;
        } else if (legTheta2 > M_PI / 4 || legTheta1 < -M_PI / 4) {
            legRate = -((negative) ? -LEG_THETA_RATE : LEG_THETA_RATE);
        }
    }

    void drawFaeryBody() const {
        glColor3f(0.2f, 0.2f, 0.2f);
        glm::mat4 bodyMtx = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
        glMultMatrixf(&bodyMtx[0][0]);
        {
            CSCI441::drawSolidSphere(1.0f, 20, 20);
        }
        glMultMatrixf(&(glm::inverse(bodyMtx))[0][0]);
    }

    void drawFaeryWing() const {
        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.0f, 0.0f);
        glm::mat4 bodyMtx = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
        glMultMatrixf(&bodyMtx[0][0]);
        {
            glBegin(GL_TRIANGLE_STRIP);
            {
                glVertex3f(0.0f, 1.2f, -1.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(0.0f, 0.1f, -1.5f);
            }
            glEnd();
        }
        glMultMatrixf(&(glm::inverse(bodyMtx))[0][0]);
        glEnable(GL_LIGHTING);
    }

    void drawFaery() const {
        drawFaeryBody();
        glm::mat4 lWingMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 0.025f, -0.01f));
        lWingMtx = glm::rotate(lWingMtx, faeryTheta, glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(&lWingMtx[0][0]);
        {
            drawFaeryWing();
        }
        glMultMatrixf(&(glm::inverse(lWingMtx))[0][0]);
        glm::mat4 rWingMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f, 0.025f, -0.01f));
        rWingMtx = glm::rotate(rWingMtx, -faeryTheta, glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(&rWingMtx[0][0]);
        {
            drawFaeryWing();
        }
        glMultMatrixf(&(glm::inverse(rWingMtx))[0][0]);
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
        glColor3f(0.3569f, 0.278f, 0.137f);
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
        glColor3f(0.553f, 0.3686f, 0.051f);
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
        glColor3f(0.553f, 0.3686f, 0.051f);
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

    void drawCharacterAndFaery() const {
        drawCartAndHorse();
        if(controlPoints.size() != 0) {
            glm::mat4 faeryMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
            glm::vec3 faeryPos = computePositionBezierCurve(controlPoints, faeryDt);
            faeryMtx = glm::translate(faeryMtx, faeryPos);
            faeryMtx = glm::rotate(faeryMtx, static_cast<float>(-faeryDir + M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&faeryMtx[0][0]);
            {
                drawFaery();
            }
            glMultMatrixf(&(glm::inverse(faeryMtx))[0][0]);
        }
    }

    float legTheta1 = 0.0f;
    float legTheta2 = M_PI / 4;
    float legRate = LEG_THETA_RATE;
    float leftWhlTheta = 0.0f;
    float rightWhlTheta = 0.0f;
    float faeryDt = 0.2f;
    float faeryThetaRate = FPS_ADJUSTMENT * 0.25;
    float faeryTheta = 0.0f;
    float faeryDir = 0.0f;
    vector<glm::vec3> controlPoints;

    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 bezierPosition = glm::vec3(0.5f, 0.0f, 0.5f);
    glm::vec3 orientation = glm::vec3(0.0f, 1.0f, 0.0f);
    float theta = 0.0f;
    float max_x = 1;
    float max_z = 1;
};

#endif //MP_HERO1_H
