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
#include "Environment.h"
#include "Hero3.h"

using namespace std;

class windowDimensions{
public:
    GLint x0,y0,width,height;
    bool inWindow(float x, float y){
        return (x>= x0 && x <= x0+width && y>=y0 && y <=y+height);
    }
};

//*************************************************************************************
//
// Global Parameters

#ifndef M_PI
#define M_PI 3.14159
#endif

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 1920, windowHeight = 1080;

// Minimap window settings
const GLint MINI_MAP_SIZE = 200;

//split screen settings
windowDimensions splitL = {0, 0, windowWidth/2, windowHeight};
windowDimensions miniMapL = {splitL.width+splitL.x0-MINI_MAP_SIZE, splitL.height+splitL.y0-MINI_MAP_SIZE, MINI_MAP_SIZE, MINI_MAP_SIZE};

windowDimensions splitR = {windowWidth/2, 0, windowWidth/2, windowHeight};
windowDimensions miniMapR = {splitR.width+splitR.x0-MINI_MAP_SIZE, splitR.height+splitR.y0-MINI_MAP_SIZE, MINI_MAP_SIZE, MINI_MAP_SIZE};


// Camera variables
ArcBallCamera arcBallCameraL;
FreeCamera freeCameraL;
FirstPersonCamera* FPVCamL;
int FPVCharacterL = 0;
time_t FPVToggleTimer = 0;
time_t FPV_TOGGLE_DURATION = 1; // wait x amount of time before switching fpv camera
CameraBase* camL = &arcBallCameraL;

ArcBallCamera arcBallCameraR;
FreeCamera freeCameraR;
FirstPersonCamera* FPVCamR;
int FPVCharacterR = 0;
CameraBase* camR = &arcBallCameraR;


// Environment drawing variables
GLuint environmentDL;                            // display list for the 'city'

vector<vector<vector<glm::vec3>>> groundControlPoints;
vector<glm::vec3> coasterControlPoints;
vector<EnvironmentObject*> objects;

// Characters
Cart cartHero;
Snake snakeHero;
Coaster coasterHero;
vector<HeroBase*> heros = {&cartHero, &snakeHero, &coasterHero};
vector<FaeryHero*> faeryHeros = {&cartHero, &snakeHero};

// Interaction variables
// camera movement
int leftMouseButton;                                // status of the mouse button
glm::vec2 mousePos;                                    // last known X and Y of the mouse
// Keys movement
bool keysDown[349] = {0};           // status of our keys
bool ctrlKey;                                        // status of the 'Ctrl' Key
bool split;


/*!
 * Reads in the faery paths from the world file
 * @param file an open readable file
 * @return true if success, false if failure
 */
bool loadFaeryPaths(FILE *file) {
    // Read in faery flight paths:
    int numFaerys;
    fscanf(file, "%d", &numFaerys);
    if(numFaerys == 0) return false;
    if(numFaerys > faeryHeros.size()){
        return false;
    }
    // Read in all the points
    for (int i = 0; i < numFaerys; i++) {
        faeryHeros.at(i)->setFaeryPath(loadControlPoints(file));
    }
    return true;
}

/*!
 * Reads in the environment objects from the world file
 * @param file an open readable file
 * @return true if success, false if failure
 */
