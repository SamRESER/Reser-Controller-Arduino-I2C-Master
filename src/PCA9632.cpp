#include "PCA9632.h"

// Register map
#define PCA9632_MODE1   0x00
#define PCA9632_MODE2   0x01
#define PCA9632_PWM0    0x02
#define PCA9632_LEDOUT  0x08

PCA9632::PCA9632(uint8_t addr, uint8_t sda, uint8_t scl) 
: _addr(addr), _sw(sda, scl) {}

void PCA9632::begin() {
  _sw.begin();
  _sw.setTimeout(500);

  writeRegister(PCA9632_MODE1, 0x00); // normal mode
  writeRegister(PCA9632_MODE2, 0x04); // totem-pole outputs
  writeRegister(PCA9632_LEDOUT, 0x2A); // LED0-2 use PWM, LED3 off
}

void PCA9632::setRGB(uint8_t r, uint8_t g, uint8_t b) {
  _sw.beginTransmission(_addr);
  _sw.write(PCA9632_PWM0 | 0x80); // auto-increment from PWM0
  _sw.write(r);
  _sw.write(g);
  _sw.write(b);
  _sw.endTransmission();
}

void PCA9632::setChannel(uint8_t channel, uint8_t value) {
  writeRegister(PCA9632_PWM0 + channel, value);
}

void PCA9632::setAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  _sw.beginTransmission(_addr);
  _sw.write(PCA9632_PWM0 | 0x80); // auto-increment from PWM0
  _sw.write(r);
  _sw.write(g);
  _sw.write(b);
  _sw.write(w);
  _sw.endTransmission();
}

void PCA9632::writeRegister(uint8_t reg, uint8_t value) {
  _sw.beginTransmission(_addr);
  _sw.write(reg);
  _sw.write(value);
  _sw.endTransmission();
}

uint8_t PCA9632::readRegister(uint8_t reg) {
  _sw.beginTransmission(_addr);
  _sw.write(reg);
  _sw.endTransmission(false);
  _sw.requestFrom(_addr, (uint8_t)1);
  if (_sw.available()) {
    return _sw.read();
  }
  return 0xFF;
}
