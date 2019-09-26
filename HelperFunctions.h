//
// Created by zsmeton on 9/25/19.
//

#ifndef MP_HELPERFUNCTIONS_H
#define MP_HELPERFUNCTIONS_H
template<typename T>
T restrictVariable(T data, T min, T max) {
    data = (data < min) ? min : data;
    data = (data > max) ? max : data;
    return data;
}

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
        }
        return true;
    }
    return false;
}


#endif //MP_HELPERFUNCTIONS_H
