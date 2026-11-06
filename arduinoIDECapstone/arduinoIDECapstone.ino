//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#include <imu_regs.h>
#include <BluetoothSerial.h>
#include <vector>
#include <SD.h>
#include <SPI.h>

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

std::vector<float> output_accelerometer;
std::vector<int64_t> output_times;
int output_arr_idx = 0;

String output_line;
String new_filename;

float values_accelerometer[50]; // Array for history of acceleration values for the total acceleration vector

int detectForCrash = 0;
int crash = 0;
int show = 0;
int reading = 0;
int writing = 0;
int file_number = 0;

String filename = "data";


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

  if (cmd == "start"){
    reading = 1;
    writing = 0;
    new_filename = "/" + filename + (String)file_number + ".txt";
    file_number++;
    File file = SD.open(new_filename, FILE_WRITE);
    file.close();
  }
  
  else if (cmd == "stop"){
    reading = 0;
    writing = 1;
  }
}
void writeToSD(){
  SerialBT.println("ENTERED WRITE");
  
  File file = SD.open(new_filename, FILE_APPEND);
  for(i = 0; i < output_accelerometer.size(); i++){
    output_line = (String)output_accelerometer[i] + "mg's " + output_times[i] + "Time";
    file.println(output_line);
  }
  file.close();
  output_accelerometer.clear();
  output_times.clear();
}

void identify_files(){
  int current_file_number = -1;
  File root = SD.open("/");
  File entry = root.openNextFile();

  

  while(entry){
    String name = entry.name();     
    if (name.startsWith("/")) name = name.substring(1);      
    int dot = name.indexOf('.');          
    String base = name.substring(0, dot); 
    if (base.substring(0, 4) == filename){
      current_file_number = base.substring(4).toInt() + 1;  
      if (current_file_number > file_number){
        file_number = current_file_number;
      }
    }
    
    Serial.println(entry.name());
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
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
  output_accelerometer.reserve(2000);
  output_times.reserve(2000);

  if (!SD.begin(5)) { // CS pin = 5
     Serial.println("Card mount failed");
    return;
  }
  Serial.println("SD card initialized successfully.");

  identify_files();
  Serial.println(file_number);

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

  

  
  if (reading == 1 && writing == 0){
    output_accelerometer.push_back(values_accelerometer[idx]);
    output_times.push_back(values_time[idx]);
    output_arr_idx++;
  }
  else if (reading == 0 && writing == 1){
    writeToSD();
    SerialBT.println("Writing to SD Card Finished");
    writing = 0;
    output_arr_idx = 0;
  }
  
  if (reading == 1 && output_arr_idx >= 2000){
      output_arr_idx = 0;
      writeToSD();
      SerialBT.println("Writing to SD Card");
    }


  idx++;

  if (idx >= 50){
    idx = 0;
  }

}


  

  

