do_DogLcd
======

This is an in progress port to enable hardware SPI use of the Electronic Assembly DOGM displays with the Particle Core and Photon.

6.14.15 version 0.1

6.15.15 version 0.1.5

This code has only been tested using a DOGM162W-A display (16x2 lines, "3V3"*, no backlight). Other variations of the display (1 line, 3 line) have NOT been tested yet. 

6.14.15
It has been tested connected to an Arduino Uno with hardware or software SPI, at 5V. Works fine, pretty much. 

6.15.15
It has been tested connected to a Spark Core with hardware or software SPI at 3V3. Works fine, pretty much. 

The contrast/gain settings were scanned to find values that work well - at 3V3, Gain 3, Contrast 50, and at 5V, Gain 2, Contrast 40 are set as defaults [bug] - but there's a range of values that work. Problem is, if you're off at all, you see nothing or you see black squares. 

V 0.1 Arduino-only code is in /src
V 0.1.5 Spark/Arduino code is in /firmware

The Library is based on and includes much of the code from the DogLcd Library written by Eberhard Fahle (https://github.com/wayoda/DogLcd) with adaptations from the dogm_7036 Library provided by ELECTRONIC ASSEMBLY  (http://www.lcd-module.com/support/application-note/arduino-meets-ea-dog.html) and the SparkCore-LiquidCrystalSPI Library written by BDub (https://github.com/technobly/SparkCore-LiquidCrystalSPI)

Changes include -
* modifications to allow hardware SPI
* addition of setGain to allow software setting of the LCD amplification ratio (works with Contrast to determine whether you see anything on the display, or see 'black boxes').
* user inputs contrast and gain (if desired) over integer range 0-63 and 0-7, respectively. 
* restructuring of the initialization code to make hardware and  voltage dependent parameter settings more explicit.
* flag to prevent hardware reset if user has added new characters (deleted during the reset, otherwise).
* heavily commented due to being a library/hardware n00b.
* added #if defined(SPARK) and #if defined(ARDUINO) statements to allow the library to work with both platforms. seems to behave as expected. 

There are some issues with this Library arising from my attempt to test simultaneously using Arduino and Particle Dev environments. This is to be cleaned up. 

http://jaldilabs.org


