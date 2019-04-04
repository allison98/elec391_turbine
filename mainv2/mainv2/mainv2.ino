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
Pin 13 is PWM 
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

// PINS FOR READING< DIRECTLY ON PCB
#define DIRECPIN 4 
#define LOAD_VOLTAGE 0
#define INITIAL_VOLTAGE 1
#define INITIAL_CURRENT 2
#define SPEED_PIN 5
#define TRACKING 11

#define POWER_DIFF 0.02
#define LOAD_RESISTANCE 87.0

#define center_low 1.98
#define center_high 2.30

Unistep2 stepperX(2,4,3,7, STEPS, 10000); // non blocking bipolar stepper motor
    // connections with the L298
     // red to green A
     // blue to green B
     // green to red A
     // black to red B
     // stepper motor 4 pins on the L298 left to right pins 2,3,4,7

AltSoftSerial altSerial; // pin 5 for transmitting
// PWM at pin 6

// variable declaration
int previous_direction = 0;
int val_direction = 0;
int step_direction = 0;
int dutycycle = 35;
float maxPower = 0, minPower = 99 , avgPower = 0 , totalE = 0 , power = 0;
float read_voltage = 0.0;
float read_current = 0.0;
float read_voltage_avg = 0.0;
float read_current_avg = 0.0;
float prev_power = 0.0;
int facing = 0; 
int degree = 0;
int  serial_write = 50;
float power_after = 0.0;
float current_after_boost = 0.0;
float load_voltage = 0.0;
float load_voltage_avg = 0.0;

float prev_load_voltage = 0.0;
float speed_rpm = 0.0, speed2 = 0.0;

float load_voltage_sum = 0.0;
float read_current_sum = 0.0;
float read_voltage_sum = 0.0;
float current_position_sum = 0.0;
float speed_sum = 0.0;

float load_voltage_sum_2 = 0.0;
float read_current_sum_2 = 0.0;
float read_voltage_sum_2 = 0.0;
float speed_sum_2 = 0.0;

float power_after_sum = 0.0;

int high_enough = 0;
int steady = 0;
int n = 0, i = 0, t = 0, s = 0, v = 0, p = 0;
int inc = 1;
int dec = 0;

float current_position = 0;

float freq;
boolean ir_sensor, prev_ir_sensor;
long num = 0;
float prev = 0.0;


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
void pwmSet13(int value)
{
OCR4A=value;   // Set PWM value
DDRC|=1<<7;    // Set Output Mode C7
TCCR4A=0x82;  // Activate channel A
}

// ------------  //
// INITIAL SETUP // -------- -------- -------- -------- -------- -------- -------- 
// ------------- //

void setup(void)
{
  Timer1.initialize(100000); //interrupt every 0.1 second
  Timer1.attachInterrupt(turbine_ISR); 
  pwm613configure(PWM94k); // configure pin 6 for PWM using hardware registers at 94kHz
  Serial.begin(9600);
  pinMode(TRACKING, INPUT); // set trackingPin as INPUT
}

// --------  //
// MAIN LOOP //  -------- -------- -------- -------- -------- -------- -------- 
// --------- //

void loop(void)
{
  stepperX.run(); // have to run for the nonblocking motor 
  read_direc(); // get all values 
}

// ------------------------  //
// READ CURRENT AND VOLTAGE  //  -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //

float digvolt(int digital) {
    return digital*5.0/1023.0; // digital to voltage
}

float digcurr(float digital) {
    return (-1.0778*digital + 2.6541); // current sensor equation experimentally found
}

