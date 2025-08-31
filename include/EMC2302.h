#ifndef EMC2302_H
#define EMC2302_H

#include <Arduino.h>
#include <Wire.h>

class EMC2302 {
  public:
    EMC2302(uint8_t addr, uint8_t sda, uint8_t scl);
    void begin();
    void setFanDuty(uint8_t fan, uint8_t duty); // duty = 0-255
    uint16_t readFanRPM(uint8_t fan);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

  private:
    uint8_t _addr;
    TwoWire _sw;
};

#endif
