/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: RollerCoasterTycoon
 *  File: main.cpp
 *
 */

// include the OpenGL library header
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

#include <math.h>                // for cos(), sin() functionality
#include <stdio.h>            // for printf functionality
#include <stdlib.h>            // for exit functionality
#include <time.h>              // for time() functionality
#include <vector>               // for vector data structure
#include <bits/stdc++.h>        // For fmod()
#include <iostream>
#include <limits>
#include "BezierCurve.h"
using namespace std;

//*************************************************************************************
//
// Global Parameters

#ifndef M_PI
#define M_PI 3.14159
#endif

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 960, windowHeight = 720;

//  Movement speed
const float FPS_ADJUSTMENT = 0.2;

// Camera variables
float cameraDist = 10;                              // camera's distance from the player
glm::vec3 camPos;                                    // camera position in cartesian coordinates
float cameraTheta, cameraPhi;                    // camera DIRECTION in spherical coordinates
const float CAM_SPEED = FPS_ADJUSTMENT*0.2f;                       // camera's rate of position change
const float CAM_ANGULAR_VEL = FPS_ADJUSTMENT*0.05f;                // camera's rate of direction change

// Environment drawing variables
GLuint environmentDL;                            // display list for the 'city'

const int MAP_SIZE = 50;
const int GRID_START = -MAP_SIZE;
const int GRID_END = MAP_SIZE;
const float DRAW_RANDOM_TOLERANCE = 0.1;                 // The chance an environment object is drawn
const float BUILDING_HEIGHT = 10;                        // The maximum size of the building
const float BUSH_SIZE = 1;                               // The maximum size of the bush
const int BUSH_ITERATION = 7;                           // The number of iterations to draw the bush
const float ROCK_SIZE = 0.5;                               // The maximum size of the rock
const int ROCK_ITERATION = 3;                            // The number of iterations to draw the rock

enum EnvironmentObject {
    BUILDING = 0, BUSH = 1, ROCK = 2, LAST
}; // enum of all the possible objects to draw

// Vehicle drawing variables
float cartAndHorseTheta;                             // vehicles DIRECTION in spherical coordinates
const float CART_WHEEL_RADIUS = 0.3;
const float CART_WIDTH = 1.0f;
const float CART_THICKNESS = 0.1f;                   // Thickness of the the vehicle's parts
const float HORSE_WIDTH = (CART_WIDTH - CART_THICKNESS * 2) / 2;
const float HORSE_LENGTH = 1.0f;
const float HORSE_LEG_ANGLE = 4 * M_PI / 3;
const float HORSE_LEG_SIZE = 0.2;
const float HORSE_KNECK_ANGLE = 3 * M_PI / 4;
const float HORSE_KNECK_LENGTH = 0.4;
float legTheta1 = 0.0f;
float legTheta2 = M_PI/4;
const float legThetaRate = FPS_ADJUSTMENT*0.1;
float legRate = legThetaRate;
glm::vec3 cartAndHorsePos = glm::vec3(10.0f, 0.0f, 10.0f); // vehicles location in the world
glm::vec3 cartAndHorseDir = glm::vec3(0.0f, 0.0f, 0.0f);
// Jump constants
bool jump;
const float JUMP_HEIGHT = 0.6f;
const float JUMP_RATE = FPS_ADJUSTMENT*0.05f;
float jumpingRate = JUMP_RATE;

// Avg is for non-shift movements
// max is for shift movements
const float AVG_VEL = FPS_ADJUSTMENT*0.1f;
const float MAX_VEL = FPS_ADJUSTMENT*0.2f;
const float ANGULAR_AVG_VEL = FPS_ADJUSTMENT*0.04f;
const float ANGULAR_MAX_VEL = FPS_ADJUSTMENT*0.06f;
float leftWhlTheta = 0;
float rightWhlTheta = 0;
float WHL_AVG_VEL = FPS_ADJUSTMENT*0.2f;
float WHL_MAX_VEL = FPS_ADJUSTMENT*0.4f;

