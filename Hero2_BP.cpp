//
// Created by linds on 9/29/2019.
//

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

#include "Hero2_BP.h"

using namespace std;
//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.





Snake::Snake() {
    fprintf(stdout, "Enter file name:");
    char filename[100];
    gets(filename);
    loadControlPoints_BP(filename);

    for (int i = 0; i < (controlPoints.size() - 3); i += 3) {
        for (int j = 0; j < 100; j++) {
            glm::vec3 point = evaluateBezierCurve_BP(controlPoints[i], controlPoints[i + 1], controlPoints[i + 2],
                                                     controlPoints[i + 3], float(j) / 100);
            faeryMovement.push_back(point);
        }
    }

    recomputeOrientation();
    recomputeSnakeOrientation();
}

//*************************************************************************************
//
// Helper Functions

bool Snake::loadControlPoints_BP( char* filename ) {

    ifstream inFile;
    inFile.open(filename);

    if(!inFile.is_open()){
        perror("Error opening file");
        return false;
    }

    int fileLength;
    string line;
    getline(inFile, line);
    fileLength = stoi(line);

    for(int i = 0; i < fileLength; i++){
        char c;
        glm::vec3 temp;
        getline(inFile,line);
        stringstream sin(line);
        sin >> temp[0] >> c >> temp[1] >> c >> temp[2];
        controlPoints.push_back(temp);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's direction in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////
void Snake::recomputeOrientation() {

    camDir.x = camRad * sin(cameraTheta) * sin(cameraPhi);
    camDir.y = camRad * cos(cameraPhi);
    camDir.z = (-1) * camRad * cos(cameraTheta) * sin(cameraPhi);
}

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

float Snake::getRand() { return rand() / (float)RAND_MAX; }

// evaluateBezierCurve_BP() ////////////////////////////////////////////////////////
//
// Computes a location along a Bezier Curve.
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 Snake::evaluateBezierCurve_BP( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
    glm::vec3 point(0,0,0);

    glm::vec3 myCoefficients[4];
    myCoefficients[0] = -p0 + glm::vec3(p1[0]*3,p1[1]*3,p1[2]*3) - glm::vec3(p2[0]*3,p2[1]*3,p2[2]*3) + p3;
    myCoefficients[0] *= pow(t,3);

    myCoefficients[1] = glm::vec3(p0[0]*3,p0[1]*3,p0[2]*3) - glm::vec3(p1[0]*6,p1[1]*6,p1[2]*6) + glm::vec3(p2[0]*3,p2[1]*3,p2[2]*3);
    myCoefficients[1] *= pow(t,2);

    myCoefficients[2] = -glm::vec3(p0[0]*3,p0[1]*3,p0[2]*3) + glm::vec3(p1[0]*3,p1[1]*3,p1[2]*3);
    myCoefficients[2] *= pow(t,1);

    myCoefficients[3] = p0;

    point = myCoefficients[0] + myCoefficients[1] + myCoefficients[2] + myCoefficients[3];


    return point;
}

// renderBezierCurve_BP() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void Snake::renderBezierCurve_BP( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution ) {

    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(0,0,1);
    for(int i = 0; i < resolution; i++){
        glm::vec3 point = evaluateBezierCurve_BP(p0, p1, p2, p3, float(i)/resolution);
        glVertex3f(point[0],point[1],point[2]);
    }
    glEnd();

}


//*************************************************************************************
//
// Event Callbacks
//
//	void error_callback( int error, const char* description )
//		Print error info to the terminal

//static void error_callback( int error, const char* description ) {
//    fprintf( stderr, "[ERROR]: %s\n", description );
//}
//
//static void Snake::keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
//
//    flickingTongue();
//
//    float speed = 1.25f;
//    glm::vec3 oldPos = snakePos;
//
//    if( action == GLFW_PRESS ) {
//        switch( key ) {
//            case GLFW_KEY_ESCAPE:
//            case GLFW_KEY_Q:
//                exit(EXIT_SUCCESS);
//
//                //move forward!
//            case GLFW_KEY_W:
//                snakePos -= speed * snakeDir;
//                break;
//
//                //move backwards!
//            case GLFW_KEY_S:
//                snakePos += speed * snakeDir;
//                break;
//            case GLFW_KEY_A:
//                snakeTheta -= 0.2;
//                recomputeSnakeOrientation();
//                break;
//            case GLFW_KEY_D:
//                snakeTheta += 0.2;
//                recomputeSnakeOrientation();
//                break;
//            case GLFW_KEY_RIGHT_CONTROL :
//            case GLFW_KEY_LEFT_CONTROL :
//                ctrlPress = true;
//                break;
//            case GLFW_KEY_R:  //toggles control points and boc
//                boxAndPointsToggle = !boxAndPointsToggle;
//                break;
//            case GLFW_KEY_F:  //toggles bezier curve
//                curveToggle = !curveToggle;
//                break;
//        }
//    }
//    if( action == GLFW_REPEAT ) {
//
//        flickingTongue();
//
//        switch( key ) {
//            case GLFW_KEY_ESCAPE:
//            case GLFW_KEY_Q:
//                exit(EXIT_SUCCESS);
//            case GLFW_KEY_W:
//                snakePos -= speed * snakeDir;
//                break;
//            case GLFW_KEY_S:
//                snakePos += speed * snakeDir;
//                break;
//            case GLFW_KEY_A:
//                snakeTheta -= 0.2;
//                recomputeSnakeOrientation();
//                break;
//            case GLFW_KEY_D:
//                snakeTheta += 0.2;
//                recomputeSnakeOrientation();
//                break;
//            case GLFW_KEY_RIGHT_CONTROL :
//            case GLFW_KEY_LEFT_CONTROL :
//                ctrlPress = true;
//                break;
//        }
//    }
//
//    if(snakePos.x > 40.0 ){
//        snakePos.x = 40;
//    }else if(snakePos.x  < -45.0){
//        snakePos.x = -45;
//    }
//    if(snakePos.z > 40.0) {
//        snakePos.z = 40;
//    }else if(snakePos.z < -45.0){
//        snakePos.z = -45;
//    }
//
//    if(action == GLFW_RELEASE && (key == GLFW_KEY_LEFT_CONTROL|| key == GLFW_KEY_RIGHT_CONTROL)){
//        ctrlPress = false;
//    }
//}
//
//// cursor_callback() ///////////////////////////////////////////////////////////
////
////  GLFW callback for mouse movement. We update cameraPhi and/or cameraTheta
////      based on how much the user has moved the mouse in the
////      X or Y directions (in screen space) and whether they have held down
////      the left or right mouse buttons. If the user hasn't held down any
////      buttons, the function just updates the last seen mouse X and Y coords.
////
//////////////////////////////////////////////////////////////////////////////////
//static void Snake::cursor_callback( GLFWwindow *window, double x, double y ) {
//
//    if(leftMouseButton == GLFW_PRESS && ctrlPress){
//        if((y - mousePosition.y ) > 0){
//            camRad += 2;
//        }else{
//            camRad -= 2;
//        }
//        recomputeOrientation();
//
//    }else if( leftMouseButton == GLFW_PRESS ) {
//        float changeXscaled = (0.005)* (mousePosition.x - x);
//        float changeYscaled = (0.005)* (mousePosition.y - y);
//
//        cameraTheta = cameraTheta + changeXscaled;
//        cameraPhi = cameraPhi - changeYscaled;
//
////		if(cameraPhi <= PI/2) {
////			cameraPhi = PI;
////		}else if(cameraPhi >= PI){
////			cameraPhi = 0;
////		}
//        recomputeOrientation();     // update camera (x,y,z) based on (theta,phi)
//    }
//
//    mousePosition.x = x;
//    mousePosition.y = y;
//}
//
//// mouse_button_callback() /////////////////////////////////////////////////////
//
//static void Snake::mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
//    if( button == GLFW_MOUSE_BUTTON_LEFT ) {
//        leftMouseButton = action;
//    }
//}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void Snake::drawGrid() {
    /*
     *	We will get to why we need to do this when we talk about lighting,
     *	but for now whenever we want to draw something with an OpenGL
     *	Primitive - like a line, triangle, point - we need to disable lighting
     *	and then re-enable it for use with the CSCI441 3D Objects.
     */
    glDisable( GL_LIGHTING );

    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f); //white
    for(int i = -50; i <= 50; i++){
        for(int j = -50; j <= 50; j++) {
            glVertex3f(i, 0.0f, j); glVertex3f(i+1,0.0f, j);
            glVertex3f(i, 0.0f, j); glVertex3f(i, 0.0f, j+1);
        }
    }
    glEnd();

    /*	As noted above, we are done drawing with OpenGL Primitives, so we
    *	must turn lighting back on.
    */
    glEnable( GL_LIGHTING );
}


