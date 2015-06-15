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
 * during the reset, otherwise).
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

#include "do_DogLcd.h"

#if defined(SPARK)
#include <application.h>
#elif defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#elif defined(ARDUINO)
#include <WProgram.h>
#endif

#if defined(ARDUINO)
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <SPI.h>
#endif

#if defined(SPARK)
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define theClockDivider SPI_CLOCK_DIV32
#elif defined(ARDUINO)
#define theClockDivider SPI_CLOCK_DIV4
#endif

DogLcdhw::DogLcdhw(int lcdSI, int lcdCLK, int lcdCSB, int lcdRS, int lcdRESET, int backLight) {
    // select Hardware SPI by setting lcdSI == lcdCLK
    if (lcdSI == lcdCLK) {
        _hardware = true;
        /* The SPI hardware pins MOSI (SI), SCK (CLK) and SS (CSB) are defined for the
         * Arduino (Uno) in the library <pins_arduino.h> as pins 11, 13 & 10, respectively,
         * and are defined for the Particle Core in the library <spark_wiring.h> as pins 15,
         * 13, & 12 (i.e. A5, A3 & A2), respectively. So, let's use them.
         */
        this->lcdSI = MOSI;     // Master Out Slave Input, this is sending the bits
        this->lcdCLK = SCK;     // Serial Clock, this is setting the timing of the bits
        this->lcdCSB = SS;      // Slave Select, this is telling the device it's selected
    } else {
        _hardware = false;
        this->lcdSI=lcdSI;
        this->lcdCLK=lcdCLK;
        this->lcdCSB=lcdCSB;
    }
    this->lcdRS=lcdRS;          // Register Select, flags DOG controller to write data to internal RAM.
    this->lcdRESET=lcdRESET;    // Reset, this provides a hardware reset. Software reset is available.
    this->backLight=backLight;
}