// Variables for the faery and its flight path
vector<glm::vec3> controlPoints;
const GLfloat CONTROL_POINT_RAD = 0.1f;
bool showControlPoints = true;
bool showCurve = true;

float faeryDt = 0.2;
const float FAERY_VEL = FPS_ADJUSTMENT*0.01;
const float F_MAX = std::numeric_limits<float>::max();
float faeryThetaRate = FPS_ADJUSTMENT * 0.25;
float faeryTheta = 0.0;
const float FAERY_RAD_MAX = 0.6;
float faeryDir;

// Interaction variables
// camera movement
int leftMouseButton;                                // status of the mouse button
glm::vec2 mousePos;                                    // last known X and Y of the mouse
// vehicle movement
int wKey;                                           // status of the 'W' key
int aKey;                                           // status of the 'A' key
int sKey;                                           // status of the 'S' key
int dKey;                                           // status of the 'D' key
int shiftKey;                                       // status of the 'Shift' Key
int ctrlKey;                                        // status of the 'Ctrl' Key
int spaceKey;                                       // status of the 'Space' Key
//*************************************************************************************
//
// Helper Functions
////////////////////////////////////////////////////////////////////////////////

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
float getRand() { return rand() / (float) RAND_MAX; }

// loadControlPoints() /////////////////////////////////////////////////////////
//
//  Load our control points from file and store them in
//	the global variable controlPoints
//
////////////////////////////////////////////////////////////////////////////////
bool loadControlPoints(char *filename) {
    // Read in control points from file.  Make sure the file can be
    // opened and handle it appropriately.  return false if there is an error
    FILE *file;
    // Check if the file can be opened
    if ((file = fopen(filename, "r"))) {
        // Read in the number of points
        int numPoints;
        fscanf(file, "%d", &numPoints);
        // Read in all the points
        for (int i = 0; i < numPoints; i++) {
            int x, y, z;
            fscanf(file, "%d,%d,%d", &x, &y, &z);
            controlPoints.emplace_back(x, y, z);
        }
        return true;
    }
    return false;
}


// computePosition() //////////////////////////////////////////////////////
//
// This function returns the camera's direction in cartesian coordinates based
//  on the passed in spherical coordinates and the cam distance.
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 computePosition(float theta, float phi, float distance) {
    GLfloat x = distance * sin(theta) * sin(phi);
    GLfloat y = -distance * cos(phi);
    GLfloat z = -distance * cos(theta) * sin(phi);
    return glm::vec3(x, y, z);
}

// recomputePosition() //////////////////////////////////////////////////////
//
// This function updates the camera's direction in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputePosition() {
    // Convert spherical coordinates into a cartesian vector
    camPos = computePosition(cameraTheta, cameraPhi, cameraDist);
}

// recomputeDirection() //////////////////////////////////////////////////////
//
// This function updates the vehicles's velocity in cartesian coordinates based
//  on its direction in spherical coordinates. Should be called every time
//  vehicleTheta is changed.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeDirection() {
    // Convert spherical coordinates into a cartesian vector
    GLfloat x = sin(cartAndHorseTheta);
    GLfloat z = cos(cartAndHorseTheta);
    GLfloat y = 0.0;
    cartAndHorseDir = glm::vec3(x, y, z);
    // and NORMALIZE this directional vector!!!
    cartAndHorseDir = glm::normalize(cartAndHorseDir);
}

template<typename T>
T restrictVariable(T data, T min, T max) {
    data = (data < min) ? min : data;
    data = (data > max) ? max : data;
    return data;
}

// restrictVehiclePos() //////////////////////////////////////////////////////
//
// This function updates the vehicles's height in the world based on the
//      vehicle's position in x,z and the maps topography (y pos)
//
////////////////////////////////////////////////////////////////////////////////
void restrictVehiclePos() {
    // Find the angle the vehicle should be at using maps y at the left and right wheel location
    cartAndHorsePos.x = restrictVariable<float>(cartAndHorsePos.x, float(GRID_START), float(GRID_END));
    cartAndHorsePos.z = restrictVariable<float>(cartAndHorsePos.z, float(GRID_START), float(GRID_END));
}

