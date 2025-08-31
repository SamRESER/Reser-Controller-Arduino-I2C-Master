#include <Arduino.h>
#include "PCA9632.h"
#include "EMC2302.h"

// Define I2C pins
#define SDA_PIN 10
#define SCL_PIN 11

// Devices
PCA9632 ledDriver(0x60, SDA_PIN, SCL_PIN);
EMC2302 fanCtrl(0x2E, SDA_PIN, SCL_PIN); // example address

void setup() {
  Serial.begin(9600);
  
  ledDriver.begin();
  fanCtrl.begin();
}

void loop() {
  // Demo: cycle LED colors
  ledDriver.setRGB(255, 0, 0);
  delay(1000);
  ledDriver.setRGB(0, 255, 0);
  delay(1000);
  ledDriver.setRGB(0, 0, 255);
  delay(1000);

  // Demo: control fans
  fanCtrl.setFanDuty(0, 128); // 50% duty for fan 1
  fanCtrl.setFanDuty(1, 200); // ~78% duty for fan 2

  // Read RPM
  uint16_t rpm1 = fanCtrl.readFanRPM(0);
  uint16_t rpm2 = fanCtrl.readFanRPM(1);
  Serial.print("Fan1 RPM: "); Serial.println(rpm1);
  Serial.print("Fan2 RPM: "); Serial.println(rpm2);

  delay(2000);
}