bool readEnvironment(FILE* file){
    int numObjects;
    fscanf(file, "%d", &numObjects);
    if(numObjects == 0) return false;
    // Read in all the points
    for (int i = 0; i < numObjects; i++) {
        float x,z;
        char type [4];
        fscanf(file, "%s %f %f", &type[0], &x, &z);
        glm::vec3 pos = computePositionBezierPatch(groundControlPoints, x,z);
        if(strcmp(type,"Ball") == 0){
            Ball* ball = new Ball(pos.x, pos.y, pos.z);
            objects.emplace_back(ball);
        }else if (strcmp(type,"Rock") == 0){
            Rock* rock = new Rock(pos.x, pos.y, pos.z);
            objects.emplace_back(rock);
        }else if (strcmp(type,"Bush") == 0){
            Bush* bush = new Bush(pos.x, pos.y, pos.z);
            objects.emplace_back(bush);
        }
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
            fprintf(stderr, "Invalid paths for faery's\n");
            return false;
        }
        // Read in bezier surface control points
        groundControlPoints = loadControlPointsBezierPatch(file);
        if(groundControlPoints.size() == 0){
            fprintf(stderr, "Invalid control points for ground surface\n");
            return false;
        }
        cartHero.setMaxZ(groundControlPoints.at(0).size()-0.00001f);
        cartHero.setMaxX(groundControlPoints.size() - 0.00001f);
        // Read in bezier curve
        coasterControlPoints = loadControlPoints(file);
        if(coasterControlPoints.size() == 0){
            fprintf(stderr, "Invalid control points for roller coaster\n");
            return false;
        }
        coasterHero.setControlPoints(coasterControlPoints);
        snakeHero.setControlPoints(coasterControlPoints);

        // Loading in environment objects
        if(!readEnvironment(file)){
            fprintf(stderr, "Invalid environment objects\n");
            return false;
        }
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
            case GLFW_KEY_X:
                split = !split;
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
        if(split){
            if(splitL.inWindow(x,y)){
                camL->mouseMovement(x - mousePos.x, y - mousePos.y, ctrlKey);
            }else{
                camR->mouseMovement(x - mousePos.x, y - mousePos.y, ctrlKey);
            }
        }else {
            camL->mouseMovement(x - mousePos.x, y - mousePos.y, ctrlKey);
        }
    }
    camL->update();
    camR->update();
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
    if((camL == &arcBallCameraL && arcBallCameraL.getTarget() == &cartHero.getPos())||(split && camR == &arcBallCameraR && arcBallCameraR.getTarget() == &cartHero.getPos())) {
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

void cameraChange(CameraBase*& cam, ArcBallCamera& arc, FreeCamera& free, FirstPersonCamera*& fpv, int& FPVCharacter) {
    // If key 1,2,3 set the camera target to different heros
    if (!(keysDown[GLFW_KEY_LEFT_SHIFT] || keysDown[GLFW_KEY_RIGHT_SHIFT])) {
        int targetIndex = -1;
        if (keysDown[GLFW_KEY_1]) {
            targetIndex = 0;
        } else if (keysDown[GLFW_KEY_2]) {
            targetIndex = 1;
        } else if (keysDown[GLFW_KEY_3]) {
            targetIndex = 2;
        }
        if (targetIndex != -1) {
            if (cam == &arc) {
                arc.setTarget(&(heros.at(targetIndex)->getPos()));
            }
        }
    } else {
        // change camera model
        if (keysDown[GLFW_KEY_1]) {
            cam = &arc;
        } else if (keysDown[GLFW_KEY_3]) {
            if ((time(NULL) - FPVToggleTimer) > FPV_TOGGLE_DURATION / 2) {
                FPVToggleTimer = time(NULL);
                FPVCharacter += 1;
                FPVCharacter %= heros.size();
                fpv = &(heros.at(FPVCharacter)->getFPVCam());
            }
        } else if (keysDown[GLFW_KEY_2]) {
            cam = &free;
        }
    }
    cam->keyPress(keysDown[GLFW_KEY_W], keysDown[GLFW_KEY_S]);
}

void updateCamera(){
    if(split){
        if(splitL.inWindow(mousePos.x, mousePos.y)){
            cameraChange(camL, arcBallCameraL, freeCameraL, FPVCamL, FPVCharacterL);
        }else{
            cameraChange(camR, arcBallCameraR, freeCameraR, FPVCamR, FPVCharacterR);
        }
    }else{
        cameraChange(camL, arcBallCameraL, freeCameraR, FPVCamR, FPVCharacterR);
    }

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
        for(auto obj : objects){
            obj->render();
        }

        drawBezierCurve(coasterControlPoints);
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

    glEnable( GL_CULL_FACE );           // enable back face culling to speed render times
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
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientCol);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT2, GL_AMBIENT, ambientCol);
    glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    //glEnable(GL_LIGHT2);

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
    arcBallCameraL.setPhi(3 * M_PI / 5);
    arcBallCameraL.setTheta(0);
    arcBallCameraL.setDistance(10);
    arcBallCameraL.setTarget(&cartHero.getPos());
    arcBallCameraL.update();

    freeCameraL.setTheta(-M_PI / 3.0f);
    freeCameraL.setPhi(M_PI / 2.8f);
    freeCameraL.setPos(glm::vec3(0.0f, 20.0f, 0.0f));
    freeCameraL.update();

    FPVCamL = &heros.at(1)->getFPVCam();
    FPVCharacterL = 0;

    arcBallCameraR.setPhi(3 * M_PI / 5);
    arcBallCameraR.setTheta(0);
    arcBallCameraR.setDistance(10);
    arcBallCameraR.setTarget(&cartHero.getPos());
    arcBallCameraR.update();

    freeCameraR.setTheta(-M_PI / 3.0f);
    freeCameraR.setPhi(M_PI / 2.8f);
    freeCameraR.setPos(glm::vec3(0.0f, 20.0f, 0.0f));
    freeCameraR.update();

    FPVCamR = &heros.at(1)->getFPVCam();
    FPVCharacterR = 0;
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
    readWorldFile("WorldFiles/WorldFile2.config");

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

