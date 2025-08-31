#include "PCA9632.h"

// //  REGISTERS PCA9632
// #define PCA9632_MODE1               0x00
// #define PCA9632_MODE2               0x01
// #define PCA9632_PWM0                0x02
// #define PCA9632_PWM1                0x03
// #define PCA9632_PWM2                0x04
// #define PCA9632_PWM3                0x05
// #define PCA9632_GRPPWM              0x06
// #define PCA9632_GRPFREQ             0x07

// #define PCA9632_LEDOUT              0x08
// #define PCA9632_SUBADR1             0x09
// #define PCA9632_SUBADR2             0x0A
// #define PCA9632_SUBADR3             0x0B
// #define PCA9632_SUBADR(x)           (0x08 +(x))  //  x = 0..3
// #define PCA9632_ALLCALLADR          0x0C


// //  ERROR CODES - compatible with PCA9634
// #define PCA9632_OK                  0x00
// #define PCA9632_ERROR               0xFF
// #define PCA9632_ERR_WRITE           0xFE
// #define PCA9632_ERR_CHAN            0xFD
// #define PCA9632_ERR_MODE            0xFC
// #define PCA9632_ERR_REG             0xFB
// #define PCA9632_ERR_I2C             0xFA


// //  Configuration bits MODE1 REGISTER
// #define PCA9632_MODE1_AUTOINCR2     0x80  //  ReadOnly,  0 = disable  1 = enable
// #define PCA9632_MODE1_AUTOINCR1     0x40  //  ReadOnly,  bit1
// #define PCA9632_MODE1_AUTOINCR0     0x20  //  ReadOnly,  bit0
// #define PCA9632_MODE1_SLEEP         0x10  //  0 = normal       1 = sleep
// #define PCA9632_MODE1_SUB1          0x08  //  0 = disable      1 = enable
// #define PCA9632_MODE1_SUB2          0x04  //  0 = disable      1 = enable
// #define PCA9632_MODE1_SUB3          0x02  //  0 = disable      1 = enable
// #define PCA9632_MODE1_ALLCALL       0x01  //  0 = disable      1 = enable
// #define PCA9632_MODE1_NONE          0x00
// #define PCA9632_MODE1_DEFAULT       0x81


// //  Configuration bits MODE2 REGISTER
// #define PCA9632_MODE2_BLINK         0x20  //  0 = dim          1 = blink
// #define PCA9632_MODE2_INVERT        0x10  //  0 = normal       1 = inverted
// #define PCA9632_MODE2_ACK           0x08  //  0 = on STOP      1 = on ACK
// #define PCA9632_MODE2_TOTEMPOLE     0x04  //  0 = open drain   1 = totem-pole
// #define PCA9632_MODE2_NONE          0x00
// #define PCA9632_MODE2_DEFAULT       0x02  //  fix #2

// //  modi for LEDOUT REGISTER (4x shifted))
// #define PCA9632_LEDOFF              0x00  //  default @ startup
// #define PCA9632_LEDON               0x01
// #define PCA9632_LEDPWM              0x02
// #define PCA9632_LEDGRPPWM           0x03

PCA9632::PCA9632(uint8_t addr, uint8_t sda, uint8_t scl) 
    : _addr(addr), _sw(sda, scl) 
{
    // Default mapping: R=0, G=1, B=2, W=3
    _ledMap[0]=0; _ledMap[1]=1; _ledMap[2]=2; _ledMap[3]=3;
    for(int i=0;i<4;i++) _currentPWM[i]=0;
}

void PCA9632::begin() {
    _sw.begin();
    _sw.setTimeout(500);
    writeRegister(PCA9632_MODE1, 0x00);   // normal mode
    writeRegister(PCA9632_MODE2, 0x04);   // totem-pole
    writeRegister(PCA9632_LEDOUT, 0xAA);  // all PWM controlled
    _updateHardware();
}

// Map logical colors to physical outputs
void PCA9632::mapColors(uint8_t r_chan, uint8_t g_chan, uint8_t b_chan, uint8_t w_chan) {
    _ledMap[0] = r_chan & 0x03;
    _ledMap[1] = g_chan & 0x03;
    _ledMap[2] = b_chan & 0x03;
    _ledMap[3] = w_chan & 0x03;
}

// Set RGBW values immediately
void PCA9632::setLEDs(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    _currentPWM[_ledMap[0]] = constrain(r,0,255);
    _currentPWM[_ledMap[1]] = constrain(g,0,255);
    _currentPWM[_ledMap[2]] = constrain(b,0,255);
    _currentPWM[_ledMap[3]] = constrain(w,0,255);
    _updateHardware();
}

// Set individual channel
void PCA9632::setChannel(uint8_t channel, uint8_t value) {
    if(channel<4){
        _currentPWM[channel] = constrain(value,0,255);
        _updateHardware();
    }
}

// Read current PWM values
uint8_t PCA9632::getLEDPWM(uint8_t logicalColor) {
    if (logicalColor > 3) return 0;
    return _currentPWM[_ledMap[logicalColor]];
}


// Write PWM registers to hardware
void PCA9632::_updateHardware() {
    _sw.beginTransmission(_addr);
    _sw.write(PCA9632_PWM0 | 0x80);  // auto-increment
    for(int i=0;i<4;i++){
        _sw.write(_currentPWM[i]);
    }
    _sw.endTransmission();
}

// Basic I2C read/write
void PCA9632::writeRegister(uint8_t reg, uint8_t value){
    _sw.beginTransmission(_addr);
    _sw.write(reg);
    _sw.write(value);
    _sw.endTransmission();
}

uint8_t PCA9632::readRegister(uint8_t reg){
    _sw.beginTransmission(_addr);
    _sw.write(reg);
    _sw.endTransmission(false);
    _sw.requestFrom(_addr,(uint8_t)1);
    if(_sw.available()) return _sw.read();
    return 0xFF;
}

// Fade smoothly to color
void PCA9632::fadeToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w,
                          uint16_t duration_ms, uint16_t steps)
{
    float delta[4];
    delta[_ledMap[0]] = (float)r - _currentPWM[_ledMap[0]];
    delta[_ledMap[1]] = (float)g - _currentPWM[_ledMap[1]];
    delta[_ledMap[2]] = (float)b - _currentPWM[_ledMap[2]];
    delta[_ledMap[3]] = (float)w - _currentPWM[_ledMap[3]];

    for(uint16_t step=1; step<=steps; step++){
        for(int i=0;i<4;i++){
            _currentPWM[i] = (uint8_t)constrain(round(_currentPWM[i] + delta[i]/steps), 0, 255);
        }
        _updateHardware();
        delay(duration_ms/steps);
    }
    // Ensure exact final values
    setLEDs(r,g,b,w);
}
