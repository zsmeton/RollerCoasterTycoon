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
#include "BezierCurve.h"
#include "BezierPatch.h"
#include "Hero1.h"
#include "Hero2.h"
#include "Hero2.cpp"
#include "CameraBase.h"
#include "ArcBallCamera.h"
#include "FreeCamera.h"
#include "FirstPersonCamera.h"

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

// Minimap window settings
const GLint MINI_MAP_SIZE = 200;
GLint miniWindowStartX = windowWidth - MINI_MAP_SIZE;
GLint miniWindowStartY = windowHeight - MINI_MAP_SIZE;

// Camera variables
ArcBallCamera arcBallCamera;
FreeCamera freeCamera;
FirstPersonCamera* FPVCam;
int FPVCharacter = 0;
CameraBase* cam = &arcBallCamera;


// Environment drawing variables
GLuint environmentDL;                            // display list for the 'city'

vector<vector<vector<glm::vec3>>> groundControlPoints;
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
Snake snakeHero;
vector<HeroBase*> heros = {&cartHero, &snakeHero};
vector<FaeryHero*> faeryHeros = {&cartHero, &snakeHero};

// Interaction variables
// camera movement
int leftMouseButton;                                // status of the mouse button
glm::vec2 mousePos;                                    // last known X and Y of the mouse
// Keys movement
bool keysDown[349] = {0};           // status of our keys
bool ctrlKey;                                        // status of the 'Ctrl' Key
//*************************************************************************************
//
// Helper Functions
////////////////////////////////////////////////////////////////////////////////

bool loadFaeryPaths(const FILE *file);

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

/*!
 * Reads in the faery paths from the world file
 * @param file an open readable file
 * @return true if success, false if failure
 */
bool loadFaeryPaths(FILE *file) {
    // Read in faery flight paths:
    int numFaerys;
    fscanf(file, "%d", &numFaerys);
    if(numFaerys > faeryHeros.size()){
        return false;
    }
    // Read in all the points
    for (int i = 0; i < numFaerys; i++) {
        faeryHeros.at(i)->setFaeryPath(loadControlPoints(file));
    }
    return true;
}

bool readWorldFile(char* filename){
    // Read in control points from file.  Make sure the file can be
    // opened and handle it appropriately.  return false if there is an error
    FILE *file;
    // Check if the file can be opened
    if ((file = fopen(filename, "r"))) {
        if(!loadFaeryPaths(file)){
            fprintf(stderr, "More faery paths than faery's");
            exit(-28);
        }
        // Read in bezier surface control points
        groundControlPoints = loadControlPointsBezierPatch(file);
        cartHero.setMaxZ(groundControlPoints.at(0).size()-0.00001f);
        cartHero.setMaxX(groundControlPoints.size() - 0.00001f);

        return true;
    }
    return false;
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
        cam->mouseMovement(x-mousePos.x, y-mousePos.y, ctrlKey);
    }
    cam->update();
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

void updateWandersPos() {
    bool moved = false;
    // Only move the wander if the camera target is the cart hero
    if((cam == &arcBallCamera && arcBallCamera.getTarget() == &cartHero.getPos())) {
        if (keysDown[GLFW_KEY_W]) {
            cartHero.moveForward();
            moved = true;
        }
        if (keysDown[GLFW_KEY_S]) {
            cartHero.moveBackward();
            moved = true;
        }
        if (keysDown[GLFW_KEY_A]) {
            cartHero.turnLeft();
            moved = true;
        }
        if (keysDown[GLFW_KEY_D]) {
            cartHero.turnRight();
            moved = true;
        }
        // Correct the hero's y position
        if (moved) {
            cartHero.setPos(characterPos(groundControlPoints, cartHero.getBezierPosition()));
            cartHero.setOrientation(characterNormal(groundControlPoints, cartHero.getBezierPosition()));
        }
    }
}

void updateCamera(){
    // If key 1,2,3 set the camera target to different heros
    if(!(keysDown[GLFW_KEY_LEFT_SHIFT]||keysDown[GLFW_KEY_RIGHT_SHIFT])){
        int targetIndex = -1;
        if(keysDown[GLFW_KEY_1]){
            targetIndex = 0;
        }else if(keysDown[GLFW_KEY_2]){
            targetIndex = 1;
        }else if(keysDown[GLFW_KEY_3]){
            //targetIndex = 2;
        }
        if(targetIndex != -1) {
            arcBallCamera.setTarget(&(heros.at(targetIndex)->getPos()));
        }
    }else{
        // change camera model
        if(keysDown[GLFW_KEY_1]){
            cam = &arcBallCamera;
        }else if(keysDown[GLFW_KEY_2]){
            FPVCharacter += 1;
            FPVCharacter %= heros.size();
            FPVCam = &(heros.at(FPVCharacter)->getFPVCam());
        }else if(keysDown[GLFW_KEY_3]){
            cam = &freeCamera;
        }
    }
    // Handle key presses
    cam->keyPress(keysDown[GLFW_KEY_W], keysDown[GLFW_KEY_S]);
}


