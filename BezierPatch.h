//
// Created by zsmeton on 9/26/19.
//

#ifndef MP_BEZIERPATCH_H
#define MP_BEZIERPATCH_H

#include <GL/gl.h>
#include <CSCI441/objects.hpp> // for our 3D objects
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>               // for vector data structure
#include <bits/stdc++.h>        // Fo
#include "BezierCurve.h"
#include "HelperFunctions.h"

using namespace std;
// helpers:
/*!
 * Checks if the follow inputs are correct
 * @param controlPoints is of size==16
 * @param u between [0,1]
 * @param v between [0,1]
 */
void _correctInput(const vector<glm::vec3>& controlPoints, float u, float v){
    assert(controlPoints.size() == 16);
    assert(0 <= u <= 1);
    assert(0 <= v <= 1);
}

/*!
 * Computes a location along a Bezier Patch. Given the a u and v position
 * @param controlPoints
 * @param u
 * @param v
 * @return [x,y,z]
 */
glm::vec3 _evaluateBezierPatch(vector<glm::vec3> controlPoints, float u, float v) {
    // Check that inputs are correct
    _correctInput(controlPoints,u,v);
    // Evaluate
    glm::vec3 bc0 = evaluateBezierCurve(controlPoints.at(0), controlPoints.at(1), controlPoints.at(2),
                                        controlPoints.at(3), u);
    glm::vec3 bc1 = evaluateBezierCurve(controlPoints.at(4), controlPoints.at(5), controlPoints.at(6),
                                        controlPoints.at(7), u);
    glm::vec3 bc2 = evaluateBezierCurve(controlPoints.at(8), controlPoints.at(9), controlPoints.at(10),
                                        controlPoints.at(11), u);
    glm::vec3 bc3 = evaluateBezierCurve(controlPoints.at(12), controlPoints.at(13), controlPoints.at(14),
                                        controlPoints.at(15), u);
    return evaluateBezierCurve(bc0, bc1, bc2, bc3, v);
}


/*!
 * Computes the derivative of the Bezier Patch. Given the time step and the computed coefficients
 * @param controlPoints
 * @param u
 * @param v
 * @return
 */
// TODO: IMPLEMENT
vector<glm::vec3> _evaluateBezierPatchDerivative(vector<glm::vec3> controlPoints, float u, float v) {
    _correctInput(controlPoints,u,v);
    // Derivative in v, find control points along v axis take derivative of bezier curve
    glm::vec3 divV0 = evaluateBezierCurve(controlPoints.at(0), controlPoints.at(1), controlPoints.at(2),
                                        controlPoints.at(3), u);
    glm::vec3 divV1 = evaluateBezierCurve(controlPoints.at(4), controlPoints.at(5), controlPoints.at(6),
                                        controlPoints.at(7), u);
    glm::vec3 divV2 = evaluateBezierCurve(controlPoints.at(8), controlPoints.at(9), controlPoints.at(10),
                                        controlPoints.at(11), u);
    glm::vec3 divV3 = evaluateBezierCurve(controlPoints.at(12), controlPoints.at(13), controlPoints.at(14),
                                        controlPoints.at(15), u);
    glm::vec3 divV = evaluateBezierCurveDerivative(divV0, divV1, divV2, divV3, v);

    // Derivative in v, find control points along v axis take derivative of bezier curve
    glm::vec3 divU0 = evaluateBezierCurve(controlPoints.at(0), controlPoints.at(4), controlPoints.at(8),
                                          controlPoints.at(12), v);
    glm::vec3 divU1 = evaluateBezierCurve(controlPoints.at(1), controlPoints.at(5), controlPoints.at(9),
                                          controlPoints.at(13), v);
    glm::vec3 divU2 = evaluateBezierCurve(controlPoints.at(2), controlPoints.at(6), controlPoints.at(10),
                                          controlPoints.at(14), v);
    glm::vec3 divU3 = evaluateBezierCurve(controlPoints.at(3), controlPoints.at(7), controlPoints.at(11),
                                          controlPoints.at(15), v);
    glm::vec3 divU = evaluateBezierCurveDerivative(divU0, divU1, divU2, divU3, u);

    vector<glm::vec3> result = vector<glm::vec3>({divU, divV});
    return result;
}

glm::vec3 _evaluateBezierPatchNormal(const vector<glm::vec3>& controlPoints, float u, float v){
    vector<glm::vec3> divUV = _evaluateBezierPatchDerivative(controlPoints, u, v);
    glm::vec3 norm = glm::normalize(glm::cross(divUV.at(0), divUV.at(1)));
    return norm;
}

/*!
 * Responsible for drawing a Bezier Patch as defined by sixteen control points.
 * Breaks the curve into n segments as specified by the resolution.
 *
 * @param controlPoints
 * @param resolution
 */
