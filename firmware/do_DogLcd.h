/* dmf 6.14.15
 * do_DogLCD - an in progress port of the DogLCD library
 * to enable hardware SPI use of the Electronic Assembly
 * DOGM displays with the Particle Core and Photon.
 *
 * Particle Core port
 *
 * Version 0.1.8
 * This code has only been tested using a
 * DOGM162W-A display (16x2 lines, 3V3, no backlight)
 * connected either to an Arduino Uno (5V) with hardware or software SPI, or
 * to a Spark Core (3V3) with hardware or software SPI. Works fine.
 *
 * The Library is based on and includes much of the code from the DogLCD
 * Library written by Eberhard Fahle (https://github.com/wayoda/DogLcd) with
 * adaptations from the dogm_7036 Library provided by ELECTRONIC ASSEMBLY
 * (http://www.lcd-module.com/support/application-note/arduino-meets-ea-dog.html)
 * and the SparkCore-LiquidCrystalSPI Library written by BDub
 * (https://github.com/technobly/SparkCore-LiquidCrystalSPI)
 *
 * Changes include -
 * > modifications to allow hardware SPI
 * > addition of setGain to allow software setting of the
 * LCD amplification ratio (works with Contrast to determine
 * whether you see anything on the display, or see 'black boxes'.
 * > user inputs contrast and gain (if desired) over integer range 1-64 and 1-8
 * > restructuring of the initialization code to make hardware and
 * voltage dependent parameter settings more explicit.
 * > flag to prevent hardware reset if user has added new characters (deleted
 * during hardware reset, otherwise).
 * > heavily commented due to being a library/hardware n00b.
 *
 */
/*
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * do_DogLcd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with do_DogLcd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2015 Douglas Freymann <jaldilabs@gmail.com>
 */

#ifndef do_DOG_LCD_h
#define do_DOG_LCD_h

#if defined(SPARK)
#include <application.h>
#elif defined(ARDUINO)
#include <inttypes.h>
#include "Print.h"
#endif

/** Define the available models */
#define DOG_LCDhw_M081 1
#define DOG_LCDhw_M162 2
#define DOG_LCDhw_M163 3

/** define the supply voltage for the display */
#define DOG_LCDhw_VCC_5V 0
#define DOG_LCDhw_VCC_3V3 1

/** define 'good' contrast and gain values for defaults */
#define GOOD_5V_GAIN 2
#define GOOD_5V_CONTRAST 40
#define GOOD_3V3_GAIN 3
#define GOOD_3V3_CONTRAST 50

/**
 * A class for Dog text LCD's using the
 * SPI-feature of the controller.
 */
class DogLcdhw : public Print {
 private:
    /** The model-type of the display connected */
    int model;
    /** The number of lines on the display */
    int rows;
    /** The number of vivible columns on the display */
    int cols;
    /** The size of the character memory on each row */
    int memSize;
    /** The starting address of each row */
    int startAddress[3];

    /** The (arduino-)pin used for the serial data */
    int lcdSI;
    /** The (arduino-)pin used for the serial clock */
    int lcdCLK;
    /**
     * The (arduino-)pin that toggles between sending a command
     * or character data to the display
     */
    int lcdRS;
    /** The (arduino-)pin used selecting the display */
    int lcdCSB;
    /** The (arduino-)pin used for resetting the dislay */
    int lcdRESET;
    /** The (arduino-)pin used for switching the backlight */
    int backLight;
    /**
     * The supply voltage used for the display. This is
     * one of the constants <code>DOG_LCD_VCC_5V</code>
     * or <code>DOG_LCD_VCC_3V3</code>.
     */
    int vcc;
    /** The contrast setting for the display */
    int contrast;
    /** The gain (amplification ratio) for the display */
    int gain;

    /** Model and voltage-dependent parameters for configuration */
    uint8_t biasAndFx;
    uint8_t boosterMode;

    /** the mode the cursor is in */
    uint8_t cursorMode;
    /** the mode the display is in */
    uint8_t displayMode;
    /** the blink setting for the cursor */
    uint8_t blinkMode;
    /** the entrymode currently used */
    uint8_t entryMode;

    /** The template for changing the instruction set */
    uint8_t instructionSetTemplate;

    /** Flag the hardware SPI selection
     *  Selection is made by setting the values of the
     *  lcdSI and lcdCLK pins equal
     */
    bool _hardware;
    /** Take care of the user-added characters - a hard reset will
     * delete them, so set a flag so that only a soft reset occurs
     * when working with new symbols.
     */
    bool _noCharsAdded=true;