void read_direc() {
  // read direction with averaging
  read_voltage_sum = read_voltage_sum + digvolt(analogRead(INITIAL_VOLTAGE))*3.0; // obtained via voltage divider
  read_current_sum = read_current_sum +  digcurr(digvolt(analogRead(INITIAL_CURRENT)));
  load_voltage_sum = load_voltage_sum + digvolt(analogRead(LOAD_VOLTAGE))*9.3; // obtained via voltage divider
  current_position_sum = digvolt(analogRead(DIRECPIN)) + current_position_sum;
  power_after_sum = (load_voltage * load_voltage / LOAD_RESISTANCE) + power_after_sum;
  speed_sum = speed_sum + digvolt(analogRead(SPEED_PIN));

  n = n + 1;

  if (n == 500) {
    read_voltage = read_voltage_sum/500.0;
    read_current = read_current_sum/500.0;
    if(read_current < 0 ) read_current = 0;
    load_voltage = load_voltage_sum/500.0;
    current_position = current_position_sum/500;
    power_after = power_after_sum/500.0;
    speed2 = speed_sum/500.0; // 97.39*volt + 36.495
    speed_sum_2 = speed_sum_2 + speed2;  

    n=0;
    read_voltage_sum = 0.0;
    read_current_sum = 0.0;
    load_voltage_sum = 0.0;
    current_position_sum= 0.0;
    power_after_sum = 0.0;
    speed_sum = 0.0;
    v = v +1;
  }
  
  // wind speed 
  if (v==100) {
    speed_rpm = speed_sum_2/100.0;
    speed_sum_2 = 0;
    v = 0;
  }

  current_after_boost = load_voltage / LOAD_RESISTANCE;
  power = (read_current*read_voltage); 

  // MPPT
  t = t+1;  
  if (t == 600) {
    prev_load_voltage = load_voltage;
    t = 0; 
  }

  // RPM 
  ir_sensor = digitalRead(TRACKING); // read the value of tracking module
  if (ir_sensor and ir_sensor!= prev_ir_sensor)
    num++;   
  if (abs(millis()- prev) > 5000) {
     prev = millis();
     freq = num/(5.0);
     num = 0;
  }
  prev_ir_sensor = ir_sensor;
 
}

// ---  //
// ISR // -------- -------- -------- -------- -------- -------- -------- -------- -------- 
// --- //

void turbine_ISR() {
  
  change_step(); // direction control
    
  if (s == 100) { // sampling at 3 second because of large transient
    change_PWM();  
    s = 0;
  }
  s = s+1;
  if (p = 100) {
    print_bt(); // print to serial
    p =0;
  }
  p = p +1;
}

void change_step(void)
{
    if ( stepperX.stepsToGo() == 0 and (current_position < center_low)){ // If stepsToGo returns 0 the stepper is not moving
      stepperX.move(-1);  // move right 
    } 
    if ( stepperX.stepsToGo() == 0 and (current_position > center_high)){ // If stepsToGo returns 0 the stepper is not moving
      stepperX.move(1);  // move left
    } 
}


// ------------------------  //
// CHANGE PWM DEPNT ON POWER  // -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //
void change_PWM() {
  
  // wait till voltage has reached 10 Volts (is spinning fast enough after starting up) once to start changing duty cycle ; and wait to stabilize
  if (load_voltage > 7 && (abs(prev_load_voltage - load_voltage) < 1) ) {
    high_enough = 1; //   
  }  
  else {
    high_enough = 0;
  }

  if (load_voltage < 3) { // when turning off or wanting to reset
    high_enough = 0;
    dutycycle = 35;
  }

  if (high_enough) {

      if (power > (prev_power + POWER_DIFF) && inc) { // increase duty cycle when power is greater than previous power and rising
          dutycycle = dutycycle + 2;
          prev_power = power;
          inc = 1;
          dec = 0;
      }
      
      else if (power < (prev_power - POWER_DIFF) && inc) { // decrease duty cycle when power is less than previous power when rising
          dutycycle = dutycycle - 2;
          prev_power = power;
          dec = 1;
          inc = 0;
      }

      else if (power < (prev_power - POWER_DIFF) && dec) { // increase duty cycle when power is less than previous power when decreasing 
          dutycycle = dutycycle + 2;
          prev_power = power;
          inc = 1;
          dec = 0;
      }

      else if (power > (prev_power + POWER_DIFF) && dec) { // decrease duty cycle when power is greater than previous power when decreasing
          dutycycle = dutycycle - 2;
          prev_power = power;
          dec = 1;
          inc = 0;
      }
      
      else {
        dutycycle = dutycycle; // max or min duty then don't change or if power is the same
      }
   // max and min duty cycle that the boost is capable of taking
  if (dutycycle < 30) dutycycle = 30; 
  if (dutycycle > 75) dutycycle = 75;

  }
// safe guard
   if (load_voltage > 15) {
    dutycycle = 100;
    high_enough  = 0;
  }
// set pin 13 for PWM output to boost converter  
  pwmSet13(DUTY2PWM(dutycycle));  

}

// ------------------------  //
// BLUETOOTH SERIAL PRINT    // -------- -------- -------- -------- -------- -------- -------- -------- -------- 
// ------------------------- //
void print_bt() {
  /* SERIAL PRINTING TO BLUETOOTH */
   
  String powervals = String(power_after, 4)  + ',' + String(dutycycle)  + ',' + String(load_voltage, 4)+ ',' + String(prev_load_voltage)+ ',' + String(read_current, 4) + ',' + String(power, 4) + ',' + String(read_voltage, 4)+ ',' + String(current_position) + ',' + String(speed_rpm, 2) + ',' + String(freq);  
    
  Serial.println(powervals); // print to python Serial
}
