#include "Hysteresis.h"

//==================================================================================================
float hysteresis_avg(float arr[], int m) {
    // takes in an array of values of size m and returns the average of the last m values
    float sum = 0;
    for(int i = 0; i< m; i++)
    {
        sum += arr[i];
    }
    return sum/m;
}

//==================================================================================================
float hysteresis_max(float arr[], int m) {
    // takes in an array of values of size m and returns the average of the last m values
    float max = 0;
    for(int i = 0; i< m; i++)
    {
        if (arr[i] > max) {
          max = arr[i];
        }
    }
    return max;
}