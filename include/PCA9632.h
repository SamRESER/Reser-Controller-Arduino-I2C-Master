#ifndef PCA9632_H
#define PCA9632_H

#include <SoftwareWire.h>
#include <Arduino.h>

// PCA9632 Registers
#define PCA9632_MODE1 0x00
#define PCA9632_MODE2 0x01
#define PCA9632_PWM0 0x02
#define PCA9632_PWM1 0x03
#define PCA9632_PWM2 0x04
#define PCA9632_PWM3 0x05
#define PCA9632_LEDOUT 0x08

class PCA9632
{
public:
    PCA9632(uint8_t addr, uint8_t sda, uint8_t scl);

    void begin();
    void mapColors(uint8_t r_chan, uint8_t g_chan, uint8_t b_chan, uint8_t w_chan);

    void setLEDs(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void setChannel(uint8_t channel, uint8_t value);
    uint8_t getLEDPWM(uint8_t logicalColor);

    void fadeToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w,
                     uint16_t duration_ms, uint16_t steps);

    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);

private:
    uint8_t _addr;
    SoftwareWire _sw;
    uint8_t _ledMap[4];
    uint8_t _currentPWM[4];

    void _updateHardware();
};

#endif
