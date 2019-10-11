//
// Created by zsmeton on 9/16/19.
//

#ifndef A4_BEZIERCURVE_H
#define A4_BEZIERCURVE_H

#include <GL/gl.h>
#include <CSCI441/objects.hpp> // for our 3D objects
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>               // for vector data structure
#include <bits/stdc++.h>        // For fmod()
using namespace std;

// CHANGING THESE VARIABLES WILL BREAK A LOT OF THINGS SO PLEASE DON'T
const int GROUP_SIZE = 4;
const int RESOLUTION = 100;


/*!
 * Computes a location along a Bezier Curve. Given the time step and the control cage
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @param t
 * @return
 */
glm::vec3 evaluateBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    glm::vec3 a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    glm::vec3 b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    glm::vec3 c = -3.0f * p0 + 3.0f * p1;
    glm::vec3 d = p0;
    glm::vec3 point = static_cast<float>(pow(t, 3)) * a + static_cast<float>(pow(t, 2)) * b + t * c + d;
    return point;
}


/*!
 * Computes the derivative of the Bezier Curve. Given the time step and the computed coefficients
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @param t
 * @return
 */
glm::vec3 evaluateBezierCurveDerivative(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    glm::vec3 a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    glm::vec3 b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    glm::vec3 c = -3.0f * p0 + 3.0f * p1;
    glm::vec3 d = p0;
    glm::vec3 point = 3.0f*static_cast<float>(pow(t, 2)) * a + 2*static_cast<float>(pow(t, 1)) * b + c;
    return normalize(point);
}


/*!
 * Computes the derivative of the bezier curve at time t
 * @param controlPoints the points defining the bezier curve
 * @param t the time step along the curve
 * @return [dx,dy,z]
 */
glm::vec3 computeRotationBezierCurve(const vector<glm::vec3>& controlPoints, const float t) {
    // The bezier curve is grouped into fours
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // to find the position of the object given a dt we must
    // calculate which group defines the bezier curve we must the object is on

    // calculate which bezier curve the object is in based on dt
    int group = int(t) % int(floor(controlPoints.size()/3));
    int groupStartIndex = 3*group;

    glm::vec3 a = controlPoints.at(groupStartIndex);
    glm::vec3 b = controlPoints.at(groupStartIndex+1);
    glm::vec3 c = controlPoints.at(groupStartIndex+2);
    glm::vec3 d = controlPoints.at(groupStartIndex+3);
    float groupT = fmod(t,1.0f) ;
    return evaluateBezierCurveDerivative(a,b,c,d,groupT);
}

/*!
 * Responsible for drawing a Bezier Curve as defined by four control points.
 * reaks the curve into n segments as specified by the resolution.
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @param resolution
 */
void renderBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution ) {

    glDisable(GL_COLOR_MATERIAL);
    glm::vec3 UP = glm::vec3(0.0f,0.0f,1.0f);
    glLineWidth(3.0f);
    for(int i = 0; i < resolution; i++){
        glColor3f((i/0.25)/255.0f, i/255.0f,(1*4)/255.0f);
        glm::vec3 coasterHeading = computeRotationBezierCurve(vector<glm::vec3>({p0,p1,p2,p3}), float(i)/ static_cast<float>(resolution));
        glm::vec3 rotAxis =glm::cross(UP, coasterHeading);
        float rad = acos(glm::dot( glm::normalize(coasterHeading), glm::normalize(UP)));
        GLfloat color[4] = {static_cast<GLfloat>((i/0.25)/255.0f), i/255.0f,(1*4)/255.0f, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
        // Find the rotation of the coaster cube
        glm::vec3 point = evaluateBezierCurve(p0, p1, p2, p3, float(i)/ static_cast<float>(resolution));
        glm::mat4 coasterCubeMtx = glm::translate(glm::mat4(1.0f), point);
        coasterCubeMtx = glm::rotate(coasterCubeMtx, rad, rotAxis);
        coasterCubeMtx = glm::scale(coasterCubeMtx, glm::vec3(4.0, 0.25, 0.25));
        glMultMatrixf(&coasterCubeMtx[0][0]);
        CSCI441::drawSolidCube(2);
        glMultMatrixf(&(glm::inverse(coasterCubeMtx))[0][0]);
        glColor3f(0.7,0.7, 0.7);

        if(i%9==1) {
            if(point[1] > 0.0f) {
                glm::mat4 CoasterPolesTrans = glm::translate(glm::mat4(1.0f), glm::vec3(point[0], -point[1], point[2]));
                glMultMatrixf(&CoasterPolesTrans[0][0]);
                CSCI441::drawSolidCylinder(.7, 0.25, point[1]*2, 25, 25);
                glMultMatrixf(&(glm::inverse(CoasterPolesTrans))[0][0]);
            }
        }

        if(i < 100 && i > 70){
            glColor3f(i/255.0f,(1*4)/255.0f, (i/0.25)/255.0f);
        }
    }
    glEnable(GL_COLOR_MATERIAL);

}


/*!
 * Draws a green sphere at each of the control points
 * @param controlCage
 * @param controlPointRad
 */
void drawControlPoints(const vector<glm::vec3>& controlPoints, const float controlPointRad) {
    glColor3f(0.0f, 1.0f, 0.0f);
    for (auto point : controlPoints) {
        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), point);
        glMultMatrixf(&transMtx[0][0]);
        {
            CSCI441::drawSolidSphere(controlPointRad, 10, 10);
        }
        glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    }
}

