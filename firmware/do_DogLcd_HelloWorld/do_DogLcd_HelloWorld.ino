/*
  do_DogLCD library - Hello World

  Particle Core port

  This exercises a number of the features of the display.
  The function at the end can be called to test the full
  range of Gain/Contrast settings for the display (note
  that it will be 'blank' or 'black' for much of the test)
*/

#include "do_DogLcd.h"

#if defined(SPARK)
#include <application.h>
#elif defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#include <SPI.h>
#elif defined(ARDUINO)
#include <WProgram.h>
#include <SPI.h>
#endif

// Initialize the library with the numbers of the interface pins
#if defined (ARDUINO)
// ARDUINO MOSI, SCK, CSB, RS, RESET, BACKLIGHT
// DogLcdhw lcd(11, 13, 10, 9, 4, -1); // ARDUINO test configuration
// Use 0, 0 for MOSI and SCK to initialize hardware SPI
DogLcdhw lcd(0, 0, 10, 9, 4, -1); // ARDUINO test configuration
#elif defined (SPARK)
// SPARK MOSI, SCK, CSB, RS, RESET, BACKLIGHT
// DogLcdhw lcd(15, 13, 12, 11, 10, -1); // SPARK test configuration
// Use 0, 0 for MOSI and SCK to initialize hardware SPI
DogLcdhw lcd(0, 0, 12, 11, 10, -1); // SPARK test configuration
#endif

//from dogm163 library
byte arrow_down[] = {0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0X04, 0X00};

void setup() {
  // set up the LCD type and the contrast setting for the display
  // what you see on the display depends on both contrast and gain settings
  // if 0 are entered for contrast or gain, suitable defaults will be set.
  // Good INPUT values for contrast and gain are 40 and 2 (5V system).
  // and 50 and 3 (3V3 system).
  // [BUG] Enter -1 for contrast/gain to set defaults. [BUG]
#if defined (ARDUINO)
  lcd.begin(DOG_LCDhw_M162, DOG_LCDhw_VCC_5V, 40, 2);  // ARDUINO test configuration
#elif defined (SPARK)
  lcd.begin(DOG_LCDhw_M162, DOG_LCDhw_VCC_3V3, 50, 3);  // SPARK test configuration
#endif

  // Print a message to the LCD.
  lcd.print("hello, spark!");

  // note - the overloaded lcd.write() statement reports an error
  // "call of overloaded 'write(int, int)' is ambiguous" if this is '0'. however,
  // if the value is cast as (byte) in lcd.write(), it will work
  // - see http://forum.arduino.cc/index.php?topic=174883.msg1299547#msg1299547
  lcd.createChar(0, arrow_down);

}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
  delay(2000);
  // play with contrast - by hand - if too low
  // for the gain, you'll see nothing
  //lcd.setContrast(36);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("same contrast");
  delay(2000);
  lcd.clear();
  //lcd.setContrast(50);
  lcd.setCursor(0,0);
  lcd.print("no blinking");
  lcd.noBlink();
  delay(2000);
  // note that we don't have access to the contrast
  // or gain used in the begin() statement. reset()
  // gets them for us, however.
  lcd.reset();
  lcd.setCursor(0,1);
  lcd.print("no cursor");
  lcd.noCursor();
  delay(2000);
  lcd.clear();
  lcd.blink();
  lcd.setCursor(0,0);
  lcd.print("ELECTRONIC");
  lcd.setCursor(8,1);
  lcd.print("ASSEMBLY");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("My new char?");
  lcd.setCursor(1,1);
  lcd.ascii(0);
  lcd.setCursor(3,1);
  lcd.write((byte)0);
  lcd.setCursor(5,1);
  lcd.write("A");
  delay(1000);
  lcd.scrollDisplayRight();
  delay(1000);
  lcd.scrollDisplayLeft();

  delay(2000);
  lcd.reset();
  lcd.print("spark, again!");
  delay(1000);

  // comment out if you don't want to run through this
  //a_little_function_to_check_gain_contrast();

}

void a_little_function_to_check_gain_contrast() {
  // cycles through the full range of gain and contrast
  // (omitting gain = 0 and 8, which are unusable) in order to
  // demonstrate relationship between contrast and gain
  for (int i=1; i<7; i++){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.setGain(4);
    lcd.setContrast(50);
    lcd.print("Gain");
    lcd.setCursor(6,0);
    lcd.print(i);
    lcd.setGain(i);
    for (int j=0; j<64; j++){
       lcd.setContrast(j);
       lcd.setCursor(0,1);
       lcd.print("Contr");
       lcd.setCursor(8,1);
       lcd.print(j);
       delay(100);
    }
  }
  lcd.clear();
  lcd.setGain(4);
  lcd.setContrast(49);
}
