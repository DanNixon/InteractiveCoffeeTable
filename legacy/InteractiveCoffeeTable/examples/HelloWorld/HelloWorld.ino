/*
 * Simple device test
 *
 * Interactive Coffee Table
 * Dan Nixon, dan-nixon.com
 * 19/06/2014
 */

#include <LiquidCrystal.h>
#include <LedControl.h>

#include <InteractiveCoffeeTable.h>

InteractiveCoffeeTable table;

void setup()
{
  uint8_t i, j;
  Serial.begin(9600);
  
  table.setButtonCallback(&button_callback);
  
  table.lcdPrint(LCD_20X4, 0,  0, "Hello, World! 1");
  table.lcdPrint(LCD_20X4, 1,  0, "Hello, World! 2");
  table.lcdPrint(LCD_20X4, 2,  0, "Hello, World! 3");
  table.lcdPrint(LCD_20X4, 3,  0, "Hello, World! 4");
  
  table.lcdPrint(LCD_40X2, 0,  0, "Hello, World! 1");
  table.lcdPrint(LCD_40X2, 1,  0, "Hello, World! 2");
  table.lcdPrint(LCD_40X2, 0, 20, "Hello, World! 3");
  table.lcdPrint(LCD_40X2, 1, 20, "Hello, World! 4");

  table.matrixFullOn();
}

void loop()
{
  table.pollButtons();
}

void button_callback(ICT_Button button, uint8_t state)
{
  Serial.print("Button ");
  Serial.print(button);
  if(state)
    Serial.println(" pressed");
  else
    Serial.println(" released");
    
  if(button == BUTTON_RED)
    table.setLED(LED_ORANGE, state);
    
  if(button == BUTTON_BLUE)
    table.setLED(LED_GREEN, state);
}
