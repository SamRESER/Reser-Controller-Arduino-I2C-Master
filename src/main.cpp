#include "PCA9632.h"
#include "EMC2302.h"

#define SDA_PIN 10
#define SCL_PIN 11

PCA9632 ledDriver(0x60, SDA_PIN, SCL_PIN);
EMC2302 fanCtrl(0x2E, SDA_PIN, SCL_PIN);

// LED Demo Variables
bool ledDemo = false;
uint8_t ledStart[4], ledTarget[4];
uint32_t ledStartTime;
uint16_t ledDuration = 2000;

// Fan Demo Variables
bool fanDemo = false;
uint8_t fanStart[2], fanTarget[2];
uint32_t fanStartTime;
uint16_t fanDuration = 2000;

// --- TRANSITIONS/EASING FUNCTIONS --- //

enum EasingType
{
  LINEAR,
  EASE_IN_OUT_QUAD,
  EASE_IN_QUAD,
  EASE_OUT_QUAD
};
EasingType ledEasing = EASE_IN_OUT_QUAD;
EasingType fanEasing = EASE_IN_OUT_QUAD;

// Easing functions
float applyEasing(float t, EasingType type)
{
  switch (type)
  {
  case LINEAR:
    return t;
  case EASE_IN_OUT_QUAD:
    return (t < 0.5) ? 2 * t * t : -1 + (4 - 2 * t) * t;
  case EASE_IN_QUAD:
    return t * t;
  case EASE_OUT_QUAD:
    return t * (2 - t);
  }
  return t; // fallback
}

uint8_t lerpWithEasing(uint8_t start, uint8_t target, float t, EasingType type)
{
  float eased = applyEasing(t, type);
  return start + (target - start) * eased;
}

// --- DEMO FUNCTIONS --- //

void setNextLEDDemoTarget()
{
  ledStartTime = millis();
  for (int i = 0; i < 4; i++)
    ledStart[i] = ledDriver.readRegister(0x02 + i);
  ledTarget[0] = random(0, 256);
  ledTarget[1] = random(0, 256);
  ledTarget[2] = random(0, 256);
  ledTarget[3] = 0;
}

void setNextFanDemoTarget()
{
  fanStartTime = millis();
  for (int i = 0; i < 2; i++)
    fanStart[i] = fanTarget[i];
  fanTarget[0] = random(50, 200);
  fanTarget[1] = random(50, 200);
}

// --- COMMANDS --- //

void cmdLED(char *args)
{
  uint8_t r = 0, g = 0, b = 0, w = 0;
  char *token;
  while ((token = strtok(NULL, " ")) != NULL)
  {
    if (token[0] == 'R')
      r = atoi(strtok(NULL, " "));
    else if (token[0] == 'G')
      g = atoi(strtok(NULL, " "));
    else if (token[0] == 'B')
      b = atoi(strtok(NULL, " "));
    else if (token[0] == 'W')
      w = atoi(strtok(NULL, " "));
  }
  ledDriver.setLEDs(r, g, b, w);
  ledDemo = false; // exit LED demo
}

void cmdFAN(char *args)
{
  uint8_t fan = atoi(strtok(NULL, " "));
  uint8_t duty = atoi(strtok(NULL, " "));
  fanCtrl.setFanDuty(fan, duty);
  fanDemo = false; // exit fan demo
}

void cmdGET(char *args)
{
  char *token = strtok(NULL, " ");
  if (strcmp(token, "FAN") == 0)
  {
    uint8_t fan = atoi(strtok(NULL, " "));
    Serial.print("FAN ");
    Serial.print(fan);
    Serial.print(" RPM: ");
    Serial.println(fanCtrl.readFanRPM(fan));
  }
  else if (strcmp(token, "LED") == 0)
  {
    Serial.print("LED R: ");
    Serial.print(ledDriver.readRegister(0x02));
    Serial.print(" G: ");
    Serial.print(ledDriver.readRegister(0x03));
    Serial.print(" B: ");
    Serial.print(ledDriver.readRegister(0x04));
    Serial.print(" W: ");
    Serial.println(ledDriver.readRegister(0x05));
  }
}