// Updates the angle of the legs for a running like motion
void moveLegs(bool negative, bool faster){
    float rate = legRate;
    if(faster){
        rate *= 2;
    }
    if(negative){
        legTheta1 += rate;
        legTheta2 -= rate;
    }else {
        legTheta1 -= rate;
        legTheta2 += rate;
    }
    if(legTheta1 > M_PI/4 || legTheta2 < -M_PI/4){
        legRate = (negative) ? -legThetaRate : legThetaRate;
    }else if(legTheta2 > M_PI/4 || legTheta1 < -M_PI/4){
        legRate = -((negative) ? -legThetaRate : legThetaRate);
    }
}


//*************************************************************************************
//
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback(int error, const char *description) {
    fprintf(stderr, "[ERROR]: %s\n", description);
}

// ESC/Q: exit program
// 1-4: will update minicam model
// W/A/S/D/Shift/Ctrl : updates key variables
static void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                exit(EXIT_SUCCESS);
            case GLFW_KEY_B:
                showCurve = !showCurve;
                break;
            case GLFW_KEY_C:
                showControlPoints = !showControlPoints;
                break;
        }
    }

    // Update key status
    switch (key) {
        case GLFW_KEY_W:
            wKey = action;
            break;
        case GLFW_KEY_A:
            aKey = action;
            break;
        case GLFW_KEY_S:
            sKey = action;
            break;
        case GLFW_KEY_D:
            dKey = action;
            break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            shiftKey = action;
            break;
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
            ctrlKey = action;
            break;
        case GLFW_KEY_SPACE:
            spaceKey = action;
            break;
    }
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//  GLFW callback for mouse movement. We update cameraPhi and/or cameraTheta
//      based on how much the user has moved the mouse in the
//      X or Y directions (in screen space) and whether they have held down
//      the left or right mouse buttons. If the user hasn't held down any
//      buttons, the function just updates the last seen mouse X and Y coords.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback(GLFWwindow *window, double x, double y) {
    if (leftMouseButton == GLFW_PRESS) {
        // If shift drag change camera position
        if (ctrlKey == GLFW_PRESS || ctrlKey == GLFW_REPEAT) {
            cameraDist += CAM_SPEED * (y - mousePos.y);
            cameraDist = restrictVariable(cameraDist, 2.0f, 15.0f);

        } else {
            // Change the yaw based on changes in the X position
            cameraTheta -= CAM_ANGULAR_VEL * (x - mousePos.x);
            // Change the pitch based on changes in the X position
            cameraPhi += CAM_ANGULAR_VEL * (y - mousePos.y);
            // Make sure pitch is within Pi/2 and Pi
            cameraPhi = restrictVariable(cameraPhi, static_cast<float>(M_PI / 2), static_cast<float>(M_PI));
        }
    }
    recomputePosition();           // update camera (x,y,z) based on (theta,phi)
    mousePos.x = x;
    mousePos.y = y;
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//  GLFW callback for mouse clicks. We save the state of the mouse button
//      when this is called so that we can check the status of the mouse
//      buttons inside the motion callback (whether they are up or down).
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftMouseButton = action;
    }
}

//*************************************************************************************
//
// Update Functions - this is where the movement happens

