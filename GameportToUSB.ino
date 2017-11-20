#include "debounce.h"

#define LED_BUILTIN PB12 // change to match your board

//
// This requires an stm32f1 board compatible with the no-grounding-pin feature of ADCTouchSensor,
// and this branch of the stm32f1 core: https://github.com/arpruss/Arduino_STM32/tree/addMidiHID
//
// Either gameport or Keyboard+Mouse+gameport mode. 
//
// Pinout for reading gameport analog values:
// Gameport 1 (X1) A0 --10K-- ground (X)
// Gameport 6 (Y1) A1 --10K-- ground (Y)
// Gameport 11 (X2) A2 --10K-- ground (slider 1)
// Gameport 13 (Y2) A3 --10K-- ground (Z rotate)
// Gameport 1, 8, 9, 15 -- 3.3V
// Gameport 4, 5, 12 -- GND
// Gameport 2 (B1) -- A4 (button 1)
// Gameport 7 (B2) -- A5 (button 2)
// Gameport 14 (B3) -- A6 (button 3)
// Gameport 10 (B4) -- A7 (button 4)

const uint32_t maximumAnalogValue = 4095;
const uint32_t minimumAnalogValue = 4095 * 10 / 100;
const uint32_t gameportButtonPins[] = { PA4, PA5, PA6, PA7 };
const unsigned numGameportButtonPins = sizeof(gameportButtonPins) / sizeof(*gameportButtonPins);

typedef void (HIDJoystick::*JoystickSetter)(uint16_t);

typedef struct {
  uint32 pin;
  JoystickSetter set;
  bool reverse;
} JoystickAxis;

JoystickAxis axes[] = {
    { PA0, &HIDJoystick::X, false },
    { PA1, &HIDJoystick::Y, false },
    { PA2, &HIDJoystick::sliderRight, false },
    { PA3, &HIDJoystick::Xrotate, false }
};

const unsigned numAxes = sizeof(axes) / sizeof(*axes);

Debounce* gameportButtons[numGameportButtonPins];

void gameportSetup() {
    for (int i=0; i<numGameportButtonPins; i++) {
        pinMode(gameportButtonPins[i], INPUT_PULLUP);
        gameportButtons[i] = new Debounce(gameportButtonPins[i], LOW);
    }

    for (int i=0; i<numAxes; i++) {
        pinMode(axes[i].pin, INPUT_ANALOG);
    }
}

void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);
    
    digitalWrite(LED_BUILTIN, 1);     
    gameportSetup();
    Joystick.setManualReportMode(true);
} 

uint16_t remap(uint16_t analogValue, bool value) {
    uint32_t v = analogValue;
    if (v < minimumAnalogValue)
      return 0;
    else {
      v = (v - minimumAnalogValue) * 4095 / ( maximumAnalogValue - minimumAnalogValue );
      if (v > 4095)
        return 4095;
      return v;
    }
}


void loop() 
{
    uint8_t pressed = 0;

    for (int i=0; i<numGameportButtonPins; i++)
       Joystick.button(i+1, gameportButtons[i]->getState());

    for (int i=0; i<numAxes; i++)
       (Joystick.*(axes[i].set))(remap(analogRead(axes[i].pin), axes[i].reverse));

    Joystick.sendManualReport();

    delay(10);
}

