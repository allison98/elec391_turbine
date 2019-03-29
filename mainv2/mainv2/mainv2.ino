// Using an interrupt for the stepper motot, now bipolar (h bridge driven)
// using the timer registers for PWM out put
// synchronous update every __ seconds

// timer 4 PWM trigger
// timer 1 altserial
// timer 3 interrupt 

// -------------  //
// AR PIN SETUP  //
// ------------- //

/*
Stepper motor 4 pins on the L298 left to right Pins 2,3,4,7
Pin 5 is bluetooth RXD
Pin 6 is PWM 

PIN 0 and 1 (mostly used for uploading will keep free for now)

PINS 6 or 13 cannot use analogWrite

PIN 10, 11,12 for generator stator pins

not used pins
PIN 0,1,,8,9 (15,16,17?)

analog pins not used
PIN 4,5 
and digital pins (pins 4, 6, 8, 9, 10, and 12 can be used for analog)
*/

#include <TimerOne.h> // timer interrupt
#include <Stepper.h> // 
#include <Unistep2.h> // non blocking stepper motor
#include <AltSoftSerial.h>

/*http://r6500.blogspot.com/2014/12/fast-pwm-on-arduino-leonardo.html */
// Frequency modes for TIMER4
#define PWM187k 1   // 187500 Hz
#define PWM94k  2   //  93750 Hz
#define PWM47k  3   //  46875 Hz // using this
#define PWM23k  4   //  23437 Hz
#define PWM12k  5   //  11719 Hz
#define PWM6k   6   //   5859 Hz
#define PWM3k   7   //   2930 Hz

// Direct PWM change variables
#define PWM6        OCR4D
#define PWM13       OCR4A
// Terminal count
#define PWM6_13_MAX OCR4C

#define DUTY2PWM(x)  ((255*(x))/100)

#define STEPS 200 // the number of steps in one revolution of your motor (28BYJ-48)

#define DIRECPIN 3 
#define LOAD_VOLTAGE 0
#define INITIAL_VOLTAGE 1
#define INITIAL_CURRENT 2

#define POWER_DIFF 0.05
#define LOAD_RESISTANCE 100

Unistep2 stepperX(2,4,3,7, STEPS, 10000); // non blocking bipolar stepper motor
    // connections with the L298
     // red to green A
     // blue to green B
     // green to red A
     // black to red B
     // stepper motor 4 pins on the L298 left to right pins 2,3,4,7

AltSoftSerial altSerial; // pin 5 for transmitting
// PWM at pin 6

int previous_direction = 0;
int val_direction = 0;
int step_direction = 0;
int dutycycle = 50;
float maxPower = 0, minPower = 99 , avgPower = 0 , totalE = 0 , power = 0;
float read_voltage = 0.0;
float read_current = 0.0;
float prev_power = 0.0;
int facing = 0; 
int degree = 0;
int  serial_write = 50;
float power_after = 0.0;
float current_after_boost = 0.0;
float load_voltage = 0.0;
float load_voltage_sum = 0.0;
float read_current_sum = 0.0;
float read_voltage_sum = 0.0;
int high_enough = 0;
int n = 0;
float power_after_sum = 0.0;
int i = 0;
int inc = 1;
int dec = 0;


// -------------------------  //
// PWM HARDWARE TIMER CONFIG  // -------- -------- -------- -------- -------- 
// -------------------------- //
// Configure the PWM clock
// The argument is one of the 7 previously defined modes
void pwm613configure(int mode)
{
  // TCCR4A configuration
  TCCR4A=0;
  // TCCR4B configuration
  TCCR4B=mode;
  // TCCR4C configuration
  TCCR4C=0;
  // TCCR4D configuration
  TCCR4D=0;
  // TCCR4D configuration
  TCCR4D=0;
  // PLL Configuration
  // Use 96MHz / 2 = 48MHz
  PLLFRQ=(PLLFRQ&0xCF)|0x30;
  // PLLFRQ=(PLLFRQ&0xCF)|0x10; // Will double all frequencies
  // Terminal count for Timer 4 PWM
  OCR4C=255;
}