void updateVehiclePos() {
    // If shift is pressed us sprint
    float vel = AVG_VEL;
    float whlVel = WHL_AVG_VEL;
    float angular = ANGULAR_AVG_VEL;
    bool faster = false;
    if (shiftKey == GLFW_PRESS || shiftKey == GLFW_REPEAT) {
        vel = MAX_VEL;
        whlVel = WHL_MAX_VEL;
        angular = ANGULAR_MAX_VEL;
        faster = true;
    }
    if (wKey == GLFW_PRESS || wKey == GLFW_REPEAT) {
        // Move forward
        cartAndHorsePos += cartAndHorseDir * vel;
        // Adjust postion based on map
        restrictVehiclePos();
        // Rotate wheels forward
        leftWhlTheta += whlVel;
        rightWhlTheta += whlVel;
        // Move horse's legs
        moveLegs(false, faster);
    }
    if (sKey == GLFW_PRESS || sKey == GLFW_REPEAT) {
        cartAndHorsePos -= cartAndHorseDir * vel;
        // Adjust postion based on map
        restrictVehiclePos();
        leftWhlTheta -= whlVel;
        rightWhlTheta -= whlVel;
        moveLegs(true, faster);
    }
    if (aKey == GLFW_PRESS || aKey == GLFW_REPEAT) {
        cartAndHorseTheta += angular;
        leftWhlTheta -= whlVel / 3;
        rightWhlTheta += whlVel / 3;
        recomputeDirection();
        restrictVehiclePos();
    }
    if (dKey == GLFW_PRESS || dKey == GLFW_REPEAT) {
        cartAndHorseTheta -= angular;
        leftWhlTheta += whlVel / 3;
        rightWhlTheta -= whlVel / 3;
        recomputeDirection();
        restrictVehiclePos();
    }
    if (spaceKey == GLFW_PRESS || spaceKey == GLFW_REPEAT) {
        jump = true;
        moveLegs(false, faster);
    }
}

void updateJump(){
    if(jump){
        cartAndHorsePos.y += jumpingRate;
        if(cartAndHorsePos.y > JUMP_HEIGHT ){
            jumpingRate = -JUMP_RATE;
        }else if(cartAndHorsePos.y <= JUMP_RATE){
            jumpingRate = JUMP_RATE;
            jump = false;
        }
    }
}

void updateFaery(){
    faeryDt += FAERY_VEL;
    faeryTheta += faeryThetaRate;
    if(faeryDt >= F_MAX - 2*FAERY_VEL){
        faeryDt = fmod(faeryDt,F_MAX - 2*FAERY_VEL);
        cout<<"Well its been about 6 septillion years!!!"<<endl;
    }if(faeryTheta >= FAERY_RAD_MAX || faeryTheta <= -FAERY_RAD_MAX){
        faeryThetaRate = - faeryThetaRate;
    }

    glm::vec3 faeryHeading= computeRotationBezierCurve(controlPoints, faeryDt);
    faeryDir = glm::atan(faeryHeading.z, faeryHeading.x);
}

// update() /////////////////////////////////////////////////////
//
//  The main update loop, called every render, calls all other update functions
//
////////////////////////////////////////////////////////////////////////////////
void update() {
    updateVehiclePos();
    updateJump();
    updateFaery();
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void drawFaeryBody(){
    glColor3f(0.2f,0.2f,0.2f);
    glm::mat4 bodyMtx = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f,0.1f,0.1f));
    glMultMatrixf(&bodyMtx[0][0]);
    {
        CSCI441::drawSolidSphere(1.0f, 20,20);
    }
    glMultMatrixf(&(glm::inverse(bodyMtx))[0][0]);
}

void drawFaeryWing(){
    glDisable(GL_LIGHTING);
    glColor3f(0.0f,0.0f,0.0f);
    glm::mat4 bodyMtx = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f,0.1f,0.1f));
    glMultMatrixf(&bodyMtx[0][0]);
    {
        glBegin(GL_TRIANGLE_STRIP);{
            glVertex3f(0.0f,1.2f,-1.0f);
            glVertex3f(0.0f,0.0f,0.0f);
            glVertex3f(0.0f,0.1f,-1.5f);
        }glEnd();
    }
    glMultMatrixf(&(glm::inverse(bodyMtx))[0][0]);
    glEnable(GL_LIGHTING);
}

void drawFaery(){
    drawFaeryBody();
    glm::mat4 lWingMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f,0.025f,-0.01f));
    lWingMtx = glm::rotate(lWingMtx, faeryTheta,glm::vec3(0.0f,1.0f,0.0f));
    glMultMatrixf(&lWingMtx[0][0]);
    {
        drawFaeryWing();
    }
    glMultMatrixf(&(glm::inverse(lWingMtx))[0][0]);
    glm::mat4 rWingMtx = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f,0.025f,-0.01f));
    rWingMtx = glm::rotate(rWingMtx, -faeryTheta,glm::vec3(0.0f,1.0f,0.0f));
    glMultMatrixf(&rWingMtx[0][0]);
    {
        drawFaeryWing();
    }
    glMultMatrixf(&(glm::inverse(rWingMtx))[0][0]);
}


