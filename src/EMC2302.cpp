#include "EMC2302.h"

// Register map (simplified)
#define EMC2302_FAN1_PWM  0x30
#define EMC2302_FAN2_PWM  0x31
#define EMC2302_FAN1_TACH 0x3E // high byte, low byte at 0x3F
#define EMC2302_FAN2_TACH 0x40 // high byte, low byte at 0x41

EMC2302::EMC2302(uint8_t addr, uint8_t sda, uint8_t scl) 
: _addr(addr), _sw() {}

void EMC2302::begin() {
  _sw.begin();
  _sw.setTimeout(500);
  // Put fans into manual PWM mode (default is usually auto)
  writeRegister(0x20, 0x01); // Example: FAN1 config
  writeRegister(0x22, 0x01); // Example: FAN2 config
}

void EMC2302::setFanDuty(uint8_t fan, uint8_t duty) {
  uint8_t reg = (fan == 0) ? EMC2302_FAN1_PWM : EMC2302_FAN2_PWM;
  writeRegister(reg, duty);
}

uint16_t EMC2302::readFanRPM(uint8_t fan) {
  uint8_t reg = (fan == 0) ? EMC2302_FAN1_TACH : EMC2302_FAN2_TACH;
  uint8_t high = readRegister(reg);
  uint8_t low  = readRegister(reg + 1);
  uint16_t tach = ((uint16_t)high << 5) | (low >> 3);
  if (tach == 0) return 0;
  // RPM = (clock * 60) / (tach * poles)
  // EMC2302 default clock = 32kHz, poles=2 (usually)
  return (uint32_t)3932160UL / tach;
}

void EMC2302::writeRegister(uint8_t reg, uint8_t value) {
  _sw.beginTransmission(_addr);
  _sw.write(reg);
  _sw.write(value);
  _sw.endTransmission();
}

uint8_t EMC2302::readRegister(uint8_t reg) {
  _sw.beginTransmission(_addr);
  _sw.write(reg);
  _sw.endTransmission(false);
  _sw.requestFrom(_addr, (uint8_t)1);
  if (_sw.available()) {
    return _sw.read();
  }
  return 0xFF;
}
