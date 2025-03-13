#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#pragma once

#include <iostream>
#include <deque>

class MovingAverage {
    private:
        std::deque<float> window;
        size_t window_size;
        float sum;
    
    public:
        MovingAverage(size_t size) : window_size(size), sum(0.0) {}
        float update(float new_value);
    };

#endif