// -------- //
// PWM SET  // -------- -------- -------- -------- -------- -------- 
// -------- //
// Set PWM to D9
// Argument is PWM between 0 and 255
// Set PWM to D13 (Timer4 A)
// Argument is PWM between 0 and 255
//void pwmSet13(int value)
//{
//OCR4A=value;   // Set PWM value
//DDRC|=1<<7;    // Set Output Mode C7
//TCCR4A=0x82;  // Activate channel A
//}

void pwmSet6(int value)
{
OCR4D=value;   // Set PWM value
DDRD|=1<<7;    // Set Output Mode D7
TCCR4C|=0x09;  // Activate channel D
}


// ------------  //
// INITIAL SETUP // -------- -------- -------- -------- -------- -------- -------- 
// ------------- //

void setup(void)
{
  Timer1.initialize(150000); //interrupt every 0.15 seconds
  Timer1.attachInterrupt(turbine_ISR); 
  // stepper.setSpeed(10); 
  pwm613configure(PWM94k); // configure pin 6 for PWM using hardware registers at 94kHz
  Serial.begin(9600);
  Serial1.begin(9600);
    while (!Serial1) {}
     // wait for serial port to connect. Needed for native USB port only  altSerial.begin(9600);
  altSerial.begin(9600);
}

// --------  //
// MAIN LOOP //  -------- -------- -------- -------- -------- -------- -------- 
// --------- //

void loop(void)
{
  stepperX.run(); // have to run for the nonblocking motor 
  read_direc(); // get all values 
  
//    Serial.print("Current Before: ");
//    Serial.println(read_current, 5);
//    Serial.print("Voltage Before: ");
//    Serial.println(read_voltage, 5);
//    Serial.print("Current After: ");
//    Serial.println(current_after_boost, 5);
//    Serial.print("Voltage After: ");
//    Serial.println(load_voltage);
//    Serial.print("Power After: ");
//    Serial.println(power_after, 5);
//    Serial.print("Duty: ");
//    Serial.println(dutycycle); 
//      //  delay(500);//delay for visual
}

// ------------------------  //
// READ CURRENT AND VOLTAGE  //  -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //

float digvolt(int digital) {
    return digital*5.0/1023.0; // digital to voltage
}

float digcurr(float digital) {
    return (-1.0838*digital + 2.6797); // current sensor equation experimentally found
}

void read_direc() {
  // read direction
  
  val_direction = analogRead(DIRECPIN);
  //read difference in direction
  step_direction = val_direction - previous_direction; 
  
//  read_voltage =  digvolt(analogRead(INITIAL_VOLTAGE))*3.0;
//  read_current =  digcurr(digvolt(analogRead(INITIAL_CURRENT)));
//  load_voltage = analogRead(LOAD_VOLTAGE) * (5.0/1023.0) * 9.3;

  read_voltage_sum = read_voltage_sum + digvolt(analogRead(INITIAL_VOLTAGE))*3.0;
  read_current_sum = read_current_sum +  digcurr(digvolt(analogRead(INITIAL_CURRENT)));
  load_voltage_sum = load_voltage_sum + analogRead(LOAD_VOLTAGE) * (5.0/1023.0) * 9.3;
  
 n = n + 1;

  if (n==100) {
    read_voltage = read_voltage_sum/100.0;
    read_current = read_current_sum/100.0;
    load_voltage = load_voltage_sum/100.0;
    
    n=0;
    read_voltage_sum = 0.0;
    read_current_sum = 0.0;
    load_voltage_sum = 0.0;
  }

  current_after_boost = load_voltage / LOAD_RESISTANCE;

  i = i+1;

  power = (read_current*read_voltage); 

  power_after_sum = (load_voltage/LOAD_RESISTANCE) + power_after_sum;

  if (i == 100) {
      power_after = power_after_sum/100.0;
      power_after_sum = 0.0;
      i = 0;
   }
 
}

