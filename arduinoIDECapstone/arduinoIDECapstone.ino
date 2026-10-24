#include <Wire.h>
#include <ISM330DLCSensor.h>
#define sdaPin 21
#define sclPin 22

TwoWire bus = TwoWire(1);
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW);

int32_t accelerometer[3];
int32_t gyroscope[3];

void setup() {
  Serial.begin(115200);
  bus.begin(sdaPin, sclPin);

  
  AccGyr.begin();
  AccGyr.Enable_X();
  AccGyr.Enable_G();

}

void loop() {
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);

  Serial.print("Accelerometer: ");
  Serial.println(accelerometer[0]);
  Serial.print("Gyroscope: ");
  Serial.println(gyroscope[0]);
  delay(100);

}


  

  

