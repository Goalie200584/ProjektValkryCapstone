//Test Script using ESP32 DEVBOARD
#include <Wire.h>
#include <ISM330DLCSensor.h>
#include <imu_regs.h>
#include <BluetoothSerial.h>
#include <SD.h>
#include <config.h>
#include <crash.h>



TwoWire bus = TwoWire(1); // starts I2C
ISM330DLCSensor AccGyr(&bus, ISM330DLC_ACC_GYRO_I2C_ADDRESS_LOW); // Initializes IMU
BluetoothSerial SerialBT;

int32_t accelerometer[3]; // Accelerometer will hold 3 values for x y and z
int32_t gyroscope[3]; // gyroscope will hold 3 values for x y and z

uint32_t start_time; 
uint32_t end_time;
uint32_t avg_time; // For discovering how long the "spike" was


int idx;
int i;

bool regSetOK;

int potentialCrash = 0;
int crash = 0;
bool isSDCard = false;
String crashFile = "/crashData.csv";
int16_t crash_accelerometer[five_second_length]; // For the 5 seconds of data AFTER a potential crash
int16_t crash_gyroscope[five_second_length];
uint32_t crash_times[five_second_length];
int crashIDX = 0; 

int16_t current_accelerometer;

int16_t history_accelerometer[two_second_length]; // For the 2 seconds of history BEFORE the crash 
int16_t history_gyroscope[two_second_length];
uint32_t history_times[two_second_length];

uint32_t largestAccelTime; //Records the time that the largest accel had taken place 
float largestAccel = 0; //records the magnitutde of the largest accleration during a crash
int largestAccel_idx;

bool canCrash = true;

float power_of(float number, int power){ // raises number to certain power
  float running = 1;
  for (int i = 0; i < power; i++){
    running = running * number;
  }
  return running;
}

int32_t standardize_xyz(int32_t values[3]){ // standardizes 3 vectors into a magnitude 
  return sqrt((power_of((float)values[0], 2) + power_of((float)values[1], 2) + power_of((float)values[2], 2)));
}

void initializeSD(){ // "Format" file 
  if(!isSDCard){
    return;
  }
  File file = SD.open(crashFile, FILE_WRITE);
  file.close();
}
void crash_data_to_SD( // Writes crash data to SD card from memory 
  int16_t history_accel[two_second_length], int16_t history_gyro[two_second_length], uint32_t hisory_times[two_second_length],
  int16_t accel[five_second_length], int16_t gyro[five_second_length], uint32_t times[five_second_length]
  ){

    if (!isSDCard){
      return;
    }
    File file = SD.open(crashFile, FILE_WRITE);
    file.println("accelerometer(mg's),gyroscope(dps),time(us)");

    for (int k = 0; k < two_second_length; k++){
      int slot = (idx + 1 + k) % two_second_length;
      file.print(history_accelerometer[slot]);
      file.print(",");
      file.print(history_gyroscope[slot]);
      file.print(",");
      file.println(history_times[slot]);
    }

    for(i = 0; i < crashIDX; i++){
      file.print(accel[i]);
      file.print(",");
      file.print(gyro[i]);
      file.print(",");
      file.println(times[i]);
    }

    file.close();
}



void setup() {
  Serial.begin(115200); 
  Serial.print("\n");
  pinMode(buzzerPin, OUTPUT);
  pinMode(cancelPin, INPUT);
  bus.begin(sdaPin, sclPin);
  bus.setClock(400000);

  AccGyr.begin();
  AccGyr.Enable_X();
  AccGyr.Enable_G();

  if (!imu::setRegister(bus, imu::CNTRL1_XL, imu::ODR_XL_833HZ, imu::FS_XL_16G)) {
    Serial.println("Accel config failed");
  } 

  SerialBT.begin("Projekt Valkry"); // Turns on Bluetooth Serial
  SerialBT.setTimeout(50); // Doesn't wait long if a line has no newline

  if (!SD.begin(5)) { // CS pin = 5
     Serial.println("Card mount failed");
     isSDCard = false;
  }
  else{
    Serial.println("SD Card Mounted");
    isSDCard = true;
    initializeSD();
  }

  idx = 0;
}

