//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#include <imu_regs.h>
#include <BluetoothSerial.h>
#define sdaPin 21
#define sclPin 22

#define limit 12000 // Limit for flagging a potential crash in mg, 12 g's or 12000 mg's

TwoWire bus = TwoWire(1); // starts I2C
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW); // INitializes IMU
BluetoothSerial SerialBT;

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
int show = 0;



float power_of(float number, int power){
  float running = 1;
  for (int i = 0; i < power; i++){
    running = running * number;
  }
  return running;
}

void readingsSwitch(){
  if (!SerialBT.available()) return;
  String cmd = SerialBT.readStringUntil('\n');
  cmd.trim(); // strip \r and \n 
  cmd.toLowerCase(); // turns to lower case

  if (cmd == "on"){
    show = 1;
  }
  else if (cmd == "off"){
    show = 0;
  }
}

String fmt(float v, const char *unit) { // Formats Strings for readability in SerialBT.println()
  if (isnan(v)) return "ER";
  return String((int)round(v)) + unit;
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
  readingsSwitch();
  
  start_time = esp_timer_get_time();
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);
  end_time = esp_timer_get_time();

  avg_time = (start_time + end_time) / 2;

  values_accelerometer[idx] = sqrt((power_of((float)accelerometer[0], 2) + power_of((float)accelerometer[1], 2) + power_of((float)accelerometer[2], 2))); // Standardizes X, Y and Z axis into one vector
  values_time[idx] = avg_time;

  if (show == 1){
    SerialBT.print(fmt(values_accelerometer[idx], "mg's; "));
    SerialBT.println(fmt(values_time[idx], "Time"));

    Serial.print(fmt(values_accelerometer[idx], "mg's; "));
    Serial.println(fmt(values_time[idx], "Time"));
  }



  idx++;
  if (idx >= 50){
    idx = 0;
  }



}


  

  