void Snake::drawSnakeEye(){
    glColor3f(0.0f, 0.0f, 0.0f); //black
    CSCI441::drawSolidSphere(0.25, 30, 30);
}

void Snake::flickingTongue(){
    if(flickingTongueDir == 15 || flickingTongueDir == 25){
        tongueChange *= -1;
    }
    flickingTongueDir += tongueChange;
}

void Snake::drawSnakeTongue(){
    glColor3f(0.9f, 0.0f, 0.4f); //red for tongue

    glm::mat4 snakeTongueTrans1 = glm::translate( glm::mat4(1.0f), glm::vec3( 0.15, 10, 0 ) );
    glMultMatrixf( &snakeTongueTrans1[0][0] );{
        CSCI441::drawSolidCone(0.25, flickingTongueDir, 10, 10);
    }; glMultMatrixf( &(glm::inverse( snakeTongueTrans1 ))[0][0] );

    glm::mat4 snakeTongueTrans2 = glm::translate( glm::mat4(1.0f), glm::vec3( -0.15, 10, 0 ) );
    glMultMatrixf( &snakeTongueTrans2[0][0] );{
        CSCI441::drawSolidCone(0.25, flickingTongueDir, 10, 10);
    }; glMultMatrixf( &(glm::inverse( snakeTongueTrans2 ))[0][0] );
}

