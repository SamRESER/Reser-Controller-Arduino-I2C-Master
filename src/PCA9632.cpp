#include "PCA9632.h"

#define PCA9632_MODE1   0x00
#define PCA9632_MODE2   0x01
#define PCA9632_PWM0    0x02
#define PCA9632_LEDOUT  0x08

PCA9632::PCA9632(uint8_t addr, uint8_t sda, uint8_t scl) 
: _addr(addr), _sw(sda, scl) 
{
  // Default color mapping: R->0, G->1, B->2, W->3
  _ledMap[0] = 0; _ledMap[1] = 1; _ledMap[2] = 2; _ledMap[3] = 3;
  for(int i=0;i<4;i++) _currentPWM[i] = 0;
}

void PCA9632::begin() {
  _sw.begin();
  _sw.setTimeout(500);

  writeRegister(PCA9632_MODE1, 0x00); // normal mode
  writeRegister(PCA9632_MODE2, 0x04); // totem pole
  writeRegister(PCA9632_LEDOUT, 0xAA); // all PWM controlled
  _updateHardware();
}

// Map logical colors to physical outputs
void PCA9632::mapColors(uint8_t r_chan, uint8_t g_chan, uint8_t b_chan, uint8_t w_chan) {
  _ledMap[0] = r_chan & 0x03;
  _ledMap[1] = g_chan & 0x03;
  _ledMap[2] = b_chan & 0x03;
  _ledMap[3] = w_chan & 0x03;
}

// Directly set RGBW values
void PCA9632::setLEDs(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  _currentPWM[_ledMap[0]] = r;
  _currentPWM[_ledMap[1]] = g;
  _currentPWM[_ledMap[2]] = b;
  _currentPWM[_ledMap[3]] = w;
  _updateHardware();
}

void PCA9632::setChannel(uint8_t channel, uint8_t value) {
  if(channel < 4) {
    _currentPWM[channel] = value;
    _updateHardware();
  }
}

void PCA9632::_updateHardware() {
  _sw.beginTransmission(_addr);
  _sw.write(PCA9632_PWM0 | 0x80); // auto-increment
  for(int i=0;i<4;i++) {
    _sw.write(_currentPWM[i]);
  }
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
  _sw.requestFrom(_addr,(uint8_t)1);
  if(_sw.available()) return _sw.read();
  return 0xFF;
}

// Smooth fade
void PCA9632::fadeToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w, uint16_t duration_ms, uint16_t steps) {
  int16_t delta[4];
  delta[_ledMap[0]] = (int16_t)r - _currentPWM[_ledMap[0]];
  delta[_ledMap[1]] = (int16_t)g - _currentPWM[_ledMap[1]];
  delta[_ledMap[2]] = (int16_t)b - _currentPWM[_ledMap[2]];
  delta[_ledMap[3]] = (int16_t)w - _currentPWM[_ledMap[3]];

  for(uint16_t step=1; step<=steps; step++) {
    for(int i=0;i<4;i++) {
      _currentPWM[i] = _currentPWM[i] + delta[i]/steps;
    }
    _updateHardware();
    delay(duration_ms / steps);
  }

  // Ensure final values are exact
  setLEDs(r,g,b,w);
}
