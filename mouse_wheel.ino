/*! 
 * Mouse exerise wheel counter
 * ---------------------------
 * 
 * We want to see how excerise our mouse is doing so want to count the number of revoltions
 * on his mouse wheel.  We don;t care which way the wheel is turning or how fast.
 * In principle this is quite a simple revolution counter.
 * 
 * A magnet is glued under the wheel and every time it passes over a hall effect sensor (A) it
 * generates a pulse which we can count by connecting it on of the external interrupt pins on the 
 * Arduino board.  This simple arrangement generates a lot of noisy triggers when the wheel rests 
 * with the magnet just over the sensor.  To avoid them, a second magnet is glued 180 degrees 
 * further round the wheel.  A second hall effect sensor (B) is placed at 90 degrees.  Each time 
 * a magnet passes a sensor we switch to looking at the other sensor.  Every 1/4 revolution the 
 * wheel generates a signel from one of the sensors and every half turn passes sensor A and 
 * increments the count.  We are actually counting half revolutions so the result is scaled by 
 * two before display.
 * 
 * In terms of assembly, the wheel is made of special mouse safe plastic but the rest of the wires 
 * and electronics need to be safe.  The simplest way to get a circular PCB right up close to 
 * the wheel was to canibalise the encoder board from a small motor (dx.com sku:418535). The rest of 
 * the electronics will be housed in an aluminium enclosure and battery powered for safety. The cables 
 * running between the sensors and enclosre are sheathed in steel braid.
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

#define ENCODER_A  2          // hall effect sensor A
#define ENCODER_B  3          // quadrature hall effect sensor B
#define OLED_RESET 4          // reset pin for display module

#define MAX_COUNT (unsigned long)10000000


/*! These variables must be declared volatile because they
 * are modifed in the interrupt handlers. If the main loop
 * needs to access them, interrupts must be disabled.
 */
volatile unsigned int encoder0Pos = 0;
volatile unsigned long revCount;
volatile boolean expectA;


Adafruit_SSD1306 display(OLED_RESET);

void setup() 
{
  pinMode(ENCODER_A, INPUT); 
  pinMode(ENCODER_B, INPUT); 

  expectA = true;
  revCount = 0;

  /* Initialise OLED */
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();                     // Clear the buffer.

  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, FALLING);

  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, FALLING);

  // serial output
  Serial.begin (115200);
}


void loop()
{
  unsigned long count;
  unsigned long newRevs;
  uint8_t oldSFR;
  static unsigned long oldRevs = -1;

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
  if (oldRevs != newRevs )
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
      Serial.print( digit );
    } while( position > 0 );
    
    display.display();    
    Serial.println();
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
}

/*! 
 * Interrupt on B changing state
 */
void doEncoderB(){
  if( !expectA )
  {
    expectA = true;
  }
}