void drawFaeryPath(){
    if(showCurve){
        glColor3f(0.0f,0.0f,1.0f);
        drawBezierCurve(controlPoints);
    }
    if(showControlPoints) {
        glColor3f(1.0f, 1.0f, 0.0f);
        drawControlPointConnections(controlPoints);
        glColor3f(0.0f, 1.0f, 0.0f);
        drawControlPoints(controlPoints, CONTROL_POINT_RAD);
    }
}

void drawHorseBody() {
    // Calculate horse Width
    glm::mat4 horseMtx = glm::scale(glm::mat4(1.0f), glm::vec3(HORSE_WIDTH, HORSE_WIDTH, HORSE_LENGTH));
    glMultMatrixf(&horseMtx[0][0]);
    {
        CSCI441::drawSolidCube(1.0f);
    }
    glMultMatrixf(&(glm::inverse(horseMtx))[0][0]);
}


void drawHorseLeg() {
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

void drawHorseHeadAndKneck() {
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

void drawHorse() {
    glColor3f(0.3569f, 0.278f, 0.137f);
    drawHorseBody();
    // Draw left front leg
    glm::mat4 lFLegMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(HORSE_WIDTH / 2 + CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                  2 * HORSE_LENGTH / 5));
    lFLegMtx = glm::rotate(lFLegMtx, legTheta1, glm::vec3(1.0f,0.0f,0.0f));
    glMultMatrixf(&lFLegMtx[0][0]);
    {
        drawHorseLeg();
    }
    glMultMatrixf(&(glm::inverse(lFLegMtx))[0][0]);
    // Draw right front leg
    glm::mat4 rFLegMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(-HORSE_WIDTH / 2 - CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                  2 * HORSE_LENGTH / 5));
    rFLegMtx = glm::rotate(rFLegMtx, legTheta2, glm::vec3(1.0f,0.0f,0.0f));
    glMultMatrixf(&rFLegMtx[0][0]);
    {
        drawHorseLeg();
    }
    glMultMatrixf(&(glm::inverse(rFLegMtx))[0][0]);
    // Draw left back leg
    glm::mat4 lBLegMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(HORSE_WIDTH / 2 + CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                  -HORSE_LENGTH / 4));
    lBLegMtx = glm::rotate(lBLegMtx, legTheta2, glm::vec3(1.0f,0.0f,0.0f));
    glMultMatrixf(&lBLegMtx[0][0]);
    {
        drawHorseLeg();
    }
    glMultMatrixf(&(glm::inverse(lBLegMtx))[0][0]);
    // draw right back leg
    glm::mat4 rBLegMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(-HORSE_WIDTH / 2 - CART_THICKNESS / 2, -HORSE_WIDTH / 2,
                                                  -HORSE_LENGTH / 4));
    rBLegMtx = glm::rotate(rBLegMtx, legTheta1, glm::vec3(1.0f,0.0f,0.0f));
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

void drawCartBody() {
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

void drawWheel() {
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

// Draws the vehicle (a cart)
void drawCart() {
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

void drawCartAndHorse() {
    glm::mat4 cartMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f/2));
    glMultMatrixf(&cartMtx[0][0]);
    {
        drawCart();
    }   glMultMatrixf(&(glm::inverse(cartMtx))[0][0]);
    glm::mat4 horseMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, HORSE_LEG_SIZE / 2, 1.5f/2));
    glMultMatrixf(&horseMtx[0][0]);
    {
        drawHorse();
    }
    glMultMatrixf(&(glm::inverse(horseMtx))[0][0]);

}

