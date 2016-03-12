/*! -----------------------------------------------------------------------
 * Mouse exerise wheel counter
 * Copyright [2016] Andy Dachs
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * ------------------------------------------------------------------------
 * 
 * We want to see how exercise our mouse is doing so want to count the number of revolutions
 * on his mouse wheel.  We don't care which way the wheel is turning or how fast.
 * In principle this is quite a simple revolution counter.
 * 
 * A magnet is glued under the wheel and every time it passes over a hall effect sensor (A) it
 * generates a pulse which we can count by connecting it on of the external interrupt pins on the 
 * Arduino board.  This simple arrangement generates a lot of noisy triggers when the wheel rests 
 * with the magnet just over the sensor.  To avoid them, a second magnet is glued 180 degrees 
 * further round the wheel.  A second hall effect sensor (B) is placed at 90 degrees.  Each time 
 * a magnet passes a sensor we switch to looking at the other sensor.  Every 1/4 revolution the 
 * wheel generates a signal from one of the sensors and every half turn passes sensor A and 
 * increments the count.  We are actually counting half revolutions so the result is scaled by 
 * two before display.
 * 
 * The display is the AdaFruit OLED 128x32 I2C module
 * 
 * Requires the Adafruit_GFX and Adafruit_SSD1306. Just install them using the library manager.
 * 
 * Author: Andy Dachs
 * Date:   5th March 2016
 * 
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>
#include <PinChangeInterrupt.h>


#define ENCODER_A  2          // hall effect sensor A
#define ENCODER_B  3          // quadrature hall effect sensor B
#define OLED_RESET 4          // reset pin for display module
#define WAKE_UP_SWITCH 8    // switch to wake up display

#define MAX_COUNT (unsigned long)10000000

#define SERIAL_OUTPUT 0

#define DISPLAY_TIME_OUT  10000    // display timeout in ms

/*! These variables must be declared volatile because they
 * are modifed in the interrupt handlers. If the main loop
 * needs to access them, interrupts must be disabled.
 */
volatile unsigned int encoder0Pos = 0;
volatile unsigned long revCount;
volatile boolean expectA;
volatile boolean displayIsActive = true;


Adafruit_SSD1306 display(OLED_RESET);

void doEncoderA(void);
void doEncoderB(void);
void wakeUp(void );

void setup() 
{
  pinMode(ENCODER_A, INPUT); 
  pinMode(ENCODER_B, INPUT); 

  pinMode( WAKE_UP_SWITCH, INPUT_PULLUP );
  digitalWrite( WAKE_UP_SWITCH, HIGH );
  
  expectA = true;
  revCount = 0;

  /* Initialise OLED */
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();                     // Clear the buffer.
  display.dim(true);
    
  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), doEncoderA, FALLING);

  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), doEncoderB, FALLING);

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(WAKE_UP_SWITCH), wakeUp, FALLING );

  disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(WAKE_UP_SWITCH));

  // serial output
#if SERIAL_OUTPUT
  Serial.begin (115200);
#endif
}


void loop()
{
  unsigned long count;
  unsigned long newRevs;
  uint8_t oldSFR;
  static unsigned long oldRevs = -1;
  static unsigned long displayOnTime = 0;

  /* disable interrupts so we can safely read and modify variables */
  oldSFR = SREG;
  cli();
  count = revCount;
  if( count >= (2*MAX_COUNT) )
    revCount = 0;
  SREG = oldSFR;

  /* we count half revolutions */
  newRevs = count/2;

  /* if there is a change, update display */
  if ((oldRevs != newRevs )  && displayIsActive )
  {
    oldRevs = newRevs;
    
    display.clearDisplay();
    display.setCursor(1,5);
    display.setTextSize(3);
    display.setTextColor(WHITE);

    /* display count with leading zeros */
    unsigned long position = MAX_COUNT/10;
    do
    {
      unsigned int digit = (newRevs / position) % 10;
      display.print( digit );
      position /= 10;
#if SERIAL_OUTPUT
      Serial.print( digit );
#endif
    } while( position > 0 );
    
    display.display();    
#if SERIAL_OUTPUT
    Serial.println();
#endif
  }


  if( (millis() - displayOnTime > DISPLAY_TIME_OUT ) && displayIsActive )
  {
    oldSFR = SREG;
    cli();
    displayIsActive = false;
    SREG = oldSFR;
    
    display.ssd1306_command( SSD1306_DISPLAYOFF );
  }

  if( !displayIsActive )
  {
    // enable pin change interrupt
    sleep_enable();  
    enablePinChangeInterrupt(digitalPinToPinChangeInterrupt(WAKE_UP_SWITCH));
    set_sleep_mode( SLEEP_MODE_PWR_DOWN );
    sleep_mode();
  
    // execute from here on wakeup
    sleep_disable();
  
  
    if( displayIsActive )
    {
      disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(WAKE_UP_SWITCH));
      displayOnTime = millis();
      display.ssd1306_command( SSD1306_DISPLAYON );
    }
  }
}

/*! 
 * Interrupt on A changing state
 */
void doEncoderA(){
  /* ignore any interrupts we're not expecting */
  if( expectA )
  {
    expectA = false;
    revCount++;
  }
  sleep_disable();
}

/*! 
 * Interrupt on B changing state
 */
void doEncoderB(){
  if( !expectA )
  {
    expectA = true;
  }
  sleep_disable();
}

void wakeUp(void)
{
  sleep_disable();
//  detachPCINT(WAKE_UP_SWITCH);
//  disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(WAKE_UP_SWITCH));
  displayIsActive = true;
}

