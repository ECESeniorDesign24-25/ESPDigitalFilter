#include "Hysteresis.h"

//==================================================================================================
float hysteresis(float arr[], int m) {
    // takes in an array of values of size m and returns the average of the last m values
    float sum = 0;
    for(int i = 0; i< m; i++)
    {
        sum += arr[i];
    }
    return sum/m;
}