int DogLcdhw::begin(int model, int vcc, int contrast, int gain) {

    //init all pins to go HIGH, we dont want to send any commands by accident
    pinMode(this->lcdCSB,OUTPUT);
    digitalWrite(this->lcdCSB,HIGH);
    pinMode(this->lcdSI,OUTPUT);
    digitalWrite(this->lcdSI,HIGH);
    pinMode(this->lcdCLK,OUTPUT);
    digitalWrite(this->lcdCLK,HIGH);

    // if hardware connections, configure SPI
    if (_hardware) {
        /* setBitOrder - MSBFIRST - most significant bit first. Fine.
         *
         * setDataMode - SPI_MODE3 - ok, here's a problem. This "seems" to work "fine" for
         * the Arduino Uno test case using either SPI_MODE0 or SPI_MODE3. For Mode0, CPOL=0
         * and CPHA=0, such that "The data must be available before the first clock signal rising.
         * The clock idle state is zero. The data on MISO and MOSI lines must be stable while the
         * clock is high and can be changed when the clock is low. The data is captured on the
         * clock's low-to-high transition and propagated on high-to-low
         * clock transition." (See http://dlnware.com/theory/SPI-Transfer-Modes).
         * For Mode3, CPOL=1 and CPHA=1, such that "The first clock signal falling can be used to
         * prepare the data.The clock idle state is one. The data on MISO and MOSI lines must be
         * stable while the clock is high and can be changed when the clock is low.The data is
         * captured on the clock's low-to-high transition and propagated on high-to-low clock
         * transition." [See TESTED, below.]
         *
         * setClockDivider - SPI_CLOCK_DIV16 - the SPI_CLOCK_DIV definitions for the Arduino (UNO)
         * are provided in the library <SPI.h> as DIV4 -> DIV128, and for the Particle Core are
         * provided by the library <spark_wiring_spi.h> as DIV2->DIV256. The Arduio UNO clock
         * speed is 16 MHz, which corresponds to a 62.5ns period. DIV16 for the UNO test case
         * means, therefore, the SPI bus is running at 1 MHz, and each byte transfer takes
         * ~8*1uS=8us. The Particle Core clock speed is 72MHz, and if DIV8 is used
         * then the SPI bus is running at 9Mhz. From the ST7036 controller datasheet, the 4 wire
         * SPI interface minimum clock period is 200 nS, which corresponds to 5Mhz. If I'm
         * reading this right, we should be able to use DIV4 on the Arduino (4Mhz), and
         * DIV16 [x NO, tested this. use DIV32] on the Particle Core (4.5Mhz) - to
         * be tested...
         * TESTED: ARDUINO Mode3 set, works DIV2->DIV128,
         *                 Mode0 set, fails with DIV2 and DIV4, works DIV8->DIV128
         * TESTED: SPARK   Mode3 set, fails with DIV16, works with DIV32 and DIV128
         *                 Mode0 set, fails with DIV16, works with DIV32 and DIV128
         */
        SPI.begin();
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE3);
        SPI.setClockDivider(theClockDivider);
    }

    pinMode(this->lcdRS,OUTPUT);
    digitalWrite(this->lcdRS,HIGH);

    if(this->lcdRESET!=-1) {
        pinMode(this->lcdRESET,OUTPUT);
        digitalWrite(this->lcdRESET,HIGH);
    }

    if(this->backLight!=-1) {
        pinMode(this->backLight,OUTPUT);
        digitalWrite(this->backLight,LOW);
    }

    // set all model-specific parameters here
    if(model==DOG_LCDhw_M081) {
        this->model=model;
        // model-dependent parameters
        rows=1;
        cols=8;
        memSize=80;
        startAddress[0]=0;
        startAddress[1]=-1;
        startAddress[2]=-1;
        // dmf - new
        biasAndFx = 0x14;
        //8-bit,1-line
        instructionSetTemplate=(uint8_t)0x30;
    }
    else if(model==DOG_LCDhw_M162) {
        this->model=model;
        // model-dependent parameters
        rows=2;
        cols=16;
        memSize=40;
        startAddress[0]=0;
        startAddress[1]=0x40;
        startAddress[2]=-1;
        // dmf - new
        biasAndFx = 0x14;
        //8-bit,2-line
        instructionSetTemplate=(uint8_t)0x38;
    }
    else if(model==DOG_LCDhw_M163) {
        this->model=model;
        // model-dependent parameters
        rows=3;
        cols=16;
        memSize=16;
        startAddress[0]=0;
        startAddress[1]=0x10;
        startAddress[2]=0x20;
        // dmf - new
        biasAndFx = 0x15;
        //8-bit,3-line
        instructionSetTemplate=(uint8_t)0x38;
    }
    else {
        //unknown or unsupported model
        return -1;
    }

    // and set all voltage-depedendent parameters here
    if (vcc==DOG_LCDhw_VCC_5V) {
        this->vcc=vcc;
        // dmf - new
        boosterMode = 0x00;
        biasAndFx |= 0x08;
        // set default contrast and gain (amplification ratio)
        if (contrast == -1){
            // set a default that seems to work for 5V
            contrast = GOOD_5V_CONTRAST;
        }
        if (gain == -1) {
            // set a default that seems to work for 5V
            gain = GOOD_5V_GAIN;
        }
    } else if (vcc==DOG_LCDhw_VCC_3V3) {
        this->vcc=vcc;
        // dmf - new
        boosterMode = 0x04;
        // set default contrast and gain (amplification ratio)
        if (contrast == -1){
            // set a default that seems to work for 3V3
            contrast = GOOD_3V3_CONTRAST;
        }
        if (gain == -1) {
            // set a default that seems to work for 3V3
            gain = GOOD_3V3_GAIN;
        }
    } else {
        //unknown or unsupported supply voltage
        return -1;
    }

    if(contrast < 0 || contrast> 0x3F) {
        //contrast is outside the valid range
        return -1;
    }
    this->contrast=contrast;
    this->gain=gain;

    // the reset() method does the actual display initialization
    reset();

    // success
    return 0;
}