    /** for hardware SPI
     * _clockDivider - convert system clock speed for SPI bus. Arduino Uno system clock
     * at 16Mhz, Particle Core is at 72Mhz. Maximum serial clock period for ST7036 controller
     * is 5Mhz (see notes).
     *
     * _dataMode - For Mode0, CPOL=0 and CPHA=0, such that "The data must be available before the
     * first clock signal rising. The clock idle state is zero. The data on MISO and MOSI lines
     * must be stable while the clock is high and can be changed when the clock is low. The data is
     * captured on the clock's low-to-high transition and propagated on high-to-low
     * clock transition." (See http://dlnware.com/theory/SPI-Transfer-Modes). For Mode3, CPOL=1 and
     * CPHA=1. Both seem to work, so far (Arduino Uno).
     *
     * _bitOrder - MSBFIRST, most significant bit first
     */
    uint8_t _clockDivider;
    uint8_t _dataMode;
    uint8_t _bitOrder;

 public:
    /**
     * Creates a new instance of DogLcd and asigns the (arduino-)pins
     * used to control the display.
     * @param lcdSI The (arduino-)pin connected to the SI-pin on the display
     * @param lcdCLK The (arduino-)pin connected to the CLK-pin on the display
     * @param lcdCSB The (arduino-)pin connected to the CSB-pin on the display
     *        [note: order of pins changed from DogLcd source]
     * @param lcdRS The (arduino-)pin connected to the RS-pin on the display
     * @param backLight If you hardware supports switching the backlight
     * on the display from software this is the (arduino-)pin to be used.
     * @param lcdRESET If you want your code to reset the display from
     * software this is the (arduino-)pin where the RESET-pin of
     * the display is connected. If you don't need this feature simply
     * connect the RESET-pin on the display to VCC.
     */
    DogLcdhw(int lcdSI, int lcdCLK, int lcdCSB, int lcdRS,
	   int lcdRESET=-1, int backLight=-1);

    /**
     * Resets and initializes the Display.
     * @param model the type of display that is connected.
     * This must be one of the constants <code>DOG_LCD_M081</code>,
     * <code>OG_LCD_M162</code> or <code>DOG_LCD_M163</code>
     * defined in this class.
     * @param vcc the supply voltage on which the display runs. This must
     * one of the constants <code>DOG_LCD_VCC_5V</code>
     * or <code>DOG_LCD_VCC_3V3</code>
     *        [note: order of parameters changed from DogLcd source]
     * @param contrast the contrast setting for the display. User entered
     * values between 1 and 64 are allowed. If no value entered, a (voltage
     * dependent) default (41 for 3V3) will be set.
     * @param gain the amplification ratio for the display. User entered
     * values between 1 and 8 are allowed. If no value entered, a (voltage
     * dependent) default (3 for 3V3) will be set.
     * Contrast and Gain are HIGHLY CORRELATED - if either are too low,
     * the display will be blank, if either are too high the display will
     * show 'black squares'.
     * @return 0 if the display was sucessfully initialized,
     * -1 otherwise.
     */
    int begin(int model, int vcc=DOG_LCDhw_VCC_3V3, int contrast=0, int gain=0);

    /**
     * Reset the display.
     */
    void reset();

    /**
     * Set the contrast for the display.
     * @param contrast the contrast to be used for the display. Setting
     * the contrast to a value < 32 will probably give the impression
     * that the display isn't working because nothing is printed.
     * Display behavior can be very sensitive to contrast and to gain.
     * A reasonable default is 0x20, although others have found 0x28
     * is a good starting point - this may depend on device configuration.
     * The valid range for the contrast value is 0.63 (6 bits) or
     * (0x00..0x3F hex). If the value is outside the valid range the
     * method does nothing.
     */
    void setContrast(int contrast);

    /**
     * dmf - Set the amplification ratio for the display.
     * @param gain the amplification ratio to be used for the display.
     * This, with contrast, determines what you see on the display and
     * the optimal window can be quite small and device dependent. Too
     * high and you get 'black squares', too low and you see nothing at
     * all. The valid range for gain is 0.7 (3 bits) or (0x00..0x07 hex)
     * which yields gain ratios 1.0, 1.25, 1.5, 1.8, 2.0, 2.5, 3, 3.75.
     * If the value is outside the valid range the method does nothing.
     */
    void setGain(int gain);

