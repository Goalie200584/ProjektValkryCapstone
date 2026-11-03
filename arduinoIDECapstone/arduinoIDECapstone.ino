//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#include <imu_regs.h>
#define sdaPin 21
#define sclPin 22

#define limit 12000 // Limit for flagging a potential crash in mg, 12 g's or 12000 mg's

TwoWire bus = TwoWire(1); // starts I2C
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW); // INitializes IMU

int32_t accelerometer[3]; // Accelerometer will hold 3 values for x y and z
int32_t gyroscope[3]; // gyroscope will hold 3 values for x y and z

int64_t start_time; 
int64_t end_time;
int64_t avg_time; // For discovering how long the "spike" was
int64_t values_time[50];

int idx;
int i;

bool regSetOK;

float values_accelerometer[50]; // Array for history of acceleration values for the total acceleration vector

int detectForCrash = 0;
int crash = 0;
int show = 1;



float power_of(float number, int power){
  float running = 1;
  for (int i = 0; i < power; i++){
    running = running * number;
  }
  return running;
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

  idx = 0;
  
}

void loop() {
  start_time = esp_timer_get_time();
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);
  end_time = esp_timer_get_time();

  avg_time = (start_time + end_time) / 2;

  values_accelerometer[idx] = sqrt((power_of((float)accelerometer[0], 2) + power_of((float)accelerometer[1], 2) + power_of((float)accelerometer[2], 2)));
  values_time[idx] = avg_time;
  


  if (values_accelerometer[idx] >= limit && show == 1){
    Serial.println(values_accelerometer[idx]);
    Serial.println("We have exceeded the limit my friend");
    show = 0;
  }
  else if (values_accelerometer[idx] < limit && show == 1){
    Serial.println(values_accelerometer[idx]);
  }


  idx++;
  if (idx >= 50){
    idx = 0;
  }



}


  

  