void Snake::drawSnakeHead(){
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeHeadTrans1 = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 15, 0 ) );
    glMultMatrixf( &snakeHeadTrans1[0][0] );{
        CSCI441::drawSolidCylinder(1.25, 2, 3, 30, 30);
    }; glMultMatrixf( &(glm::inverse( snakeHeadTrans1 ))[0][0] );

    glm::mat4 snakeHeadTrans2 = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 18, 0 ) );
    glMultMatrixf( &snakeHeadTrans2[0][0] );{
        CSCI441::drawSolidCylinder(2, 1, 2, 30, 30);
    }; glMultMatrixf( &(glm::inverse( snakeHeadTrans2 ))[0][0] );

    glm::mat4 snakeHeadTrans3 = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 19.7, 0 ) );
    glMultMatrixf( &snakeHeadTrans3[0][0] );{
        CSCI441::drawSolidSphere(1.0, 30, 30);
    }; glMultMatrixf( &(glm::inverse( snakeHeadTrans3 ))[0][0] );

    glm::mat4 snakeEyeTrans1 = glm::translate( glm::mat4(1.0f), glm::vec3( -1, 19.7, 0.75 ) );
    glMultMatrixf( &snakeEyeTrans1[0][0] );{
        drawSnakeEye();
    }; glMultMatrixf( &(glm::inverse( snakeEyeTrans1 ))[0][0] );

    glm::mat4 snakeEyeTrans2 = glm::translate( glm::mat4(1.0f), glm::vec3( 0.75, 19.7, 1) );
    glMultMatrixf( &snakeEyeTrans2[0][0] );{
        drawSnakeEye();
    }; glMultMatrixf( &(glm::inverse( snakeEyeTrans2 ))[0][0] );
}

