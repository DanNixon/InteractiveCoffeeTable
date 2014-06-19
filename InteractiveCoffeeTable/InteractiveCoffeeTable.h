/*
 * Interactive Coffee Table library
 * For controling features of the Arduino interative coffee table.
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 19/06/2014
 */

#ifndef INTERACTIVECOFFEETABLE_H
#define INTERACTIVECOFFEETABLE_H

#include <Arduino.h>

#include <LiquidCrystal.h>
#include <LedControl.h>
#include <inttypes.h>

#include <pins.h>

#define DEFAULT_BUTTON_DEBOUNCE_MS 200

#define NUM_MATRIX_DRIVERS 4

enum ICT_Button
{
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_RED,
  BUTTON_BLUE
};

#define NUM_BUTTONS 6

enum ICT_LED
{
  LED_ORANGE,
  LED_GREEN
};

#define NUM_LEDS 2

enum ICT_Control
{
  CONTROL_AMP_POWER,
  CONTROL_BT_POWER,
  CONTROL_AUDIO_SIGNAL
};

#define NUM_CONTROLS 3

enum ICT_LCD
{
  LCD_20X4,
  LCD_40X2
};

#define NUM_LCDS 2

class InteractiveCoffeeTable
{
  public:
    InteractiveCoffeeTable();
    ~InteractiveCoffeeTable();

    uint8_t getButtonState(ICT_Button button);
    void setButtonCallback(void (* callback) (ICT_Button button, uint8_t state));

    uint16_t getButtonDebounceTime();
    void setButtonDebounceTime(uint16_t time);
    void pollButtons();

    uint8_t getLEDState(ICT_LED led);
    void setLED(ICT_LED led, uint8_t state);

    uint8_t getControlState(ICT_Control control);
    void setControl(ICT_Control control, uint8_t state);

    void lcdPrint(ICT_LCD lcd, uint8_t row, uint8_t column, char *text);
    void lcdClear(ICT_LCD lcd);

  private:
    uint16_t buttonDebounceDelay;
    uint8_t lastButtonState[NUM_BUTTONS];
    uint32_t lastButtonEdgeTime[NUM_BUTTONS];

    void (*buttonCallback) (ICT_Button button, uint8_t state);

    LiquidCrystal *lcd[NUM_LCDS];
    LedControl *matrix;
};

#endif
