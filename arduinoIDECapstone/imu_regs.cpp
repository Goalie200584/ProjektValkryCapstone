#include "imu_regs.h"

namespace imu {

    bool writeReg(TwoWire &bus, uint8_t reg, uint8_t value) {
        bus.beginTransmission(ADDR);
        bus.write(reg);
        bus.write(value);
        return bus.endTransmission() == 0;       
    }

    bool readReg(TwoWire &bus, uint8_t reg, uint8_t &value) {
      bus.beginTransmission(ADDR);
      bus.write(reg);
      if (bus.endTransmission(false) != 0) return false;
      if (bus.requestFrom(ADDR, (uint8_t)1) != 1) return false;
        value = bus.read();
      return true;
    }
    bool setRegister(TwoWire &bus, uint8_t writeAddress, uint8_t odrBits, uint8_t fsBits) {
    uint8_t value = (odrBits << 4) | (fsBits << 2); 

    if (!writeReg(bus, writeAddress, value)) return false;
    
    uint8_t check = 0;

    if (!readReg(bus, writeAddress, check)) return false;
        return check == value;
    }

}