void Snake::drawSnakeBody(){
    double snakeLength = 15.0;
    double snakeRad = 1.5;
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeBodyTrans = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 0, 0 ) );
    glMultMatrixf( &snakeBodyTrans[0][0] );{
        CSCI441::drawSolidCylinder(snakeRad, snakeRad-0.25, snakeLength,30,30);
    }; glMultMatrixf( &(glm::inverse( snakeBodyTrans ))[0][0] );


    glColor3f(0.9f, 0.0f, 0.4f); //red for scales
    glm::mat4 snakeBodyWireTrans = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 0, 0 ) );
    glMultMatrixf( &snakeBodyWireTrans[0][0] );{
        glm::mat4 snakeBodyScale = glm::scale(glm::mat4(1.0f), glm::vec3(1.04f, 1.0, 1.04f));
        glMultMatrixf(&snakeBodyScale[0][0]);{
            CSCI441::drawWireCylinder(snakeRad, snakeRad-0.25, snakeLength,15,15);
        }; glMultMatrixf(&(glm::inverse(snakeBodyScale))[0][0]);
    }; glMultMatrixf( &(glm::inverse( snakeBodyWireTrans ))[0][0] );
}

void Snake::drawSnakeTail(){
    double snakeTailLength = 12.0;
    double snakeTailRad = 1.5;
    glColor3f(1.0f, 1.0f, 1.0f); //white

    glm::mat4 snakeBodyTrans = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 0, 0 ) );
    glMultMatrixf( &snakeBodyTrans[0][0] );{
        glm::mat4 snakeBodyRot = glm:: rotate( glm::mat4(1.0f), PI ,glm::vec3( 0.0f, 0.0f, 1.0f ) );
        glMultMatrixf( &snakeBodyRot[0][0] ); {
            CSCI441::drawSolidCone(snakeTailRad, snakeTailLength, 30, 30);
        }; glMultMatrixf(&(glm::inverse(snakeBodyRot))[0][0]);
    }; glMultMatrixf( &(glm::inverse( snakeBodyTrans ))[0][0] );


    glColor3f(0.9f, 0.0f, 0.4f); //red for scales
    glm::mat4 snakeBodyWireTrans = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 0, 0 ) );
    glMultMatrixf( &snakeBodyWireTrans[0][0] );{
        glm::mat4 snakeBodyWireRot = glm:: rotate( glm::mat4(1.0f), PI ,glm::vec3( 0.0f, 0.0f, 1.0f ) );
        glMultMatrixf( &snakeBodyWireRot[0][0] ); {
            glm::mat4 snakeBodyWireScale = glm::scale( glm::mat4(1.0f), glm::vec3(1.05f, 1.0, 1.05f));
            glMultMatrixf(&snakeBodyWireScale[0][0]);{
                CSCI441::drawSolidCone(snakeTailRad, snakeTailLength, 7, 7);
            }; glMultMatrixf(&(glm::inverse(snakeBodyWireScale))[0][0]);
        }; glMultMatrixf(&(glm::inverse(snakeBodyWireRot))[0][0]);
    }; glMultMatrixf( &(glm::inverse( snakeBodyWireTrans ))[0][0] );
}

void Snake::drawSnake(){
    drawSnakeHead();
    drawSnakeBody();
    drawSnakeTail();
}

//places giant solid and wired balls around like a fun ball pit for the character to run thru
void Snake::drawGiantBalls() {

    for(int i = -50; i <= 50; i++){
        for(int j = -50; j <= 50; j++) {
            float randVal = getRand();
            float randScale = rand() % 2 + 1;
            int randScaleInt = rand() % 2 + 1;
            if(randVal < 0.2 && i%2==0 && j%2==0){
                glm::mat4 randBuilding = glm::translate( glm::mat4(1.0f), glm::vec3( i, randScale, j ) );
                glMultMatrixf( &randBuilding[0][0] );{
                    glm::mat4 scaleBuilding = glm::scale(glm::mat4(1.0f), glm::vec3(randScale* 1.0f, randScale*1.0, randScale*1.0f));
                    glMultMatrixf(&scaleBuilding[0][0]);{
                        glColor3f(getRand(), getRand(), getRand()); //random color
                        if(randScaleInt % 2 == 0) {
                            CSCI441::drawSolidSphere(getRand(), 30, 30);
                        }else{
                            CSCI441::drawWireSphere(getRand(), 15, 15);
                        }
                    }; glMultMatrixf(&(glm::inverse(scaleBuilding))[0][0]);
                }; glMultMatrixf( &(glm::inverse( randBuilding ))[0][0] );

            }
        }
    }
}

