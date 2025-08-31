#ifndef PCA9632_H
#define PCA9632_H

#include <Arduino.h>
#include <Wire.h>

class PCA9632
{
public:
    // Constructor: addr = I2C, sda/scl = pins
    PCA9632(uint8_t addr, uint8_t sda, uint8_t scl);

    void begin();

    // Basic control
    void setLEDs(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0); // direct set
    void setChannel(uint8_t channel, uint8_t value);              // single channel
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    // Fade API
    void fadeToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0, uint16_t duration_ms = 1000, uint16_t steps = 50);

    // Map logical color to physical channel (0-3)
    void mapColors(uint8_t r_chan, uint8_t g_chan, uint8_t b_chan, uint8_t w_chan);

private:
    uint8_t _addr;
    TwoWire _sw;

    uint8_t _ledMap[4];     // logical color -> physical channel
    uint8_t _currentPWM[4]; // store current PWM values

    void _updateHardware();
};

#endif
