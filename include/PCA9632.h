#ifndef PCA9632_H
#define PCA9632_H

#include <Arduino.h>
#include <SoftWire.h>

class PCA9632 {
  public:
    PCA9632(uint8_t addr, uint8_t sda, uint8_t scl);
    void begin();
    void setRGB(uint8_t r, uint8_t g, uint8_t b);
    void setChannel(uint8_t channel, uint8_t value);
    void setAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

  private:
    uint8_t _addr;
    SoftWire _sw;
};

#endif