/* runs (or re-runs) the controller initialization sequence */
void DogLcdhw::reset() {

    // a hardware reset will delete any createChars() so protect created
    // characters by testing before allowing a hard reset
    if(lcdRESET!=-1 && _noCharsAdded) {
        //If user wired the reset line, pull it low and wait for 40 millis
        digitalWrite(lcdRESET,LOW);
        delay(40);
        digitalWrite(lcdRESET,HIGH);
        delay(40);
    }
    else {
        //User wants software reset, we simply wait a bit for stable power
        delay(50);
    }

    /* initialization sequence */
    // set Bias and Fx
    setBiasAndFx();
    // set Contrast
    setContrast(this->contrast);
    // set Gain
    setGain(this->gain);
    // set Display mode to a standard setting: display on, cursor on, no blink
    displayMode=0x04;
    cursorMode=0x02;
    blinkMode=0x00;
    writeDisplayMode();
    // set Entry mode - the command selector for entry mode is 0x04, and
    // text direction methods simply modify that value.
    entryMode=0x04;
    leftToRight();

    // finally, clear the display
    clear();

}

void DogLcdhw::setInstructionSet(uint8_t is) {
    if(is<0 || is>3)
        return;
    uint8_t cmd=instructionSetTemplate | is;
    writeCommand(cmd,30);
}

/* the following commands are all accessible through Instruction Table 1 */
void DogLcdhw::setBiasAndFx() {
    // contrast is in instruction Table 1
    setInstructionSet(1);
    //bias and Fx are model- and voltage- specific
    writeCommand(biasAndFx,30);
}

/* set the contrast - contrast and gain (amplification ratio) are highly correlated */
void DogLcdhw::setContrast(int contrast) {
    if(contrast<0 || contrast>0x3F)
	return;
    // contrast is in instruction Table 1
    setInstructionSet(1);

    // contrast is determined by 6 bits, written as part of two
    // commands, 0x50 and 0x70, under Instruction Table 1.
    // boosterMode (off for 5V, on for 3V3) is written during the
    // same command as the (2-bit) high-nibble of contrast
    writeCommand(0x50 | boosterMode | ((contrast>>4)&0x03), 30);
    // now set the low-nibble of the contrast
    writeCommand((0x70 | (contrast & 0x0F)),30);

}

/* set the amplification ratio (gain) - gain and contrast are highly correlated */
void DogLcdhw::setGain(int gain) {
    if (gain<0 || gain>0x07)
        return;
    // Gain is in instruction Table 1
    setInstructionSet(1);
    // The command selector is 0x60, follower control is set with
    // 0x08, and gain is determined by the three bits, 0x00->0x07
    writeCommand(0x60 | 0x08 | gain,30);

}

/* the following commands are all accessible through Instruction Table 0 */
void DogLcdhw::scrollDisplayLeft(void) {
    setInstructionSet(0);
    writeCommand(0x18,30);
}

void DogLcdhw::scrollDisplayRight(void) {
    setInstructionSet(0);
    writeCommand(0x1C,30);
}

/* Eight character addresses at the start of the CGRAM
 * character matrix are empty and meant to be available to
 * create custom characters as needed.
 */
void DogLcdhw::createChar(int charPos, uint8_t charMap[]) {

    int baseAddress;

    /* charPos selects which of the 8 addresses available
     * to use at the start of the CGRAM table is selected.
     */
    if(charPos<0 || charPos>7)
        return;

    //changing CGRAM address belongs to instruction Table 0
    setInstructionSet(0);

    /* Tell the controller the CGRAM address to write to.
     * The 0x40 command in Table 0 is the instruction for setting
     * the CGRAM address. Note the CGRAM address will autoincrement
     * as we write to it.
     */
    baseAddress=charPos*8;
    writeCommand((0x40|(baseAddress)),30);

    /* We are now writing the bits of the character matrix
     * Important - because of the previous CGRAM address command
     * the controller knows to write these bits to CGRAM, not DDRAM
     */
    for (int i=0; i<8; i++) {
        writeChar(charMap[i]);
    }

    /* The following simply sets the cursor position, but that's
     * done by setting the DDRAM address, so it also serves to tell
     * the controller that future data writes are to the display DDRAM,
     * not CGRAM. The 0x80 command used in setCursor is in the default
     * instruction Table, so no setInstructionSet required.
     */
    setCursor(0,0);

    /* set flag to prevent hard reset (which will delete our new chars)
     */
    _noCharsAdded = false;

}

