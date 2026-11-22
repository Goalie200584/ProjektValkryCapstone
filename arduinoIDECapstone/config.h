#pragma once

#define sdaPin 21
#define sclPin 22
#define accel_limit 5000 // Limit for flagging a potential crash in mg, 12 g's or 12000 mg's set at 13,000 to account for gravity
#define degrees_limit 90 // limit for flagging whether the bike is upright or not 
#define still_low_limit 950
#define still_high_limit 1100
#define single_stationary_time_limit 1 // gives a limit for the amount of time being still to be considered stationary after the crash 
#define total_stationary_time_limit 3 // limit for the full 5 secs
#define GYRO_LIMIT 5 // limit in dps

#define frequency 833
#define buzzerPin 25
#define cancelPin 33

const int two_second_length = frequency * 2;
const int five_second_length = frequency * 5;