void loop() {
  start_time = esp_timer_get_time();
  AccGyr.Get_X_Axes(accelerometer);
  AccGyr.Get_G_Axes(gyroscope);
  end_time = esp_timer_get_time();

  avg_time = start_time + (end_time - start_time) / 2; // avoids overflow as opposed to a direct (start_time + end_time) / 2; averaging scheme
  current_accelerometer = standardize_xyz(accelerometer);
  if (potentialCrash != 1){
    history_accelerometer[idx] = standardize_xyz(accelerometer); // Standardizes acceleration to one vector
    history_gyroscope[idx] = standardize_xyz(gyroscope)/1000; // Standardizes gyro to one vector
    history_times[idx] = avg_time; 
  }

  // If potential crash detected i.e. if acceleration value is past threshold 
  if (current_accelerometer >= accel_limit && canCrash){
    Serial.println(current_accelerometer);
    Serial.println("We have exceeded the accel_limit my friend");

    potentialCrash = 1; // Notifies board that a crash may hvae occured 
  

    // Find peak of crash ie. find the highest number of g's exerted on the device
    if (largestAccel < current_accelerometer){
      largestAccel = current_accelerometer;
      largestAccelTime = avg_time;
      largestAccel_idx = crashIDX;
    }

  }
  else if (potentialCrash == 1 && current_accelerometer < accel_limit && canCrash){
    canCrash = false;
  }
  
  /*(history_)
    This portion stores the 5 secs AFTER the suspected crash 
    It stores all the values after the crash to be analyzed by the next portion of the if statement
  */
  if (potentialCrash == 1 && (avg_time - (largestAccelTime)) < 5000000){
    if (!(crashIDX > five_second_length - 1)){
      crash_accelerometer[crashIDX] = standardize_xyz(accelerometer); // Standardizes acceleration to one vector
      crash_gyroscope[crashIDX] = standardize_xyz(gyroscope)/1000; // Standardizes gyro to one vector
      crash_times[crashIDX] = avg_time;
      crashIDX++;
    }
  }

  /*DECISION PART OF THE CRASH TEST
    This if statement is after the 5 seconds of a potential crash
    this is where we analyze data from it, and determine if it is a crash or not 
  */


  /*
    - Needs to identify which axis is down BEFORE the crash happens so that we can distinguish between somehow who simply 
    stopped on a trail, vs someone who crashed then the bike stopped 
    - Needs to measure the amount of acceleration AFTER the potentialCrash happens; if there is one or two large but less 
    than the original value spikes then it s most likely a crash
    - Needs to combine the detection from the one or two accelerations with gyro data, if a crash, the gyro will often be 
    large and quick twists from the bike 
  */
  else if (potentialCrash == 1 && (avg_time - (largestAccelTime)) >= 5000000){ //If 5 second window after crash is over
    // crash_data_to_SD(history_accelerometer, history_gyroscope, history_times, crash_accelerometer, crash_gyroscope, crash_times); // Writes 5 secs after crash and 2 secs before to SD card
    Serial.println("Determining if Crash Occured");
    potentialCrash = 0;
    crash = crash_detection::detectCrash(
      history_accelerometer, history_gyroscope, history_times, 
      crash_accelerometer, crash_gyroscope, crash_times, 
      largestAccel_idx, crashIDX
    );
    if (crash == 1){
      Serial.println("Crash Occured!");
      tone(buzzerPin, 4000);
      delay(1000);
      noTone(buzzerPin);
    }
    else if (crash == 0){
      Serial.println("Crash Did not Occur");
    }
    
    largestAccel = 0;
    largestAccelTime = 0;
    largestAccel_idx = 0;
    crashIDX = 0;
    canCrash = true;
  }

  if (potentialCrash != 1){
    idx++;
  }
  if (idx >= two_second_length){
    idx = 0;
  }
}


  

  

