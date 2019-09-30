
#ifndef HERO2_BP_H
#define HERO2_BP_H

class Snake {

public:
    int leftMouseButton;                // status of the mouse button
    glm::vec2 mousePosition;            // last known X and Y of the mouse

    float cameraTheta, cameraPhi;       // camera DIRECTION in spherical coordinates
    glm::vec3 camDir;                   // camera DIRECTION in cartesian coordinates
    float camRad;

    glm::vec3 snakePos = {0,1,0};                 // snake position in cartesian coordinates
    float snakeTheta = PI;                  // snake DIRECTION in spherical coordinates
    glm::vec3 snakeDir;                 // snake DIRECTION in cartesian coordinates

    int flickingTongueDir = 20;         //
    int tongueChange = 1;               //

    vector<glm::vec3> faeryMovement;    //
    int faeryIndex = 0;                 //
    float sparkleRotTheta = 0;          //

    boolean ctrlPress = false;          //
    boolean boxAndPointsToggle = false; //
    boolean curveToggle = false;        //

    GLuint environmentDL;               // display list for the 'city'
    float PI = 3.141592;                //

    vector<glm::vec3> controlPoints;    //

    float pointTrackingVal = 0.0f;      //
    int numSegments = 0;                //


    Snake();                            //Constructor

    bool loadControlPoints_BP( char* filename );
    void recomputeOrientation();
    void recomputeSnakeOrientation();
    float getRand();
    glm::vec3 evaluateBezierCurve_BP(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
    void renderBezierCurve_BP( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution );
    void drawGrid();
    void drawSnakeEye();
    void flickingTongue();
    void drawSnakeTongue();
    void drawSnakeHead();
    void drawSnakeBody();
    void drawSnakeTail();
    void drawSnake();
    void drawGiantBalls();
    void drawConeUp();
    void drawConeDown();
    void drawConeLeft();
    void drawConeRight();
    void drawSparkle();
    void drawFaery();
    void updateThings();
    void renderScene();


};

#endif //HERO2_BP_H