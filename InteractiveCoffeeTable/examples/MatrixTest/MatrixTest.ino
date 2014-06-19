/*
 * LED matrix test
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
  Serial.begin(9600);
}

void loop()
{
  uint8_t x, y;

  for(x = 0; x < 16; x++)
  {
    for(y = 0; y < 16; y++)
    {
      Serial.print(x);
      Serial.print("\t\t");
      Serial.println(y);
      
      table.matrixSetPixel(x, y, 1);
      delay(50);
    }
  }

  table.matrixClear();
}
