#pragma once
#include <Arduino.h>
#include <Wire.h>

namespace imu{
    constexpr uint8_t ADDR = 0x6A;

    // Register Address
    constexpr uint8_t CNTRL1_XL = 0x10; // base address for whole reg
    constexpr uint8_t CNTRL1_XL_FS_XL_1 = 0x0C; 

    constexpr uint8_t ODR_XL_416HZ = 0b0110;
    constexpr uint8_t ODR_XL_833HZ = 0b0111;
    constexpr uint8_t ODR_XL_1660HZ = 0b1000;

    constexpr uint8_t FS_XL_16G = 0b01; 
    constexpr uint8_t FS_XL_8G = 0b11;
    constexpr uint8_t FS_XL_4G = 0b10;
    constexpr uint8_t FS_XL_2G = 0b00;
    

    bool writeReg(TwoWire &bus, uint8_t reg, uint8_t value);
    bool readReg(TwoWire &bus, uint8_t reg, uint8_t &value);
    bool setRegister(TwoWire &bus, uint8_t writeAddress, uint8_t odrBits, uint8_t fsBits);
}