// update() /////////////////////////////////////////////////////
//
//  The main update loop, called every render, calls all other update functions
//
////////////////////////////////////////////////////////////////////////////////
void update() {
    updateWandersPos();
    updateCamera();
    for(auto hero : heros) {
        hero->update();
    }
    //firstPersonCamera.setTargetDir(&cartHero.getDirection());

}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!


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
        drawBezierPatch(groundControlPoints);
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
    // Draw heros
    for(auto hero : heros) {
        hero->draw();
    }
    //firstPersonCamera.draw();
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


/*!
 * Set the initial parameters for the cameras
 */
void setupCameras(){
    arcBallCamera.setPhi(3 * M_PI / 5);
    arcBallCamera.setTheta(0);
    arcBallCamera.setDistance(10);
    arcBallCamera.setTarget(&cartHero.getPos());
    arcBallCamera.update();

    freeCamera.setTheta( -M_PI / 3.0f);
    freeCamera.setPhi(M_PI / 2.8f);
    freeCamera.setPos(glm::vec3(0.0f,20.0f,0.0f));
    freeCamera.update();

    FPVCam = &heros.at(0)->getFPVCam();
    FPVCharacter = 0;
}

//
//  void setupScene()
//
//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
void setupScene() {
    // Get world file
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
    readWorldFile("WorldFiles/WorldFile1.config");

    // Set up the environment
    generateEnvironmentDL();

    // Compute the hero's position and orientation
    cartHero.setPos(characterPos(groundControlPoints, cartHero.getBezierPosition()));
    cartHero.setOrientation(characterNormal(groundControlPoints, cartHero.getBezierPosition()));

    // TODO: Set snake hero based on location along curve
    snakeHero.setPos(characterPos(groundControlPoints, glm::vec3(0.5f,0.0f,0.5f)));
}


///*************************************************************************************
//
// Our main function

void mainScreen(GLint framebufferWidth, GLint framebufferHeight){
    // update the viewport - tell OpenGL we want to render to the whole window
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glMatrixMode(GL_MODELVIEW);    // make the ModelView matrix current to be modified by any transformations
    glLoadIdentity();                            // set the matrix to be the identity

    // set up our look at matrix to position our camera
    glm::mat4 viewMtx = cam->getLookAt();
    // multiply by the look at matrix - this is the same as our view martix
    glMultMatrixf(&viewMtx[0][0]);
    GLfloat lPosition[4] = {0, 100, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lPosition);

    renderScene();                    // draw everything to the window
}

void miniMap(GLint framebufferWidth, GLint framebufferHeight){
    // Do minimap render
    glScissor(miniWindowStartX, miniWindowStartY, framebufferWidth - miniWindowStartX,
              framebufferHeight - miniWindowStartY);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the Scissor to the full screen again
    glScissor(0, 0, framebufferWidth, framebufferHeight);
    // update the viewport - This is for the minimap view
    glViewport(miniWindowStartX, miniWindowStartY, framebufferWidth - miniWindowStartX,
               framebufferHeight - miniWindowStartY);

    glMatrixMode(GL_MODELVIEW);    // make the ModelView matrix current to be modified by any transformations
    glLoadIdentity();                            // set the matrix to be the identity

    // set up our look at matrix to position our camera
    glm::mat4 miniViewMtx = FPVCam->getLookAt();
    // multiply by the look at matrix - this is the same as our view martix
    glMultMatrixf(&miniViewMtx[0][0]);
    GLfloat lPosition[4] = {0, 100, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lPosition);

    renderScene();                    // draw everything to the window
}

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main(int argc, char *argv[]) {
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();    // initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();                                        // initialize all of the OpenGL specific information

    fprintf(stdout, "[INFO]: /--------------------------------------------------------\\\n");
    fprintf(stdout, "[INFO]: | OpenGL Information                                     |\n");
    fprintf(stdout, "[INFO]: |--------------------------------------------------------|\n");
    fprintf(stdout, "[INFO]: |   OpenGL Version:  %35s |\n", glGetString(GL_VERSION));
    fprintf(stdout, "[INFO]: |   OpenGL Renderer: %35s |\n", glGetString(GL_RENDERER));
    fprintf(stdout, "[INFO]: |   OpenGL Vendor:   %35s |\n", glGetString(GL_VENDOR));
    fprintf(stdout, "[INFO]: \\--------------------------------------------------------/\n");



    setupScene();                                            // initialize objects in our scene
    setupCameras();
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

        mainScreen(framebufferWidth, framebufferHeight);

        miniMap(framebufferWidth, framebufferHeight);

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();                // check for any events and signal to redraw screen
    }
    glfwDestroyWindow(window);// clean up and close our window
    glfwTerminate();                        // shut down GLFW to clean up our context

    return EXIT_SUCCESS;                // exit our program successfully!
}