void Snake::drawConeUp(){
    glm::mat4 coneTrans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glMultMatrixf( &coneTrans[0][0] );
    CSCI441::drawSolidCone(1.0,4.0,20,60);
    glMultMatrixf( &(glm::inverse( coneTrans ))[0][0] );
}

void Snake::drawConeDown(){
    glm::mat4 coneTrans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0));
    glMultMatrixf( &coneTrans[0][0] );
    glm::mat4 coneRot = glm::rotate( glm::mat4(1.0f), PI, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf( &coneRot[0][0] );
    CSCI441::drawSolidCone(1.0,4.0,20,60);
    glMultMatrixf( &(glm::inverse( coneRot ))[0][0] );
    glMultMatrixf( &(glm::inverse( coneTrans ))[0][0] );
}

void Snake::drawConeLeft(){
    glm::mat4 coneTrans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0));
    glMultMatrixf( &coneTrans[0][0] );
    glm::mat4 coneRot = glm::rotate( glm::mat4(1.0f), -PI/2, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf( &coneRot[0][0] );
    CSCI441::drawSolidCone(1.0,4.0,20,60);
    glMultMatrixf( &(glm::inverse( coneRot ))[0][0] );
    glMultMatrixf( &(glm::inverse( coneTrans ))[0][0] );
}

void Snake::drawConeRight(){
    glm::mat4 coneTrans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0));
    glMultMatrixf( &coneTrans[0][0] );
    glm::mat4 coneRot = glm::rotate( glm::mat4(1.0f), PI/2, glm::vec3(0.0f, 0.0f, 1.0f));
    glMultMatrixf( &coneRot[0][0] );
    CSCI441::drawSolidCone(1.0,4.0,20,60);
    glMultMatrixf( &(glm::inverse( coneRot ))[0][0] );
    glMultMatrixf( &(glm::inverse( coneTrans ))[0][0] );
}

void Snake::drawSparkle(){

    sparkleRotTheta += 0.1f;
    if(sparkleRotTheta >= 2*PI) {
        sparkleRotTheta -= (2 * PI);
    }

    glm::mat4 sparkleRot = glm::rotate( glm::mat4(1.0f), sparkleRotTheta, glm::vec3(0.0f, 1.0f, 0.0));
    glMultMatrixf( &sparkleRot[0][0] );
    drawConeRight();
    drawConeLeft();
    drawConeUp();
    drawConeDown();
    glMultMatrixf( &(glm::inverse( sparkleRot ))[0][0] );
}

void Snake::drawFaery(){

    glm::mat4 faeryTrans = glm::translate( glm::mat4(1.0f), faeryMovement[faeryIndex]);
    glMultMatrixf( &faeryTrans[0][0] );{
        glm::mat4 faeryScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
        glMultMatrixf(&faeryScale[0][0]);{
            glColor3f(1.0f, 1.0f, 1.0); //white
            drawSparkle();
        }; glMultMatrixf(&(glm::inverse(faeryScale))[0][0]);
    }; glMultMatrixf( &(glm::inverse( faeryTrans ))[0][0] );

}

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates a display list with the code to draw a simple
//      environment for the user to navigate through.
////////////////////////////////////////////////////////////////////////////////
//void Snake::generateEnvironmentDL() {
//
//    GLsizei n = 1;
//    environmentDL = glGenLists(n);
//    glNewList( environmentDL, GL_COMPILE );
//    drawGrid();
//    drawGiantBalls();
//    glEndList();
//}

