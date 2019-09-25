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



#endif //MP_HELPERFUNCTIONS_H