/*!
 * Draws a yellow line between the control points in strip style
 * @param controlPoints
 */
void drawControlPointConnections(const vector<glm::vec3>& controlPoints) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    {
        for (auto point : controlPoints) {
            glVertex3f(point.x, point.y, point.z);
        }
    };
    glEnd();
    glEnable(GL_LIGHTING);
}

/*!
 * Draws a line along a bezier curve defined by controlPoints
 * @param controlPoints the points defining the bezier curve
 */
void drawBezierCurve(const vector<glm::vec3>& controlPoints) {
    // Break the control cage into groups of four
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // Then draws curve defined by the groups
    // 1) Create a group of four, render it
    // 2) add the last member of that group to the next group
    // 3) goto 1)
    int i = 0;
    vector<glm::vec3> group;
    for (auto point : controlPoints) {
        group.push_back(point);
        i++;
        if (i == GROUP_SIZE) {
            glLineWidth(3.0f);
            renderBezierCurve(group.at(0), group.at(1), group.at(2), group.at(3), RESOLUTION);
            group.clear();
            group.push_back(point);
            i = 1;
        }
    }
}


/*!
 * Computes the position of a point along the bezier curve at time step t
 * @param controlPoints the points defining the bezier curve
 * @param t the timestep along the curve
 * @return [x,y,z]
 */
glm::vec3 computePositionBezierCurve(const vector<glm::vec3>& controlPoints, const float t) {
    // The bezier curve is grouped into fours
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // to find the position of the object given a dt we must
    // calculate which group defines the bezier curve we must the object is on

    // calculate which bezier curve the object is in based on dt
    int group = int(t) % int(floor(controlPoints.size()/3));
    int groupStartIndex = 3*group;

    glm::vec3 p0 = controlPoints.at(groupStartIndex);
    glm::vec3 p1 = controlPoints.at(groupStartIndex+1);
    glm::vec3 p2 = controlPoints.at(groupStartIndex+2);
    glm::vec3 p3 = controlPoints.at(groupStartIndex+3);
    float groupDt = fmod(t,1.0f) ;
    return evaluateBezierCurve(p0,p1,p2,p3,groupDt);
}




// loadControlPoints() /////////////////////////////////////////////////////////
//
//  Load our control points from file and store them in
//	the global variable controlPoints
//
////////////////////////////////////////////////////////////////////////////////
vector<glm::vec3> loadControlPoints(FILE* file) {
    // Read in control points from file.  Make sure the file can be
    // opened and handle it appropriately.  return false if there is an error
    // Check if the file can be opened
    // Read in the number of points
    int numPoints;
    vector<glm::vec3> controlPoints;
    fscanf(file, "%d", &numPoints);
    // Read in all the points
    for (int i = 0; i < numPoints; i++) {
        int x, y, z;
        fscanf(file, "%d,%d,%d", &x, &y, &z);
        controlPoints.emplace_back(glm::vec3(x,y,z));
    }
    return controlPoints;
}


#endif //A4_BEZIERCURVE_H