void cmdLEDD(char *args)
{
  ledDemo = true;
  setNextLEDDemoTarget();

  if (args)
  {
    // Easing
    if (strstr(args, "LINEAR"))
      ledEasing = LINEAR;
    else if (strstr(args, "INOUT"))
      ledEasing = EASE_IN_OUT_QUAD;
    else if (strstr(args, "IN"))
      ledEasing = EASE_IN_QUAD;
    else if (strstr(args, "OUT"))
      ledEasing = EASE_OUT_QUAD;

    // Optional duration
    char *durStr = strstr(args, " "); // first space
    if (durStr)
    {
      uint16_t dur = atoi(durStr);
      if (dur > 0)
        ledDuration = dur;
    }
  }
}

void cmdFAND(char *args)
{
  fanDemo = true;
  setNextFanDemoTarget();

  if (args)
  {
    // Easing
    if (strstr(args, "LINEAR"))
      fanEasing = LINEAR;
    else if (strstr(args, "INOUT"))
      fanEasing = EASE_IN_OUT_QUAD;
    else if (strstr(args, "IN"))
      fanEasing = EASE_IN_QUAD;
    else if (strstr(args, "OUT"))
      fanEasing = EASE_OUT_QUAD;

    // Optional duration
    char *durStr = strstr(args, " "); // first space
    if (durStr)
    {
      uint16_t dur = atoi(durStr);
      if (dur > 0)
        fanDuration = dur;
    }
  }
}

void cmdSTOP(char *args)
{
  ledDemo = false;
  fanDemo = false;
}

// --- SERIAL COMMAND HANDLER --- //

struct Command
{
  const char *name;
  void (*callback)(char *);
};

// Command table
Command commands[] = {
    {"LED", cmdLED},
    {"FAN", cmdFAN},
    {"GET", cmdGET},
    {"LEDD", cmdLEDD},
    {"FAND", cmdFAND},
    {"STOP", cmdSTOP}};
const int numCommands = sizeof(commands) / sizeof(commands[0]);

void handleSerial()
{
  static char input[64];
  if (Serial.available())
  {
    size_t len = Serial.readBytesUntil('\n', input, sizeof(input) - 1);
    input[len] = '\0';
    char *cmd = strtok(input, " ");
    if (cmd == NULL)
      return;
    for (int i = 0; i < numCommands; i++)
    {
      if (strcmp(cmd, commands[i].name) == 0)
      {
        commands[i].callback(strtok(NULL, ""));
        break;
      }
    }
  }
}

// --- MAIN --- //

void setup()
{
  Serial.begin(9600);

  ledDriver.begin();
  fanCtrl.begin();

  ledDriver.mapColors(0, 1, 2, 3);
  ledDriver.setLEDs(0, 0, 0, 0);

  // Initialize demo variables
  for (int i = 0; i < 4; i++)
    ledStart[i] = ledTarget[i] = 0;
  for (int i = 0; i < 2; i++)
    fanStart[i] = fanTarget[i] = 0;
  ledStartTime = fanStartTime = millis();
}

void loop()
{
  handleSerial();

  uint32_t now = millis();

  // LED demo
  if (ledDemo)
  {
    float t = (float)(now - ledStartTime) / ledDuration;
    if (t > 1.0)
      t = 1.0;
    uint8_t r = lerpWithEasing(ledStart[0], ledTarget[0], t, ledEasing);
    uint8_t g = lerpWithEasing(ledStart[1], ledTarget[1], t, ledEasing);
    uint8_t b = lerpWithEasing(ledStart[2], ledTarget[2], t, ledEasing);
    uint8_t w = lerpWithEasing(ledStart[3], ledTarget[3], t, ledEasing);
    ledDriver.setLEDs(r, g, b, w);
    if (t >= 1.0)
      setNextLEDDemoTarget();
  }

  // Fan demo
  if (fanDemo)
  {
    float t = (float)(now - fanStartTime) / fanDuration;
    if (t > 1.0)
      t = 1.0;
    uint8_t fan0 = lerpWithEasing(fanStart[0], fanTarget[0], t, fanEasing);
    uint8_t fan1 = lerpWithEasing(fanStart[1], fanTarget[1], t, fanEasing);
    fanCtrl.setFanDuty(0, fan0);
    fanCtrl.setFanDuty(1, fan1);
    if (t >= 1.0)
      setNextFanDemoTarget();
  }
}
