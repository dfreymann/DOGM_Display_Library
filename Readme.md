do_DogLcd
======

This is an in progress port to enable hardware SPI use of the Electronic Assembly DOGM displays with the Particle Core and Photon.

6.14.15 version 0.5 

At this point this code has only been tested using a DOGM162W-A display (16x2 lines, 3V3, no backlight) connected to an Arduino Uno with hardware or software SPI. Works fine. 
Other variations of the display (1 line, 3 line), and 5V operation have NOT been tested yet. 

* NOT yet converted for or tested to run on the Spark Core/Photon! 

The Library is based on and includes much of the code from the DogLCD Library written by Eberhard Fahle (https://github.com/wayoda/DogLcd) with adaptations from the dogm_7036 Library provided by ELECTRONIC ASSEMBLY  (http://www.lcd-module.com/support/application-note/arduino-meets-ea-dog.html) and the SparkCore-LiquidCrystalSPI Library written by BDub (https://github.com/technobly/SparkCore-LiquidCrystalSPI)

Changes include -
> modifications to allow hardware SPI
> addition of setGain to allow software setting of the LCD amplification ratio (works with Contrast to determine whether you see anything on the display, or see 'black boxes'.
> user inputs contrast and gain (if desired) over integer range 1-64 and 1-8
> restructuring of the initialization code to make hardware and  voltage dependent parameter settings more explicit.
> flag to prevent hardware reset if user has added new characters (deleted during the reset, otherwise).
> heavily commented due to being a library/hardware n00b.

http://jaldilabs.org


