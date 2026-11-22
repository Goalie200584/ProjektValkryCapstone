#include <crash.h>
#include <config.h>

namespace crash_detection{

    bool inside_limit(int16_t value, int low_limit, int high_limit){
        if (low_limit <= value && value <= high_limit){
            return true;
        }
        else {
            return false;
        }
    }
    bool is_still(
        int16_t accelValue, int accelLowlimit, int accelHighLimit, 
        int16_t gyroValue, int gyroLimit
    ){
        bool bigAccel = !inside_limit(accelValue, accelLowlimit, accelHighLimit);

        if (!bigAccel && gyroValue <= gyroLimit){
            return true;
        }
        else {
            return false;
        }
    }

    bool detectCrash(
        int16_t history_accel[two_second_length], int16_t history_gyro[two_second_length], uint32_t history_times[two_second_length],
        int16_t accel[five_second_length], int16_t gyro[five_second_length], uint32_t times[five_second_length], 
        int largestAccelIDX, int crashIDX
    ){ 
        int16_t currentAccel; // stored in mg's = mm/s^2
        int16_t currentGyro; // stored in dps = degrees per second = d/s
        uint32_t currentTime; //stored in us = microseconds

        bool ifStill;
        bool spinTrigger = false;
        bool stationary = false;

        int firstStillIDX = -1; // -1 if not stationary everything else is its index
        uint32_t first_still_time = 0;
        int lastStillIDX = -1; // -1 if not stationary, 0 if stationary until the end, anothing else if its index
        uint32_t last_still_time = 0;
        float longest_still_time = -1;
        float still_time_elapsed = -1;
        float total_still_time = 0;


        float degreesRotated = 0;
        float timeSinceLast;
        int lastIDX = crashIDX - 1;
        

        for (int k = 1; k < two_second_length; k++){
            currentGyro = history_gyro[k]; // rotations
            currentTime = history_times[k]; // and times

            if (((times[largestAccelIDX] - currentTime) / 1000000.0f) < 1){
                timeSinceLast = (currentTime - history_times[k-1]) / 1000000.0f;
                degreesRotated = degreesRotated + (currentGyro * timeSinceLast);
            }

             
        }

        for (int i = largestAccelIDX + 1; i <= crashIDX - 1; i++){
            currentAccel = accel[i]; // stores current values for accelerations
            currentGyro = gyro[i]; // rotations
            currentTime = times[i]; // and times

            timeSinceLast = (currentTime - times[i-1]) / 1000000.0f; // finds time elapsed since last sample in seconds

            degreesRotated = degreesRotated + (currentGyro * timeSinceLast); 

            ifStill = is_still(currentAccel, still_low_limit, still_high_limit, gyro[i], GYRO_LIMIT);
            
            if (!stationary && ifStill){ // catches any inital moment that appears to be still
                firstStillIDX = i;
                first_still_time = times[i];
                stationary = true;
            }
            else if (stationary && !ifStill){ // catches any movement after a moement that appears to be still
                stationary = false;
                lastStillIDX = i;
                last_still_time = times[i];
                still_time_elapsed = ((last_still_time - first_still_time) / 1000000.0f);
                total_still_time += still_time_elapsed;
                
                if (still_time_elapsed > longest_still_time){
                    longest_still_time = still_time_elapsed;
                }
                still_time_elapsed = -1;  
            }

        } 

        if (stationary){ // after loop if bike is still stationary, make the last time the end still time 
            lastStillIDX = lastIDX;
            last_still_time = times[lastIDX];
            still_time_elapsed = ((last_still_time - first_still_time) / 1000000.0f);
            total_still_time += still_time_elapsed;

            if (still_time_elapsed > longest_still_time){
                longest_still_time = still_time_elapsed;
            }
                still_time_elapsed = -1;  
        }
        
        if (longest_still_time < single_stationary_time_limit && total_still_time < total_stationary_time_limit){
            return false; 
        }
        else{
            if (degreesRotated >= degrees_limit){
                spinTrigger = true;
                return true;
            }  
            return false;
        }

    }


}