// ---  //
// ISR // -------- -------- -------- -------- -------- -------- -------- -------- -------- 
// --- //

void turbine_ISR() {
  change_step(); // direction control
  change_PWM();  // PWM control
  print_bt();    // serial control
}

void change_step(void)
{
  if ( stepperX.stepsToGo() == 0 && abs(step_direction) > 50){ // If stepsToGo returns 0 the stepper is not moving
    stepperX.move(step_direction/4.5);
    previous_direction = val_direction;
    degree = 0.204*val_direction - 104;
  } 
}

// ------------------------  //
// CHANGE PWM DEPNT ON POWER  // -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //
void change_PWM() {
  
  // wait till voltage has reached 10 Volts (is spinning fast enough after starting up) once to start changing duty cycle
  if (load_voltage > 10) {
    high_enough = 1;
  }
  
  if (load_voltage < 5) { // when turning off 
    high_enough = 0;
  }

  if (high_enough) {

      if (power > (prev_power + POWER_DIFF) && inc) { // increase duty cycle when power is greater than previous power and rising
          dutycycle = dutycycle + 1;
          prev_power = power;
          inc = 1;
          dec = 0;
      }
      
      else if (power < (prev_power - POWER_DIFF) && inc) { // decrease duty cycle when power is less than previous power when rising
          dutycycle = dutycycle - 1;
          prev_power = power;
          dec = 1;
          inc = 0;
      }

      else if (power < (prev_power - POWER_DIFF) && dec) { // increase duty cycle when power is less than previous power when decreasing 
          dutycycle = dutycycle + 1;
          prev_power = power;
          inc = 1;
          dec = 0;
      }

      else if (power > (prev_power - POWER_DIFF) && dec) { // decrease duty cycle when power is greater than previous power when decreasing
          dutycycle = dutycycle - 1;
          prev_power = power;
          dec = 1;
          inc = 0;
      }
      
      else {
        dutycycle = dutycycle; // max or min duty then don't change or if power is the same
      }
  }
      // set pin 6 for PWM output to boost converter

  // max and min duty cycle that the boost is capable of taking
  if (dutycycle < 20) dutycycle = 20; 
  if (dutycycle > 80) dutycycle = 80;
      
  if(maxPower < power) maxPower = power;  
  if(minPower > power) minPower = power;

  pwmSet6(DUTY2PWM(dutycycle));  

}

int currentMillis;
// ------------------------  //
// BLUETOOTH SERIAL PRINT    // -------- -------- -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //

void print_bt() {
  /* SERIAL PRINTING TO BLUETOOTH */
 
/* calculations, average wind speed and average direction? Maximum power*/
    // currentMillis = millis()/1000;
    //totalE = power*currentMillis;
//    Serial.print("Current Before: ");
//    Serial.println(read_current, 5);
//    Serial.print("Voltage Before: ");
//    Serial.println(read_voltage, 5);
//    Serial.print("Current After: ");
//    Serial.println(current_after_boost, 5);
//    Serial.print("Voltage After: ");
//    Serial.println(load_voltage, 5);
//    Serial.print("Power After: ");
//    Serial.println(power_after, 5);
//    Serial.print("Duty: ");
//    Serial.println(dutycycle);    
   
    String powervals = String(power_after)  + ',' + String(dutycycle)  + ',' + String(load_voltage)+ ',' + String(degree) ',' 
        + String(read_current)  ',' + String(power) ',' + String(read_voltage) ',' + String(current_after_boost);  

    String powervals_bt = String(power_after)  + ','+ String(load_voltage)  + ',' + String(read_current)+','+ String(degree) + ';';    

    //altSerial.println(powervals_bt); 
    
    Serial.println(powervals); // print to python Serial
}
