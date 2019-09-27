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
#include "BezierPatch.h"
#include "Hero1.h"
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

// Camera variables
float cameraDist = 10;                              // camera's distance from the player
glm::vec3 camPos;                                    // camera position in cartesian coordinates
float cameraTheta, cameraPhi;                    // camera DIRECTION in spherical coordinates
const float CAM_SPEED = FPS_ADJUSTMENT*0.2f;                       // camera's rate of position change
const float CAM_ANGULAR_VEL = FPS_ADJUSTMENT*0.05f;                // camera's rate of direction change

// Environment drawing variables
GLuint environmentDL;                            // display list for the 'city'

vector<glm::vec3> groundControlPoints;
const int MAP_SIZE = 100;
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

// Characters
Cart cartHero;

// Interaction variables
// camera movement
int leftMouseButton;                                // status of the mouse button
glm::vec2 mousePos;                                    // last known X and Y of the mouse
// Keys movement
bool keysDown[256] = {0};           // status of our keys
bool ctrlKey;                                        // status of the 'Ctrl' Key
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

// characterPos() //////////////////////////////////////////////////////
//
// This function updates keeps the player on the map
////////////////////////////////////////////////////////////////////////////////
glm::vec3 characterPos(glm::vec3 position) {
    position = computePositionBezierPatch(groundControlPoints, position.x,position.z);
    return position;
}

glm::vec3 characterNormal(glm::vec3 position){
    vector<glm::vec3> divUV = evaluateBezierPatchDerivative(groundControlPoints, position.x, position.z);
    return glm::normalize(glm::cross(divUV.at(0), divUV.at(1)));
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
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL:
                ctrlKey = true;
                break;
            default:
                keysDown[key] = true;
        }
    }
    // Update key status
    if(action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL:
                ctrlKey = false;
                break;
            default:
                keysDown[key] = false;
        }
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
        if (ctrlKey) {
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

// TODO:
void updateWandersPos() {
    bool moved = false;
    if(keysDown[GLFW_KEY_W]) {
        cartHero.moveForward();
        moved = true;
    }
    if(keysDown[GLFW_KEY_S]){
        cartHero.moveBackward();
        moved = true;
    }
    if(keysDown[GLFW_KEY_A]){
        cartHero.turnLeft();
        moved = true;
    }
    if(keysDown[GLFW_KEY_D]){
        cartHero.turnRight();
        moved = true;
    }
    // Correct the hero's y position
    if(moved){
        cartHero.setPos(characterPos(cartHero.getBezierPosition()));
        cartHero.setOrientation(characterNormal(cartHero.getBezierPosition()));
    }

}

// update() /////////////////////////////////////////////////////
//
//  The main update loop, called every render, calls all other update functions
//
////////////////////////////////////////////////////////////////////////////////
void update() {
    updateWandersPos();
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!




// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void drawGrid() {
    // Creating a realistic topography

    int spacing = (GRID_END-GRID_START)/3;
    float height = 0.2*(GRID_END-GRID_START);
    for (int i = GRID_START; i < GRID_END; i+= spacing) {
        for (int j = GRID_START; j < GRID_END; j+= spacing) {
            groundControlPoints.emplace_back(glm::vec3((float)i, (float)(height*getRand()-height/2), (float)j));
        }
    }
    glColor3f(1.0f,1.0f,1.0f);
    GLfloat matColorD[4] = { 1.0f,1.0f,1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matColorD);
    drawBezierPatch(groundControlPoints);
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
        //drawEnvironment();
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
    cartHero.draw();
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

    //glEnable( GL_CULL_FACE );           // enable back face culling to speed render times
    glFrontFace( GL_CCW );              // denote front faces specified by counter-clockwise winding order
    glCullFace( GL_BACK );              // cull our back faces
    //******************************************************************
    // this is some code to enable a default light for the scene;
    // feel free to play around with this, but we won't talk about
    // lighting in OpenGL for another couple of weeks yet.
    GLfloat lightCol[4] = {1, 1, 1, 1};
    GLfloat ambientCol[4] = {0.0, 0.0, 0.0, 1.0};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientCol);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // tell OpenGL not to use the material system; just use whatever we
    // pass with glColor*()
    glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

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
    recomputePosition();
    srand(time(NULL));    // seed our random number generator
    generateEnvironmentDL();
    cartHero.setPos(characterPos(cartHero.getBezierPosition()));
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
    /*
    char fileName[100];
    fprintf(stdout, "Please enter a file name:");
    gets(fileName);
    while (!loadControlPoints(fileName)) {
        fprintf(stdout, "The file %35s is invalid.\n", fileName);
        fprintf(stdout, "Please enter a file name:");
        gets(fileName);
    }
     */
    //loadControlPoints("controlPoints4.csv");

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while (!glfwWindowShouldClose(window)) {    // check if the window was instructed to be clo
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
        glm::mat4 viewMtx = glm::lookAt(camPos + cartHero.getPos(),        // camera is located at (10, 10, 10)
                                        cartHero.getPos(),        // camera is looking at (0, 0, 0,)
                                        glm::vec3(0, 1, 0));        // up vector is (0, 1, 0) - positive Y
        // multiply by the look at matrix - this is the same as our view martix
        glMultMatrixf(&viewMtx[0][0]);
        GLfloat lPosition[4] = {10, 10, 10, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, lPosition);

        renderScene();                    // draw everything to the window

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();                // check for any events and signal to redraw screen
    }
    glfwDestroyWindow(window);// clean up and close our window
    glfwTerminate();                        // shut down GLFW to clean up our context

    return EXIT_SUCCESS;                // exit our program successfully!
}
