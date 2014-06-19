/*
 * Interactive Coffee Table library
 * For controling features of the Arduino interative coffee table.
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 19/06/2014
 */

#include <InteractiveCoffeeTable.h>

/*
 * Pin config arrays
 */
uint8_t button_pins[NUM_BUTTONS] =
{
  PIN_BUTTON_UP,    PIN_BUTTON_DOWN,
  PIN_BUTTON_LEFT,  PIN_BUTTON_RIGHT,
  PIN_BUTTON_RED,   PIN_BUTTON_BLUE
};

uint8_t led_pins[NUM_LEDS] =
{
  PIN_LED_ORANGE,
  PIN_LED_GREEN
};

uint8_t control_pins[NUM_CONTROLS] =
{
  PIN_AMP_POWER,
  PIN_BLUETOOTH_POWER,
  PIN_AUDIO_SIGNAL
};

InteractiveCoffeeTable::InteractiveCoffeeTable()
{
  uint8_t i;

  buttonDebounceDelay = DEFAULT_BUTTON_DEBOUNCE_MS;
  buttonCallback = NULL;
  
  /*
   * Setup LED matrix object
   */
  matrix = new LedControl(
      PIN_MATRIX_DATA, PIN_MATRIX_CLOCK, PIN_MATRIX_LOAD,
      NUM_MATRIX_DRIVERS
    );

  for(i = 0; i < NUM_MATRIX_DRIVERS; i++)
  {
    matrix->shutdown(i, 0);
    matrix->setIntensity(i, 8);
    matrix->clearDisplay(i);
  }

  /*
   * Setup 20x4 LCD
   */
  lcd[LCD_20X4] = new LiquidCrystal(
      LCD_20X4_RS, LCD_20X4_EN,
      LCD_20X4_D4, LCD_20X4_D5, LCD_20X4_D6, LCD_20X4_D7
    );
  lcd[LCD_20X4]->begin(20, 4);

  /*
   * Setup 40x2 LCD
   */
  lcd[LCD_40X2] = new LiquidCrystal(
      LCD_40X2_RS, LCD_40X2_EN,
      LCD_40X2_D4, LCD_40X2_D5, LCD_40X2_D6, LCD_40X2_D7
    );
  lcd[LCD_40X2]->begin(40, 2);

  /*
   * Configure button pins
   */
  for(i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(button_pins[i], INPUT);
    digitalWrite(button_pins[i], HIGH);
  }

  /*
   * Configure LED pins
   */
  for(i = 0; i < NUM_LEDS; i++)
  {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], LOW);
  }

  /*
   * Configure controls pins
   */
  for(i = 0; i < NUM_CONTROLS; i++)
  {
    pinMode(control_pins[i], OUTPUT);
    digitalWrite(control_pins[i], LOW);
  }
}

InteractiveCoffeeTable::~InteractiveCoffeeTable()
{
  uint8_t i;

  delete matrix;
  delete lcd;
}

uint8_t InteractiveCoffeeTable::getButtonState(ICT_Button button)
{
  return lastButtonState[button];
}

void InteractiveCoffeeTable::setButtonCallback(void (* callback) (ICT_Button button, uint8_t state))
{
  buttonCallback = callback;
}

uint16_t InteractiveCoffeeTable::getButtonDebounceTime()
{
  return buttonDebounceDelay;
}

void InteractiveCoffeeTable::setButtonDebounceTime(uint16_t time)
{
  buttonDebounceDelay = time;
}

void InteractiveCoffeeTable::pollButtons()
{
  uint8_t i;
  for(i = 0; i < NUM_BUTTONS; i++)
  {
    if((millis() - lastButtonEdgeTime[i]) >= buttonDebounceDelay)
    {
      uint8_t state = !digitalRead(button_pins[i]);
      if(state != lastButtonState[i])
      {
        if(buttonCallback != NULL)
          buttonCallback((ICT_Button) i, state);
        lastButtonState[i] = state;
        lastButtonEdgeTime[i] = millis();
      }
    }
  }
}

uint8_t InteractiveCoffeeTable::getLEDState(ICT_LED led)
{
  return digitalRead(led_pins[led]);
}

void InteractiveCoffeeTable::setLED(ICT_LED led, uint8_t state)
{
  digitalWrite(led_pins[led], state);
}

uint8_t InteractiveCoffeeTable::getControlState(ICT_Control control)
{
  return digitalRead(control_pins[control]);
}

void InteractiveCoffeeTable::setControl(ICT_Control control, uint8_t state)
{
  digitalWrite(control_pins[control], state);
}

void InteractiveCoffeeTable::lcdPrint(ICT_LCD lcd, uint8_t row, uint8_t column, char *text)
{
  this->lcd[lcd]->setCursor(column, row);
  this->lcd[lcd]->print(text);
}

void InteractiveCoffeeTable::lcdClear(ICT_LCD lcd)
{
  this->lcd[lcd]->clear();
}