void drawCharacterAndFaery(){
    drawCartAndHorse();
    glColor3f(0.0f,0.0f,1.0f);

    glm::mat4 faeryMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,-0.5f,0.0f));
    glMultMatrixf(&faeryMtx[0][0]);
    {
        drawFaeryPath();
    }
    glMultMatrixf(&(glm::inverse(faeryMtx))[0][0]);
    glm::vec3 faeryPos = computePositionBezierCurve(controlPoints, faeryDt);
    faeryMtx = glm::translate(faeryMtx, faeryPos);
    faeryMtx = glm::rotate(faeryMtx, static_cast<float>(-faeryDir+M_PI/2), glm::vec3(0.0f,1.0f,0.0f));
    glMultMatrixf(&faeryMtx[0][0]);
    {
        drawFaery();
    }
    glMultMatrixf(&(glm::inverse(faeryMtx))[0][0]);

}


// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void drawGrid() {
    glDisable(GL_LIGHTING);
    // Draw the map in the xz plane using perlin noise to generate y position
    // Creating a realistic topography
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = GRID_START + 1; i < GRID_END + 1; i++) {
        for (int j = GRID_START + 1; j < GRID_END + 1; j++) {
            // Draw lines running along x-axis
            glBegin(GL_LINES);
            {
                glVertex3f(i - 1, 0.0f, j);
                glVertex3f(i, 0.0f, j);
            }
            glEnd();
            glBegin(GL_LINES);
            {
                glVertex3f(i - 1, 0.0f, j - 1);
                glVertex3f(i, 0.0f, j - 1);
            }
            glEnd();
            // Draw lines running along z-axis
            glBegin(GL_LINES);
            {
                glVertex3f(i, 0.0f, j - 1);
                glVertex3f(i,0.0f, j);
            }
            glEnd();
            glBegin(GL_LINES);
            {
                glVertex3f(i - 1, 0.0f, j - 1);
                glVertex3f(i - 1, 0.0f, j);
            }
            glEnd();
        }
    }
    glEnable(GL_LIGHTING);
}

// Draw a "house" a rectangle of random color and height
// At the x and z location at the map height
void drawRandomHouse(float x, float z) {
    // Choose random color
    glColor3f(getRand(), getRand(), getRand());
    // Choose random height
    float buildingHeight = getRand() * BUILDING_HEIGHT + 1;
    glm::mat4 buildingMtx = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(x, buildingHeight / 2, z));
    buildingMtx = glm::scale(buildingMtx, glm::vec3(1.0f, buildingHeight, 1.0f));
    glMultMatrixf(&buildingMtx[0][0]);
    {
        CSCI441::drawSolidCube(1.0f);
    }
    glMultMatrixf(&(glm::inverse(buildingMtx))[0][0]);
}

// Draw a bush of random size and random shade of green
void drawBush(float x, float z) {
    // Choose random color
    for (int i = 0; i < BUSH_ITERATION; i++) {
        glColor3f(0.0f, getRand() * 255, 0.0f);
        // Choose random height
        float bushHeight = getRand() * BUSH_SIZE;
        glm::mat4 bushMtx = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(x, bushHeight / 2, z));
        bushMtx = glm::rotate(bushMtx, float(getRand() * M_PI), glm::vec3(getRand(), getRand(), getRand()));
        bushMtx = glm::scale(bushMtx, glm::vec3(bushHeight, bushHeight, bushHeight));
        glMultMatrixf(&bushMtx[0][0]);
        {
            CSCI441::drawWireCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(bushMtx))[0][0]);
    }
}

// Draws a grey rock at the location
void drawRock(float x, float z) {
    // Create the rock by generating rocks of random size
    // and rotating them to a random angle creating a conglomerate of square
    float grey = 0.2118;
    for (int i = 0; i < ROCK_ITERATION; i++) {
        glColor3f(grey, grey, grey);
        // Choose random size
        float rockHeight = getRand() * ROCK_SIZE;
        glm::mat4 rockMtx = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(x, rockHeight / 2, z));
        rockMtx = glm::rotate(rockMtx, float(getRand() * M_PI), glm::vec3(getRand(), getRand(), getRand()));
        rockMtx = glm::scale(rockMtx, glm::vec3(rockHeight, rockHeight, rockHeight));
        glMultMatrixf(&rockMtx[0][0]);
        {
            CSCI441::drawSolidCube(1.0f);
        }
        glMultMatrixf(&(glm::inverse(rockMtx))[0][0]);
    }
}