void placeLighting(){
    GLfloat lPosition[4] = {0, 100, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lPosition);
    GLfloat lPosition1[4] = {-100, 200, -100, 1};
    glLightfv(GL_LIGHT1, GL_POSITION, lPosition1);
    GLfloat lPosition2[4] = {100, 200, 100, 1};
    glLightfv(GL_LIGHT2, GL_POSITION, lPosition2);
}

void windowRender(windowDimensions window, Lookable* cam, GLint framebufferWidth, GLint framebufferHeight){
    // Do minimap render
    glScissor(window.x0, window.y0, window.width, window.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the Scissor to the full screen again
    glScissor(0, 0, framebufferWidth, framebufferHeight);
    // update the viewport - This is for the minimap view
    glViewport(window.x0, window.y0, window.width, window.height);

    glMatrixMode(GL_MODELVIEW);    // make the ModelView matrix current to be modified by any transformations
    glLoadIdentity();                            // set the matrix to be the identity

    // set up our look at matrix to position our camera
    glm::mat4 miniViewMtx = cam->getLookAt();
    // multiply by the look at matrix - this is the same as our view martix
    glMultMatrixf(&miniViewMtx[0][0]);
    placeLighting();

    renderScene();                    // draw everything to the window
}

void mainScreen(GLint framebufferWidth, GLint framebufferHeight){
    if(split){
        windowRender(splitL, camL,framebufferWidth, framebufferHeight);
        windowRender(miniMapL, FPVCamL,framebufferWidth, framebufferHeight);
        windowRender(splitR, camR,framebufferWidth, framebufferHeight);

    }else {
        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glMatrixMode(GL_MODELVIEW);    // make the ModelView matrix current to be modified by any transformations
        glLoadIdentity();                            // set the matrix to be the identity

        // set up our look at matrix to position our camera
        glm::mat4 viewMtx = camL->getLookAt();
        // multiply by the look at matrix - this is the same as our view matrix
        glMultMatrixf(&viewMtx[0][0]);
        placeLighting();

        renderScene();                    // draw everything to the window
    }
    windowRender(miniMapR, FPVCamR, framebufferWidth, framebufferHeight);
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


        (framebufferWidth, framebufferHeight);

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();                // check for any events and signal to redraw screen
    }
    glfwDestroyWindow(window);// clean up and close our window
    glfwTerminate();                        // shut down GLFW to clean up our context

    return EXIT_SUCCESS;                // exit our program successfully!
}
