//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#include <imu_regs.h>
#include <BluetoothSerial.h>

#define sdaPin 21
#define sclPin 22
#define limit 12000 // Limit for flagging a potential crash in mg, 12 g's or 12000 mg's

#define frequency 833

const int two_second_length = frequency * 2;
const int five_second_length = frequency * 5;

TwoWire bus = TwoWire(1); // starts I2C
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW); // Initializes IMU
BluetoothSerial SerialBT;

int32_t accelerometer[3]; // Accelerometer will hold 3 values for x y and z
int32_t gyroscope[3]; // gyroscope will hold 3 values for x y and z

int32_t start_time; 
int32_t end_time;
int32_t avg_time; // For discovering how long the "spike" was


int idx;
int i;

bool regSetOK;

int potentialCrash = 0;
int crash = 0;
int16_t crash_accelerometer[five_second_length]; // For the 5 seconds of data AFTER a potential crash
int16_t crash_gyroscope[five_second_length];
int32_t crash_times[five_second_length];
int crashIDX; 

int16_t history_accelerometer[two_second_length]; // For the 2 seconds of history BEFORE the crash 
int16_t history_gyroscope[two_second_length];
int32_t history_times[two_second_length];

int64_t largestAccelTime = 0;
float largestAccel = 0;

int64_t delta_t;

float power_of(float number, int power){
  float running = 1;
  for (int i = 0; i < power; i++){
    running = running * number;
  }
  return running;
}

int16_t standardize_xyz(int32_t values[3]){
  return sqrt((power_of((float)accelerometer[0], 2) + power_of((float)accelerometer[1], 2) + power_of((float)accelerometer[2], 2)));
}

void setup() {
  Serial.begin(115200); 
  bus.begin(sdaPin, sclPin);

  AccGyr.begin();
  AccGyr.Enable_X();
  AccGyr.Enable_G();

  if (!imu::setRegister(bus, imu::CNTRL1_XL, imu::ODR_XL_833HZ, imu::FS_XL_16G)) {
    Serial.println("Accel config failed");
  } 

  SerialBT.begin("Projekt Valkry"); // Turns on Bluetooth Serial
  SerialBT.setTimeout(50); // Doesn't wait long if a line has no newline

  idx = 0;
  
}

void loop() {
  start_time = esp_timer_get_time();
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);
  end_time = esp_timer_get_time();

  avg_time = (start_time + end_time) / 2;

  history_accelerometer[idx] = standardize_xyz(accelerometer); // Standardizes acceleration to one vector
  history_gyroscope[idx] = standardize_xyz(gyroscope); // Standardizes gyro to one vector
  history_times[idx] = avg_time; 
  

  // If potential crash detected 
  if (history_accelerometer[idx] >= limit){
    Serial.println(history_accelerometer[idx]);
    Serial.println("We have exceeded the limit my friend");

    potentialCrash = 1; 
    largestAccelTime = avg_time; // also t0 

    // Find peak of crash ie. find the highest number of g's exerted on the device
    if (largestAccel < history_accelerometer[idx]){
      largestAccel = history_accelerometer[idx];
    }

  }
  /*
    This portion stores the 5 secs AFTER the suspected crash 
    It stores all the values after the crash to be analyzed by the next portion of the if statement
  */
  else if (potentialCrash == 1 && (history_times[idx] - ((largestAccelTime)/1000) < 5)){
    crashIDX = five_second_length;
    for(crashIDX = five_second_length; crashIDX > 0; i--){

      start_time = esp_timer_get_time();
      AccGyr.Get_X_Axes(accelerometer);
      AccGyr.Get_G_Axes(gyroscope);
      end_time = esp_timer_get_time();

      avg_time = (start_time + end_time) / 2;

      crash_accelerometer[crashIDX] = standardize_xyz(accelerometer); // Standardizes acceleration to one vector
      crash_gyroscope[crashIDX] = standardize_xyz(gyroscope); // Standardizes gyro to one vector
      crash_times[crashIDX] = avg_time; 

    }
    
  }

  /*DECISION PART OF THE CRASH TEST
    This if statement is after the 5 seconds of a potential crash
    this is where we analyze data from it, and determine if it is a crash or not 
  */
  else if (potentialCrash == 1 && (history_times[idx] - ((largestAccelTime)/1000) >= 5)){ //If 5 second window after crash is over
    delta_t = history_times[idx] - ((largestAccelTime)/1000);
    
    potentialCrash = 0;
  }

  idx++;
  if (idx >= two_second_length){
    idx = 0;
  }
}


  

  

