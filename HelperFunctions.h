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


// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
float getRand() { return rand() / (float) RAND_MAX; }


#endif //MP_HELPERFUNCTIONS_H