// drawEnvironment() //////////////////////////////////////////////////////////////////
//
//  Function to draw a random environment using CSCI441 3D Cubes
//
////////////////////////////////////////////////////////////////////////////////
void drawEnvironment() {
    // Randomly place random objects
    for (int i = GRID_START + 1; i < GRID_END + 1; i++) {
        for (int j = GRID_START + 1; j < GRID_END + 1; j++) {
            // Draw object if coordinates are even and getRand() is
            // less than BUILDING_RANDOM_TOLERANCE
            if (abs(i) % 2 == 0 && abs(j) % 2 == 0 && getRand() < DRAW_RANDOM_TOLERANCE) {
                // Chose which enivironment objec
                EnvironmentObject toDraw = static_cast<EnvironmentObject>(rand() % LAST);
                switch (toDraw) {
                    case BUILDING:
                        drawRandomHouse(i, j);
                        break;
                    case BUSH:
                        drawBush(i, j);
                        break;
                    case ROCK:
                        drawRock(i, j);
                        break;

                }
            }
        }
    }
}

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates a display list with the code to draw a simple
//      environment for the user to navigate through.
//
//  And yes, it uses a global variable for the display list.
//  I know, I know! Kids: don't try this at home. There's something to be said
//      for object-oriented programming after all.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironmentDL() {
    // Create a Display List & Call our Drawing Functions
    environmentDL = glGenLists(GLsizei(1));
    glNewList(environmentDL, GL_COMPILE);
    {
        drawGrid();
        drawEnvironment();
    }
    glEndList();

}

//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene(void) {
    // CREATE A CITY SCENE ON A GRID...but call it's display list!
    // Tell OpenGL to execute out display list
    glCallList(environmentDL);
    update();
    // Draw vehicle
    glm::mat4 vehicleMtx = glm::translate(glm::mat4(1.0f), cartAndHorsePos + glm::vec3(0.0f, CART_WHEEL_RADIUS, 0.0f));
    vehicleMtx = glm::rotate(vehicleMtx, cartAndHorseTheta, glm::vec3(0.0f, 1.0f, 0.0f));

    glMultMatrixf(&vehicleMtx[0][0]);{
        drawCharacterAndFaery();
    }
    glMultMatrixf(&(glm::inverse(vehicleMtx))[0][0]);
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow *setupGLFW() {
    // set what function to use when registering errors
    // this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
    // all other GLFW calls must be performed after GLFW has been initialized
    glfwSetErrorCallback(error_callback);

    // initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "[ERROR]: Could not initialize GLFW\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "[INFO]: GLFW initialized\n");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);    // request OpenGL v2.X
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);    // request OpenGL v2.1
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);        // do not allow our window to be able to be resized

    // create a window for a given size, with a given title
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Driving Simulator", NULL, NULL);
    if (!window) {                        // if the window could not be created, NULL is returned
        fprintf(stderr, "[ERROR]: GLFW Window could not be created\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "[INFO]: GLFW Window created\n");
    }

    glfwMakeContextCurrent(window);        // make the created window the current window
    glfwSwapInterval(1);                        // update our screen after at least 1 screen refresh

    glfwSetKeyCallback(window, keyboard_callback);                            // set our keyboard callback function
    glfwSetCursorPosCallback(window, cursor_callback);                    // set our cursor position callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);    // set our mouse button callback function

    return window;                               // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
    // tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
    //		surface removal.  We will discuss this more very soon.
    glEnable(GL_DEPTH_TEST);

    //******************************************************************
    // this is some code to enable a default light for the scene;
    // feel free to play around with this, but we won't talk about
    // lighting in OpenGL for another couple of weeks yet.
    GLfloat lightCol[4] = {1, 1, 1, 1};
    GLfloat ambientCol[4] = {0.0, 0.0, 0.0, 1.0};
    GLfloat lPosition[4] = {10, 10, 10, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientCol);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // tell OpenGL not to use the material system; just use whatever we
    // pass with glColor*()
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_SCISSOR_TEST);
    //******************************************************************

    // tells OpenGL to blend colors across triangles. Once lighting is
    // enabled, this means that objects will appear smoother - if your object
    // is rounded or has a smooth surface, this is probably a good idea;
    // if your object has a blocky surface, you probably want to disable this.
    glShadeModel(GL_SMOOTH);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    // set the clear color to black
}

