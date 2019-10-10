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


/*!
 * maps a variables values based on old range and new range linearly
 * source: https://stackoverflow.com/questions/929103/convert-a-number-range-to-another-range-maintaining-ratio
 *
 * @tparam T any type that has support for - * / +
 * @param value the desired value to map to the range
 * @param fromLow the previous range low
 * @param fromHigh the previous range high
 * @param toLow the desired range low
 * @param toHigh the desired range high
 * @return
 */
template<typename T>
T variableMapping(T value, T fromLow, T fromHigh, T toLow, T toHigh){
    T newRange = toHigh - toLow;
    T oldRange = fromHigh - fromLow;
    T newValue = (((value - fromLow) * newRange) / oldRange) + toLow;
    return newValue;
}

/*!
 * Performs linear interpolation on t between A and B
 * @tparam T
 * @param A value at 0
 * @param B value at 1
 * @param t [0,1]
 * @return interpolated value
 */
template<typename T>
T lerp(T A, T B, T t){
    return A+(B-A)*t;
}






#endif //MP_HELPERFUNCTIONS_H