void _renderBezierPatch(const vector<glm::vec3>& controlPoints, int resolution) {
    assert(controlPoints.size() == 16);
    glColor3f(0.0f,0.6f,0.0f);
    glDisable(GL_COLOR_MATERIAL);
    GLfloat diffColorD[4] = { 0.0f,0.6f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffColorD);
    float spacing = 1 / float(resolution);
    for (float i = 0; i < 1; i += spacing) {
        for (float j = 0; j < 1; j += spacing) {
            glBegin(GL_TRIANGLE_STRIP);
            {              // Make this into a quad
                glm::vec3 norm0 = _evaluateBezierPatchNormal(controlPoints, i, j);
                glNormal3f(norm0.x,norm0.y,norm0.z);
                glm::vec3 p0 = _evaluateBezierPatch(controlPoints, i, j);
                glVertex3f(p0.x, p0.y, p0.z);
                glm::vec3 norm2 = _evaluateBezierPatchNormal(controlPoints, i + spacing, j);
                glNormal3f(norm2.x,norm2.y,norm2.z);
                glm::vec3 p2 = _evaluateBezierPatch(controlPoints, i + spacing, j);
                glVertex3f(p2.x, p2.y, p2.z);
                glm::vec3 norm1 = _evaluateBezierPatchNormal(controlPoints, i, j + spacing);
                glNormal3f(norm1.x,norm1.y,norm1.z);
                glm::vec3 p1 = _evaluateBezierPatch(controlPoints, i, j + spacing);
                glVertex3f(p1.x, p1.y, p1.z);
                glm::vec3 norm3 = _evaluateBezierPatchNormal(controlPoints, i + spacing, j + spacing);
                glNormal3f(norm3.x,norm3.y,norm3.z);
                glm::vec3 p3 = _evaluateBezierPatch(controlPoints, i + spacing, j + spacing);
                glVertex3f(p3.x, p3.y, p3.z);
            };glEnd();
        }
    }
    glEnable(GL_COLOR_MATERIAL);
}

/*!
 * Draws a bezier patch defined by controlPoints
 * @param controlPoints
 */
void drawBezierPatch(const vector<vector<vector<glm::vec3>>>& controlPoints) {
    for (const auto &row : controlPoints){
        for (const auto &column : row) {
            _renderBezierPatch(column, RESOLUTION);
        }
    }
}

/*!
 * Draws a green sphere at each of the control points
 * @param controlCage
 * @param controlPointRad
 */
 //TODO: multipe patches
void drawPatchControlPoints(const vector<glm::vec3> &controlPoints, const float controlPointRad) {
    drawControlPoints(controlPoints, controlPointRad);
}

/*!
 * Computes the position of a point along the bezier curve at time step t
 * @param controlPoints the points defining the bezier curve
 * @param u the x position [0,number_of_patches]
 * @param v the z position [0,number_of_patches]
 * @return [x,y,z]
 */
glm::vec3 computePositionBezierPatch(const vector<vector<vector<glm::vec3>>> &controlPoints, float u, float v) {
    return _evaluateBezierPatch(controlPoints.at(int(u)).at(int(v)), fmod(u,1.0f), fmod(v,1.0f));
}

glm::vec3 computeNormalBezierPatch(const vector<vector<vector<glm::vec3>>> &controlPoints, float u, float v){
    return _evaluateBezierPatchNormal(controlPoints.at(int(u)).at(int(v)), fmod(u,1.0f), fmod(v,1.0f));
}

glm::vec3 characterPos(const vector<vector<vector<glm::vec3>>> &controlPoints, glm::vec3 position) {
    position = computePositionBezierPatch(controlPoints, position.x,position.z);
    return position;
}

glm::vec3 characterNormal(const vector<vector<vector<glm::vec3>>> &controlPoints, glm::vec3 position){
    return computeNormalBezierPatch(controlPoints, position.x, position.z);
}
vector<vector<vector<glm::vec3>>> loadControlPointsBezierPatch(FILE* file){
    /*
     * the control points are loaded row by row across u then v
     * 32 control points [][]
     *
     * 48 control points []
     *                   [][]
     *
     * 64 control points [][]
     *                   [][]
     */

    int numPoints;
    fscanf(file, "%d", &numPoints);
    // Calculate number of rows and columns
    int rows = 1;
    float _columns = 1;
    for(int i = 2; i <= numPoints/16; i ++){
        _columns += 1;
        if(floor(_columns/rows) > rows +1){
            rows += 1;
        }
    }
    int columns = int(ceil(_columns)/rows);

    // Read in all the points
    vector<vector<vector<glm::vec3>>> controlPoints;
    for(int i = 0; i < rows; i ++) {
        vector<vector<glm::vec3>> row;
        for(int j = 0; j < columns; j ++) {
            vector<glm::vec3> column;
            for (int k = 0; k < 16; k++) {
                int x, y, z;
                fscanf(file, "%d,%d,%d", &x, &y, &z);
                column.emplace_back(glm::vec3(x, y, z));
            }
            row.emplace_back(column);
        }
        controlPoints.emplace_back(row);
    }
    return controlPoints;
}

vector<glm::vec3> generateRandomBezierPatch(int patchEnd, int patchStart, int height){
    vector<glm::vec3> controlPoints;
    for(int k = 0; k < 2; k++) {
        int spacing = (patchEnd-patchStart)/(6);
        int temp = (patchEnd-patchStart)/2;
        for (int i = patchStart+temp*k; i < patchEnd-temp*(1-k); i += spacing) {
            for (int j = patchStart; j < patchEnd; j++){}
        }
    }
    FILE* file;
    if((file = fopen("BezierSurface1.txt", "w"))) {
        auto intRound = [](float a) { return int(round(a)); };
        fprintf(file, "%zu\n", controlPoints.size());
        for (auto point : controlPoints) {
            fprintf(file, "%d,%d,%d\n", intRound(point.x), intRound(point.y), intRound(point.z));
        }
        fclose(file);
    }else{
        printf("Bezier patch was not put in file..");
    }
}

#endif //MP_BEZIERPATCH_H
