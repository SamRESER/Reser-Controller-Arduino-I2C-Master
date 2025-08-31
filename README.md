# RGBW + Fan Control with PCA9632 & EMC2302

## Overview

This repository provides **Arduino libraries and a sketch** to control:

- RGBW LEDs via **PCA9632** I²C LED driver  
- Two fans via **EMC2302** I²C fan controller  

Features include:

- Non-blocking **LED fades** and **fan ramps**  
- Independent **demo modes** for LEDs and fans  
- Configurable **easing functions** and **duration** for demo mode  
- **Serial command interface** for manual control  
- Extensible for future devices or commands  

## Hardware Connections

### Arduino Mega

| Signal | PCA9632 | EMC2302 |
|--------|---------|---------|
| SDA    | SDA     | SDA     |
| SCL    | SCL     | SCL     |
| GND    | GND     | GND     |
| VCC    | 3.3V/5V | 3.3V/5V |

- RGBW LED connected to PCA9632 outputs (LED0–LED3)  
- Fans connected to EMC2302 PWM outputs  

## Installation

1. Copy `PCA9632.h/cpp` and `EMC2302.h/cpp` into your Arduino `libraries` folder  
2. Include them in your sketch:

```cpp
#include "PCA9632.h"
#include "EMC2302.h"
```

3. Compile and upload to your Arduino Mega  

## Serial Command Interface

Commands are **space-separated strings** terminated by newline (`\n`).  

### LED Control

| Command | Description |
|---------|-------------|
| `LED R <0-255> G <0-255> B <0-255> W <0-255>` | Set LED color manually |
| `LEDD [EASING] [DURATION]` | Start LED demo; optional easing (`LINEAR`, `INOUT`, `IN`, `OUT`) and duration in ms |
| `STOP` | Stop all demos |

### Fan Control

| Command | Description |
|---------|-------------|
| `FAN <0\|1> <0-255>` | Set PWM duty for fan 0 or 1 |
| `FAND [EASING] [DURATION]` | Start fan demo; optional easing and duration |
| `STOP` | Stop all demos |

### State Queries

| Command | Description |
|---------|-------------|
| `GET LED` | Return current RGBW values |
| `GET FAN <0\|1>` | Return RPM of specified fan |

## Demo Mode Behavior

- LEDs and fans **can run independent demos** simultaneously  
- Demos **loop indefinitely** until manually overridden or stopped  
- **Easing and duration** can be changed dynamically with commands  

## Adding New Serial Commands

The sketch uses a **table-driven command parser**, making it easy to add new commands without changing the core logic.

### How it Works

1. Each command is represented as a struct:

```cpp
struct Command {
  const char* name;
  void (*callback)(char* args);
};
```

2. All commands are stored in an array:

```cpp
Command commands[] = {
  {"LED", cmdLED},
  {"FAN", cmdFAN},
  {"GET", cmdGET},
  {"LEDD", cmdLEDD},
  {"FAND", cmdFAND},
  {"STOP", cmdSTOP}
};
```

3. When a line is received over Serial, the sketch iterates through the array and calls the associated callback if the command matches.

### Adding a New Command

1. **Write a callback function**:

```cpp
void cmdMYCMD(char* args){
  Serial.println("MYCMD executed!");
}
```

2. **Add it to the command table**:

```cpp
Command commands[] = {
  {"LED", cmdLED},
  {"FAN", cmdFAN},
  {"GET", cmdGET},
  {"LEDD", cmdLEDD},
  {"FAND", cmdFAND},
  {"STOP", cmdSTOP},
  {"MYCMD", cmdMYCMD}
};
```

3. Optionally parse arguments using `strtok()` or `strstr()`.

### Tips for Extensibility

- Keep each callback **short and non-blocking**.  
- Use **global or static state** if multi-step actions are needed.  
- Commands can include **optional parameters** like easing or duration.  

## Example Usage

```
LEDD INOUT 3000   // LED demo with ease-in-out, 3-second fade
FAND LINEAR 1500  // Fan demo linear ramp, 1.5-second duration
LED R 255 G 128 B 64 W 0  // Manual LED set
FAN 0 128  // Manual fan 0 duty
GET FAN 1  // Query fan 1 RPM
STOP       // Stop all demos
```

## Getting Started Example

```cpp
#include "PCA9632.h"
#include "EMC2302.h"

PCA9632 ledDriver(0x60, SDA, SCL);
EMC2302 fanCtrl(0x2E, SDA, SCL);

void setup() {
  Serial.begin(9600);
  ledDriver.begin();
  fanCtrl.begin();
  ledDriver.mapColors(0,1,2,3);
  ledDriver.setLEDs(0,0,0,0);

  // Start LED demo with ease-in-out, 3-second fade
  Serial.println("LEDD INOUT 3000");
  // Start fan demo linear, 1.5-second duration
  Serial.println("FAND LINEAR 1500");
}

void loop() {
  // Call handleSerial() from the main sketch loop to parse commands
}
```

## Notes

- Both libraries use **SoftWire** for non-blocking I²C communication  
- RGBW mapping is configurable via `mapColors()`  
- Default LED mapping: R→LED0, G→LED1, B→LED2, W→LED3  
- Fans 0 and 1 correspond to EMC2302 PWM outputs  