void Snake::renderScene()  {

    glCallList( environmentDL );

    glm::mat4 transSnake = glm::translate(glm::mat4(1.0f), snakePos);
    glMultMatrixf(&transSnake[0][0]);{
        glm::mat4 rotateSnakechange = glm::rotate(glm::mat4(1.0f), -snakeTheta, glm::vec3(0.0f, 1.0, 0.0f));
        glMultMatrixf(&rotateSnakechange[0][0]);{
            drawFaery();

            if(!curveToggle) {
                for (int i = 0; i < (controlPoints.size() - 3); i += 3) {
                    renderBezierCurve_BP(controlPoints[i], controlPoints[i + 1], controlPoints[i + 2],
                                      controlPoints[i + 3], 100);
                }
            }

            if(!boxAndPointsToggle) {
                glColor3f(0, 1, 0);
                for (auto point : controlPoints) {
                    glm::mat4 transPoint = glm::translate(glm::mat4(1), point);
                    glMultMatrixf(&transPoint[0][0]);
                    CSCI441::drawSolidSphere(0.25f, 100.0f, 100.0f);
                    glMultMatrixf(&inverse(transPoint)[0][0]);
                }

                glBegin(GL_LINE_STRIP);
                {
                    glColor3f(1.0f, 1.0f, 0.0f);
                    glLineWidth(3.0f);
                    for (auto point : controlPoints) {
                        glVertex3f(point[0], point[1], point[2]);
                    }
                };
                glEnd();
            }

            glm::mat4 rotateSnake = glm::rotate(glm::mat4(1.0f), PI/2, glm::vec3(0.0f, 0.0, 1.0f));
            glMultMatrixf(&rotateSnake[0][0]);{
                glm::mat4 scaleSnake = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
                glMultMatrixf(&scaleSnake[0][0]);{
                    drawSnakeTongue();
                    drawSnake();
                }; glMultMatrixf(&(glm::inverse(scaleSnake))[0][0]);
            }; glMultMatrixf(&(glm::inverse(rotateSnake))[0][0]);
        }; glMultMatrixf(&(glm::inverse(rotateSnakechange))[0][0]);
    }; glMultMatrixf(&(glm::inverse(transSnake))[0][0]);
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
//GLFWwindow* setupGLFW() {
//    // set what function to use when registering errors
//    // this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
//    // all other GLFW calls must be performed after GLFW has been initialized
//    glfwSetErrorCallback( error_callback );
//
//    // initialize GLFW
//    if( !glfwInit() ) {
//        fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
//        exit( EXIT_FAILURE );
//    } else {
//        fprintf( stdout, "[INFO]: GLFW initialized\n" );
//    }
//
//    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
//    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
//    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized
//
//    // create a window for a given size, with a given title
//    GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Snake & Faery in a Ball Pit", NULL, NULL );
//    if( !window ) {						// if the window could not be created, NULL is returned
//        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
//        glfwTerminate();
//        exit( EXIT_FAILURE );
//    } else {
//        fprintf( stdout, "[INFO]: GLFW Window created\n" );
//    }
//
//    glfwMakeContextCurrent(window);		// make the created window the current window
//    glfwSwapInterval(1);				     	// update our screen after at least 1 screen refresh
//
//    glfwSetKeyCallback( window, keyboard_callback );							// set our keyboard callback function
//    glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
//    glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function
//
//    return window;						       // return the window that was created
//}

//
//  void setupOpenGL()
//      Used to setup everything OpenGL related.

//void setupOpenGL() {
//    // tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
//    //		surface removal.  We will discuss this more very soon.
//    glEnable( GL_DEPTH_TEST );
//
//    //******************************************************************
//    // this is some code to enable a default light for the scene;
//    // feel free to play around with this, but we won't talk about
//    // lighting in OpenGL for another couple of weeks yet.
//    GLfloat lightCol[4] = { 1, 1, 1, 1};
//    GLfloat ambientCol[4] = { 0.0, 0.0, 0.0, 1.0 };
//    GLfloat lPosition[4] = { 10, 10, 10, 1 };
//    glLightfv( GL_LIGHT0, GL_POSITION,lPosition );
//    glLightfv( GL_LIGHT0, GL_DIFFUSE,lightCol );
//    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );
//    glEnable( GL_LIGHTING );
//    glEnable( GL_LIGHT0 );
//
//    // tell OpenGL not to use the material system; just use whatever we
//    // pass with glColor*()
//    glEnable( GL_COLOR_MATERIAL );
//    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
//    //******************************************************************
//
//    // tells OpenGL to blend colors across triangles. Once lighting is
//    // enabled, this means that objects will appear smoother - if your object
//    // is rounded or has a smooth surface, this is probably a good idea;
//    // if your object has a blocky surface, you probably want to disable this.
//    glShadeModel( GL_SMOOTH );
//
//    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
//}

//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
//void setupScene() {
//    // give the camera a scenic starting point.
//
//    cameraTheta = -PI / 3.0f;
//    cameraPhi = PI / 2.8f;
//    camRad = 20;
//
//  snakePos.x = 0;
//  snakePos.y = 1;
//   snakePos.z = 0;
//    snakeTheta = PI;
//
//    recomputeOrientation();
//    recomputeSnakeOrientation();
//
//    srand( time(NULL) );
//    generateEnvironmentDL();
//}

///*************************************************************************************

// int main( int argc, char *argv[] ) {
//    GLFWwindow *window = setupGLFW();
//    setupOpenGL();
//    setupScene();
//
//    fprintf(stdout, "Enter file name:");
//    char filename[100];
//    gets(filename);
//    loadControlPoints_BP(filename);
//
//    for(int i = 0; i < (controlPoints.size() - 3); i += 3) {
//        for(int j = 0; j < 100; j++){
//            glm::vec3 point = evaluateBezierCurve_BP(controlPoints[i], controlPoints[i+1], controlPoints[i+2], controlPoints[i+3], float(j)/100);
//            faeryMovement.push_back(point);
//        }
//    }

//    while( !glfwWindowShouldClose(window) ) {
//        glDrawBuffer( GL_BACK );				// work with our back frame buffer
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window
//
//        // update the projection matrix based on the window size
//        // the GL_PROJECTION matrix governs properties of the view coordinates;
//        // i.e. what gets seen - use a perspective projection that ranges
//        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
//        glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f );
//        glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
//        glLoadIdentity();				// set the matrix to be the identity
//        glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state
//
//        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
//        // when using a Retina display the actual window can be larger than the requested window.  Therefore
//        // query what the actual size of the window we are rendering to is.
//        GLint framebufferWidth, framebufferHeight;
//        glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );
//
//        // update the viewport - tell OpenGL we want to render to the whole window
//        glViewport( 0, 0, framebufferWidth, framebufferHeight );
//
//        glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
//        glLoadIdentity();							// set the matrix to be the identity
//
//        // set up our look at matrix to position our camera
//        glm::vec3 eyePoint = snakePos + camDir;
//        glm::vec3 lookAtPoint = glm::vec3( snakePos );
//        glm::vec3 upVector = glm::vec3( 0, 1, 0 );      // up vector is (0, 1, 0) - positive Y
//        glm::mat4 viewMtx = glm::lookAt( eyePoint, lookAtPoint, upVector );
//        // multiply by the look at matrix - this is the same as our view matrix
//        glMultMatrixf( &viewMtx[0][0] );
//
//        faeryIndex = (faeryIndex + 1) % faeryMovement.size();
//
//        renderScene();
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//
//    }
//
//    glfwDestroyWindow( window );
//    glfwTerminate();
//
//    return EXIT_SUCCESS;
//}


void Snake::updateThings(){

    faeryIndex = (faeryIndex + 1) % faeryMovement.size();

}