    /**
     * Clears the display and moves the cursor back to
     * the start of the first line.
     */
    void clear();

    /**
     * Moves the cursor back to the start of the first line
     * without clearing the display.
     */
    void home();

    /**
     * Switches the display off
     */
    void noDisplay();

    /**
     * Switches the display on
     */
    void display();

    /**
     * Disables blinking of the cursor
     */
    void noBlink();

    /**
     *Enables blinking of the cursor
     */
    void blink();

    /**
     * Disables the (underline-)cursor
     */
    void noCursor();

    /**
     * Enables the (underline-)cursor
     */
    void cursor();

    /**
     * Scroll the contents of the display to the left
     */
    void scrollDisplayLeft();

    /**
     * Scroll the contents of the display to the right
     */
    void scrollDisplayRight();

    /**
     * Calling this method will change the way new characters
     * are printed at the current cursor-position.
     * The character is printed and then the cursor is advanced to the
     * column on the right of the character.
     * This is "sort of" the standard editor behaviour we are
     * used to with western european languages.
     * It is also the standard behaviuor when the display is reset.
     */
    void leftToRight();

    /**
     * Calling this method will change the way new characters
     * are printed at the current cursor-position.
     * The character is printed and then the cursor is advanced to the
     * column on the left of the character.
     */
    void rightToLeft();

    void autoscroll();

    void noAutoscroll();

    /**
     * Set one of the eight user-defineable chars
     * @param charCode the code of the char you want to define.
     * Values from 0..7 are allowed here
     * @param charMap an array of 8 bytes that contains the char
     * definition.
     */
    void createChar(int charCode, uint8_t charMap[]);

    /**
     * Set the cursor to a new loaction.
     * @param col the column to move the cursor to
     * @param row the row to move the cursor to
     * If the column- or row-index does exceed
     * the number of columns/rows on the hardware
     * the cursor stays where it is.
     */
    void setCursor(int col, int row);

    /** dmf - issues with the overloaded print() [below]
     *  this from dogm_7036.h
     */
    void ascii (char character);

    /*
       Using namespace Print::write makes it possible to
       to send data to the Lcd via the lcd.write(const char *) or
       a lcd.write(const uint8_t *,int) methods.
    */
    using Print::write;


#if defined (SPARK) || (defined(ARDUINO) && ARDUINO >= 100)
    //The Print::write() signature was changed with Arduino versions >= 1.0

    /**
     * Implements the write()-method from the base-class that
     * is called whenever a character is to be printed to the
     * display.
     * @param c the character to be printed.
     * @return int number of characters written
     */
     virtual size_t write(uint8_t c) { writeChar(c); return 1; }

#elif defined(ARDUINO)
    //This keeps the library compatible with pre-1.0 versions of the Arduino core
    virtual void write(uint8_t c) { writeChar(c); }

#endif

    /**
     * Set the backlight. This is obviously only possible
     * if you have build a small circuit for switching/dimming the
     * backlight of the display.
     * @param value the new value for the backlight. If the
     * backlight driver circuit is connected to a normal digital IO-pin
     * on the arduino using value=LOW or value=HIGH will switch
     * off/on as expected.
     * @param usePWM set this to true if the driver is connected to
     * a PWM pin on the arduino you can set any dim the backlight
     * with values from 0..255
     */
    void setBacklight(int value,bool PWM=false);

 private:
    /**
     * Set the intruction set to use for the next command
     * @param is the index of the instructionSet to use
     */
    void setInstructionSet(uint8_t is);

    /* set Bias and Fx during initialization
     */
    void setBiasAndFx();

    /**
     * Call the displaymode function when cursor settings
     * have been changed
     */
    void writeDisplayMode();

    /**
     * Send a command to the display
     * @param cmd the command to send.
     * @param executionTime the hardware needs some time to
     * execute the instruction just send. This is the time in
     * microseconds the code should wait after trandd´sfrerring the data
     */
    void writeCommand(uint8_t cmd, int executionTime);

    /**
     * Send a character to the display
     * @param c the character to send.
     */
    void writeChar(uint8_t c);

    /**
     * Implements the low-level transfer of the data
     * to the hardware.
     * @param executionTime the hardware needs some time to
     * execute the instruction just send. This is the time in
     * microseconds the code should wait after trandd´sfrerring the data
     */
    void spiTransfer(uint8_t c, int executionTime);
};

#endif
