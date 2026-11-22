#pragma once
#include <Arduino.h>
#include <config.h>

namespace crash_detection {

    bool inside_limit(int16_t value, int low_limit, int high_limit);
    
    bool detectCrash(
        int16_t history_accel[two_second_length], int16_t history_gyro[two_second_length], uint32_t hisory_times[two_second_length],
        int16_t accel[five_second_length], int16_t gyro[five_second_length], uint32_t times[five_second_length], 
        int largestAccelIDX, int crashIDX
    );
}