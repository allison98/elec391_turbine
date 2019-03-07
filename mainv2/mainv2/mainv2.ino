// Using an interrupt for the stepper motot, now bipolar (h bridge driven)
// using the timer registers for PWM out put


// timer 4 PWM trigger
// timer 1 altserial
// timer 3 interrupt 


#include <TimerOne.h> // timer interrupt
#include <Stepper.h> // 
#include <Unistep2.h> // non blocking stepper motor
#include <AltSoftSerial.h>

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

Unistep2 stepperX(2,6,3,7, STEPS, 10000); // non blocking stepper motor
//SoftwareSerial mySerial(8, 9); // RX, TX
AltSoftSerial altSerial; // pin 5 for transmitting
// PWM at pin 13

//Stepper stepper(STEPS, 2, 6, 4, 7); // Connections: 2- orange/IN1, 4 - blue/IN2, 6 - green/IN3, 7 - yellow/IN4


int previous_direction = 0;
int val_direction = 0;
int step_direction = 0;
int dutycycle = 50;
float maxPower = 0, minPower = 99 , avgPower = 0 , totalE = 0 , power = 0;
float read_voltage = 0;
float read_current = 0;
float prev_power = 0;


// ------------  //
// INITIAL SETUP //
// ------------- //

void setup(void)
{
 
  Timer1.initialize(500000);
  Timer1.attachInterrupt(turbine_ISR); // blinkLED to run every 0.15 seconds
  // stepper.setSpeed(10); 
  pwm613configure(PWM47k); // configure pin 9 for PWM using hardware registers
  Serial.begin(9600);
  Serial1.begin(9600);
    while (!Serial1) {}
     // wait for serial port to connect. Needed for native USB port only  altSerial.begin(9600);
  altSerial.begin(9600);
}


// ---  //
// ISR //
// --- //

void turbine_ISR() {
  change_step_ISR(); //update all values every 0.5 seconds
  change_PWM();
  print_bt();
}

void change_step_ISR(void)
{

  if ( stepperX.stepsToGo() == 0 && abs(step_direction) > 100){ // If stepsToGo returns 0 the stepper is not moving
    stepperX.move(step_direction/5);
    previous_direction = val_direction;
  } 
}


// --------  //
// MAIN LOOP //
// --------- //

void loop(void)
{
  
  stepperX.run(); // have to run for the nonblocking motor 
  read_direc();// get all values
 
}

// ------------------------  //
// READ CURRENT AND VOLTAGE  //
// ------------------------- //

float digvolt(float digital) {
    return digital*5/1023;
}

float digcurr(float digital) {
    return (10.639*digital - 26.643);
}

void read_direc() {
  val_direction = analogRead(DIRECPIN);
  step_direction = val_direction - previous_direction; 
  read_voltage = digvolt(analogRead(INITIAL_VOLTAGE));
 // read_current = digcurr(analogRead(INITIAL_CURRENT));
}

// ------------------------  //
// CHANGE PWM DEPNT ON POWER  //
// ------------------------- //
void change_PWM() {
  read_current = 0.4;
  power = read_voltage * read_current;
  //for demo
  //power = read_voltage;
  
if (power > (prev_power + 0.05)) {
    dutycycle = dutycycle + 3;
    prev_power = power;
}

else if (power < (prev_power - 0.05)) {
    dutycycle = dutycycle - 3;
    prev_power = power;
}

else {
  dutycycle = dutycycle; // max or min duty then don't change or if power is the same
}

if (dutycycle < 0) dutycycle = 0;
if (dutycycle > 100) dutycycle = 100;
    
  if(maxPower < power) maxPower = power;
  
  if(minPower > power) minPower = power;
  
  // set pin 9 for PWM output to boost converter
  pwmSet13(DUTY2PWM(dutycycle));  
}

int currentMillis;
// ------------------------  //
// BLUETOOTH SERIAL PRINT    //
// ------------------------- //

void print_bt() {
  /* SERIAL PRINTING TO BLUETOOTH */
   change_step_ISR();
/* calculations, average wind speed and average direction? Maximum power*/
    avgPower = (maxPower + minPower)/2;
   // currentMillis = millis()/1000;
    //totalE = power*currentMillis;

    String powervals = String(avgPower)  + ',' + String(power)  + ',' + String(read_voltage)+ ';';    
    altSerial.print(powervals);  

}

// -------------------------  //
// PWM HARDWARE TIMER CONFIG  //
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
// PWM SET  //
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
