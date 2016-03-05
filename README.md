# mouse-wheel
Arduino based exercise wheel counter for a (real, live, furry mammal) mouse

## Overview
We want to see how much excerise our mouse is doing so want to count the number of revolutions on his mouse wheel.  We don't care which way the wheel is turning or how fast. In principle this is quite a simple revolution counter.

## Design
A magnet is glued under the wheel and every time it passes over a hall effect sensor (A) it generates a pulse which we can count by connecting it on of the external interrupt pins on the Arduino board.  This simple arrangement generates a lot of noisy triggers when the wheel rests with the magnet just over the sensor.  To avoid them, a second magnet is glued 180 degrees further round the wheel and a second hall effect sensor (B) is placed at 90 degrees.  Each time a magnet passes a sensor we switch to looking at the other sensor.  Every 1/4 revolution the wheel generates a signel from one of the sensors and every half turn passes sensor A and increments the count.  We are actually counting half revolutions so the result is scaled by two before display.

![Wheel with magnets](/images/IMG_0185.JPG)

In terms of assembly, the wheel is made of special mouse safe plastic but the rest of the wires and electronics need to be safe too.  The simplest way to get a circular PCB right up close to the wheel was to canibalise the encoder board from a small motor from ([dx](http://www.dx.com/p/high-torque-25mm-dc-12-0v-330rpm-encoder-precision-gear-motor-418535). 

![Encoder circuit](/images/IMG_0182.JPG)

The rest of the electronics will be housed in an aluminium enclosure and battery powered for safety. The cables running between the sensors and enclosre are sheathed in steel braid.

![Electronics](/images/IMG_0183.JPG)

The display is the AdaFruit OLED 128x32 I2C module

## Libraries
Requires the Adafruit_GFX and Adafruit_SSD1306. Just install them using the library manager.
