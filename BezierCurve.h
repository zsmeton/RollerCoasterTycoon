//
// Created by zsmeton on 9/16/19.
//

#ifndef A4_BEZIERCURVE_H
#define A4_BEZIERCURVE_H

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <vector>               // for vector data structure
#include <bits/stdc++.h>        // For fmod()
using namespace std;

// CHANGING THESE VARIABLES WILL BREAK A LOT OF THINGS SO PLEASE DON'T
const int GROUP_SIZE = 4;
const int RESOLUTION = 20;

// evaluateBezierCurve() ////////////////////////////////////////////////////////
//
// Computes a location along a Bezier Curve. Given the time step and the
//  precomputed coefficients
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurve(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float t) {
    glm::vec3 point = static_cast<float>(pow(t, 3)) * a + static_cast<float>(pow(t, 2)) * b + t * c + d;
    return point;
}

// evaluateBezierCurveDerivative() ////////////////////////////////////////////////////////
//
// Computes the derivative of the Bezier Curve. Given the time step and the
//  precomputed coefficients
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurveDerivative(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float t) {
    glm::vec3 point = 2.0f*static_cast<float>(pow(t, 2)) * a + 2*static_cast<float>(pow(t, 1)) * b + c;
    return point;
}

// computeCoefficients()////////////////////////////////////////////////////////
//
// Computes the coefficients for evaluateBezierCurve() based on the given control
//  points. modifies the passed in controlPoints
//
////////////////////////////////////////////////////////////////////////////////
void convertControlToCoefficients(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3) {
    glm::vec3 a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    glm::vec3 b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    glm::vec3 c = -3.0f * p0 + 3.0f * p1;
    glm::vec3 d = p0;
    p0 = a;
    p1 = b;
    p2 = c;
    p3 = d;
}

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution) {
    glm::vec3 a = p0;
    glm::vec3 b = p1;
    glm::vec3 c = p2;
    glm::vec3 d = p3;

    // Precompute a,b,c,d
    convertControlToCoefficients(a,b,c,d);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_STRIP);
    {
        for (float i = 0; i < 1.0f + 1.0f / resolution; i += 1.0f / resolution) {
            glm::vec3 curvePoint = evaluateBezierCurve(a,b,c,d, i);
            glVertex3f(curvePoint.x, curvePoint.y, curvePoint.z);
        }
    };
    glEnd();
    glEnable(GL_LIGHTING);
}

// drawBezierCurve() //////////////////////////////////////////////////////////////////
//
//  Function to draw a blue line along a bezier curve defined by controlCage
//
////////////////////////////////////////////////////////////////////////////////
void drawBezierCurve(const vector<glm::vec3>& controlCage) {
    // Break the control cage into groups of four
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // Then draws curve defined by the groups
    // 1) Create a group of four, render it
    // 2) add the last member of that group to the next group
    // 3) goto 1)
    int i = 0;
    vector<glm::vec3> group;
    for (auto point : controlCage) {
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

// computePositionBezierCurve() //////////////////////////////////////////////////////////////////
//
//  Returns the x,y,z position of an object moving along a bezier curve
//  defined by the input controlCage
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 computePositionBezierCurve(const vector<glm::vec3>& controlCage, const float dt) {
    // The bezier curve is grouped into fours
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // to find the position of the object given a dt we must
    // calculate which group defines the bezier curve we must the object is on

    // calculate which bezier curve the object is in based on dt
    int group = int(dt) % int(controlCage.size()/3);
    int groupStartIndex = 4*group;
    if(groupStartIndex != 0){
        groupStartIndex -=1;
    }
    glm::vec3 a = controlCage.at(groupStartIndex);
    glm::vec3 b = controlCage.at(groupStartIndex+1);
    glm::vec3 c = controlCage.at(groupStartIndex+2);
    glm::vec3 d = controlCage.at(groupStartIndex+3);
    float groupDt = fmod(dt,1.0f) ;
    convertControlToCoefficients(a,b,c,d);
    return evaluateBezierCurve(a,b,c,d,groupDt);



}

//  Returns the x,y,z position of an object moving along a bezier curve
//  defined by the input controlCage
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 computeRotationBezierCurve(const vector<glm::vec3>& controlCage, const float dt) {
    // The bezier curve is grouped into fours
    // i.e. p1,p2,p3,p4,p5,p6,p7 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    //      p1,p2,p3,p4,p5,p6,p7,p8 -> (p1,p2,p3,p4),(p4,p5,p6,p7)
    // to find the position of the object given a dt we must
    // calculate which group defines the bezier curve we must the object is on

    // calculate which bezier curve the object is in based on dt
    int group = int(dt) % int(controlCage.size()/3);
    int groupStartIndex = 4*group;
    if(groupStartIndex != 0){
        groupStartIndex -=1;
    }
    glm::vec3 a = controlCage.at(groupStartIndex);
    glm::vec3 b = controlCage.at(groupStartIndex+1);
    glm::vec3 c = controlCage.at(groupStartIndex+2);
    glm::vec3 d = controlCage.at(groupStartIndex+3);
    float groupDt = fmod(dt,1.0f) ;
    convertControlToCoefficients(a,b,c,d);
    return evaluateBezierCurveDerivative(a,b,c,d,groupDt);



}

// drawControlPoints() //////////////////////////////////////////////////////////////////
//
//  Function to draw a green sphere at each of the control points
//
////////////////////////////////////////////////////////////////////////////////
void drawControlPoints(const vector<glm::vec3>& controlCage, const float controlPointRad) {
    glColor3f(0.0f, 1.0f, 0.0f);
    for (auto point : controlCage) {
        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), point);
        glMultMatrixf(&transMtx[0][0]);
        {
            CSCI441::drawSolidSphere(controlPointRad, 10, 10);
        }
        glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
    }
}

// drawControlPoints() //////////////////////////////////////////////////////////////////
//
//  Function to draw a yellow line between the control points in strip style
//
////////////////////////////////////////////////////////////////////////////////
void drawControlPointConnections(const vector<glm::vec3>& controlCage) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    {
        for (auto point : controlCage) {
            glVertex3f(point.x, point.y, point.z);
        }
    };
    glEnd();
    glEnable(GL_LIGHTING);
}

#endif //A4_BEZIERCURVE_H
