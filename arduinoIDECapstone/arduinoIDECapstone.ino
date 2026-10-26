//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#define sdaPin 21
#define sclPin 22

#define limit 5000 // Limit for flagging a potential crash in mg, 12 g's or 12000 mg's

TwoWire bus = TwoWire(1); // starts I2C
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW); // INitializes IMU

int32_t accelerometer[3]; // Accelerometer will hold 3 values for x y and z
int32_t gyroscope[3]; // gyroscope will hold 3 values for x y and z

int64_t start_time; 
int64_t end_time;
int64_t avg_time; // For discovering how long the "spike" was

int32_t values_accelerometer_x[50]; // Arra for history of accelerometer values
int32_t values_accelerometer_y[50];
int32_t values_accelerometer_z[50];
int64_t values_time[50];

int idx;
int i;

int detectForCrash = 0;
int crash = 0;

void setup() {
  Serial.begin(115200);
  bus.begin(sdaPin, sclPin);

  
  AccGyr.begin();
  AccGyr.Enable_X();
  AccGyr.Enable_G();

  idx = 0;

}

void loop() {
  start_time = esp_timer_get_time();
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);
  end_time = esp_timer_get_time();

  avg_time = (start_time + end_time) / 2;

  values_accelerometer_x[idx] = accelerometer[0];
  values_accelerometer_y[idx] = accelerometer[1];
  values_accelerometer_z[idx] = accelerometer[2];
  values_time[idx] = avg_time;


  for(i = 0; i <= 2; i++){
    if (accelerometer[i] >= limit){
      Serial.println("We have exceeded the my friend");
    }
    else{
      Serial.println("We have NOT exceeded the limit");
    }
  }
  idx++;

  if (idx >= 50){
    idx = 0;
  }



}


  

  