//
//  void setupScene()
//
//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
void setupScene() {
    // give the camera a scenic starting point.
    camPos.x = 60;
    camPos.y = 40;
    camPos.z = 30;
    cameraPhi = 3 * M_PI / 5;
    cameraTheta = 0;
    cartAndHorseTheta = 0;
    recomputeDirection();
    recomputePosition();
    restrictVehiclePos();

    srand(time(NULL));    // seed our random number generator
    generateEnvironmentDL();
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main(int argc, char *argv[]) {
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();    // initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();                                        // initialize all of the OpenGL specific information
    setupScene();                                            // initialize objects in our scene

    fprintf(stdout, "[INFO]: /--------------------------------------------------------\\\n");
    fprintf(stdout, "[INFO]: | OpenGL Information                                     |\n");
    fprintf(stdout, "[INFO]: |--------------------------------------------------------|\n");
    fprintf(stdout, "[INFO]: |   OpenGL Version:  %35s |\n", glGetString(GL_VERSION));
    fprintf(stdout, "[INFO]: |   OpenGL Renderer: %35s |\n", glGetString(GL_RENDERER));
    fprintf(stdout, "[INFO]: |   OpenGL Vendor:   %35s |\n", glGetString(GL_VENDOR));
    fprintf(stdout, "[INFO]: \\--------------------------------------------------------/\n");

    // Prompt user to enter a file name.  Then read the points from file
    char fileName[100];
    fprintf(stdout, "Please enter a file name:");
    gets(fileName);
    while (!loadControlPoints(fileName)) {
        fprintf(stdout, "The file %35s is invalid.\n", fileName);
        fprintf(stdout, "Please enter a file name:");
        gets(fileName);
    }
    //loadControlPoints("controlPoints4.csv");

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while (!glfwWindowShouldClose(window)) {    // check if the window was instructed to be closed
        glDrawBuffer(GL_BACK);                // work with our back frame buffer
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);    // clear the current color contents and depth buffer in the window

        // update the projection matrix based on the window size
        // the GL_PROJECTION matrix governs properties of the view coordinates;
        // i.e. what gets seen - use a perspective projection that ranges
        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
        glm::mat4 projMtx = glm::perspective(45.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.001f, 1000.0f);
        glMatrixMode(GL_PROJECTION);    // change to the Projection matrix
        glLoadIdentity();                // set the matrix to be the identity
        glMultMatrixf(&projMtx[0][0]);// load our orthographic projection matrix into OpenGL's projection matrix state

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glMatrixMode(GL_MODELVIEW);    // make the ModelView matrix current to be modified by any transformations
        glLoadIdentity();                            // set the matrix to be the identity

        // set up our look at matrix to position our camera
        glm::mat4 viewMtx = glm::lookAt(camPos + cartAndHorsePos,        // camera is located at (10, 10, 10)
                                        cartAndHorsePos,        // camera is looking at (0, 0, 0,)
                                        glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
        // multiply by the look at matrix - this is the same as our view martix
        glMultMatrixf(&viewMtx[0][0]);

        renderScene();                    // draw everything to the window

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();                // check for any events and signal to redraw screen
    }

    glfwDestroyWindow(window);// clean up and close our window
    glfwTerminate();                        // shut down GLFW to clean up our context

    return EXIT_SUCCESS;                // exit our program successfully!
}
