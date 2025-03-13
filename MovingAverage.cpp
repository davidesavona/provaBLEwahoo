#include "MovingAverage.h"


float MovingAverage::update(float new_value) {
    window.push_back(new_value);
    sum += new_value;

    if (window.size() > window_size) {
        sum -= window.front();
        window.pop_front();
    }

    return sum / window.size();  // Compute the moving average
}