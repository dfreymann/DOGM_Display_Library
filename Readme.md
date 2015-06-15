do_DogLcd
======

This is a port enabling use of the Electronic Assembly DOGM Character LCD displays using hardware SPI with the Particle Core and Photon. The code also works with Arduino. 

6.14.15 version 0.1

6.15.15 version 0.1.8

This code has only been tested using a DOGM162W-A display (black on white, 16x2 lines, no backlight). Other variations of the display (1 line, 3 line) have NOT been tested. 

6.15.15
The Library has been tested using a Spark Core with hardware or software SPI at 3V3, and tested using an Arduino Uno with hardware or software SPI at 5V. In both cases, it works fine, pretty much. There's a HelloWorld example program that demonstrates various behaviors.

The contrast/gain settings were scanned to find values that work well - at 3V3, the display is VERY sensitive to the Gain and Contrast - too low and you see nothing, too high and you see black boxes. Good values for 3V3 in my hands are Gain 3, Contrast 50.  At 5V, the display seems much more robust to the settings, but a good choice seems to be Gain 2, Contrast 40. These values are used as defaults. 

V 0.1.8 Spark/Arduino code is in /firmware. I'm trying to figure out how to set this up for easiest access for Arduino vs Particle library management. Arduino specific stuff is in /Arduino.  This is to be cleaned up. 

The Library is based on and includes much of the code from the DogLcd Library written by Eberhard Fahle (https://github.com/wayoda/DogLcd) with adaptations from the dogm_7036 Library provided by ELECTRONIC ASSEMBLY  (http://www.lcd-module.com/support/application-note/arduino-meets-ea-dog.html) and the SparkCore-LiquidCrystalSPI Library written by BDub (https://github.com/technobly/SparkCore-LiquidCrystalSPI)

Changes include -
* modifications to allow hardware SPI
* addition of setGain to allow software setting of the LCD amplification ratio (works with Contrast to determine whether you see anything on the display, or see 'black boxes').
* user inputs contrast and gain (if desired) over integer range 0-63 and 0-7, respectively. 
* restructuring of the initialization code to make hardware and voltage dependent parameter settings more explicit.
* flag to prevent hardware reset if user has added new characters (deleted during the reset, otherwise).
* added #if defined(SPARK) and #if defined(ARDUINO) statements to allow the library to work with both platforms. seems to behave as expected. 
* heavily commented due to being a library/hardware n00b.

EA DOGM documentation is available here: http://www.lcd-module.de/fileadmin/eng/pdf/doma/dog-me.pdf

The display controller documentation is available here: http://www.lcd-module.de/eng/pdf/zubehoer/st7036.pdf

http://jaldilabs.org