/* the following commands are all accessible through the default Instruction Table */
void DogLcdhw::clear() {
    writeCommand(0x01,1080);
}

void DogLcdhw::home() {
    writeCommand(0x02,1080);
}

void DogLcdhw::setCursor(int col, int row) {
    if(col>=memSize || row>=rows) {
	//not a valid cursor position
	return;
    }
    int address=(startAddress[row]+col) & 0x7F;
    writeCommand(0x80|address,30);
}

void DogLcdhw::noDisplay() {
    displayMode=0x00;
    writeDisplayMode();
}

void DogLcdhw::display() {
    displayMode=0x04;
    writeDisplayMode();
}

void DogLcdhw::noCursor() {
    cursorMode=0x00;
    writeDisplayMode();
}

void DogLcdhw::cursor() {
    cursorMode=0x02;
    writeDisplayMode();
}

void DogLcdhw::noBlink() {
    blinkMode=0x00;
    writeDisplayMode();
}

void DogLcdhw::blink() {
    blinkMode=0x01;
    writeDisplayMode();
}

void DogLcdhw::writeDisplayMode() {
    writeCommand((0x08 | displayMode | cursorMode | blinkMode),30);
}

void DogLcdhw::leftToRight(void) {
    entryMode|=0x02;
    writeCommand(entryMode,30);
}

void DogLcdhw::rightToLeft(void) {
    entryMode&=~0x02;
    writeCommand(entryMode,30);
}

void DogLcdhw::autoscroll(void) {
    entryMode|=0x01;
    writeCommand(entryMode,30);
}

void DogLcdhw::noAutoscroll(void) {
    entryMode&=~0x01;
    writeCommand(entryMode,30);
}

void DogLcdhw::setBacklight(int value, bool usePWM) {
    if(backLight!=-1 && value>=0) {
	if(!usePWM) {
	    if(value==LOW) {
		digitalWrite(backLight,LOW);
	    }
	    else {
		digitalWrite(backLight,HIGH);
	    }
	}
	else {
	    if(value>255)
		value=255;
	    analogWrite(backLight,value);
	}
    }
}

void DogLcdhw::ascii (char character) {
    writeChar(character);
}

void DogLcdhw::writeChar(uint8_t value) {
    /* Setting RS HIGH tells the controller we're
     * sending data, not a sending a command. Data
     * is written to the register address (CGRAM, or DDRAM)
     * that was last set
     */
    digitalWrite(lcdRS,HIGH);
    spiTransfer(value,30);
}

void DogLcdhw::writeCommand(uint8_t value,int executionTime) {
    /* Setting RS LOW tells the controller we're sending
     * a command, not writing data
     */
    digitalWrite(lcdRS,LOW);
    spiTransfer(value,executionTime);
}

void DogLcdhw::spiTransfer(uint8_t value, int executionTime) {

    digitalWrite(lcdCSB,LOW);

    if (_hardware){
        // Let hardware SPI handle it
        SPI.transfer(value);
    } else {
        // Otherwise, let the software bit-bang it
        digitalWrite(lcdCLK,HIGH);
        for(int i=7;i>=0;i--) {
            if(bitRead(value,i)) {
                digitalWrite(lcdSI,HIGH);
            }
            else {
                digitalWrite(lcdSI,LOW);
            }
            digitalWrite(lcdCLK,LOW);
            digitalWrite(lcdCLK,HIGH);
        }
    }

    digitalWrite(lcdCSB,HIGH);
    delayMicroseconds(executionTime);
}
