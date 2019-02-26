// Using an interrupt for the stepper motot, now bipolar (h bridge driven)
// using the timer registers for PWM out put

#include <TimerThree.h>

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

// Frequency modes for TIMER1
#define PWM62k   1   //62500 Hz


// Direct PWM change variables
#define PWM9   OCR1A

#define DUTY2PWM(x)  ((255*(x))/100)



const int led = LED_BUILTIN;  // the pin with a LED

void setup(void)
{
  pinMode(led, OUTPUT);
  Timer3.initialize(150000);
  Timer3.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds

  pwm91011configure(PWM62k);

  
  Serial.begin(9600);
}


// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

void blinkLED(void)
{
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(led, ledState);
}


// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void)
{
  unsigned long blinkCopy;  // holds a copy of the blinkCount

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
  noInterrupts();
  blinkCopy = blinkCount;
  interrupts();

  Serial.print("blinkCount = ");
  Serial.println(blinkCopy);
  pwmSet9(DUTY2PWM(50));  

  delay(100);
}

void pwm91011configure(int mode)
{
// TCCR1A configuration
//  00 : Channel A disabled D9
//  00 : Channel B disabled D10
//  00 : Channel C disabled D11
//  01 : Fast PWM 8 bit
TCCR1A=1;

// TCCR1B configuration
// Clock mode and Fast PWM 8 bit
TCCR1B=mode|0x08;  

// TCCR1C configuration
TCCR1C=0;
}

// Set PWM to D9
// Argument is PWM between 0 and 255
void pwmSet9(int value)
{
OCR1A=value;   // Set PWM value
DDRB|=1<<5;    // Set Output Mode B5
TCCR1A|=0x80;  // Activate channel
}
