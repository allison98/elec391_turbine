/*
Connect the Arduino Leonard and TLC4950 (SparkFun PWM Shield) the following way:

Leo --- TLC4950

5V -- 5V
GND -- GND (one on each side) 
ICSP inner center pin - SLCK (13)
ICSP outer cennter pin - SIN (11)
PIN 10 - BLANK (10)
PIN 9 - XLAT (9)
PIN 5 -- GSCLK (3)
VIN -- VIN

*/

#include <Stepper.h> // For the stepper motor
#include "Tlc5940.h"
#include "tlc_fades.h"

#define STEPS 1023 // the number of steps in one revolution of your motor (28BYJ-48)
#define TLC_0 0
#define TLC_1 1
#define TLC_2 2
#define DIRECPIN 0
#define BC_VOLTAGE 1
#define BC_CURRENT 2

Stepper stepper(STEPS, 0, 2, 1, 4);

int previous_direction = 0;
float bc_current, bc_voltage, read_current, read_voltage;
int duty = 0;

float power = 1;
float prev_power;

void setup() {
    Serial.begin(9600); // connect to computer
    Tlc.init(); //PWM init 
      stepper.setSpeed(10); 
    
}

void loop() {   

// Step 1: Measure all the variable

// Step 2: Manipulate the variables back to the original units or whatever
    // convert poteniometer value to a direction from 0 to 512 (90 degrees, calculated 1/4 of 2048)
    // thats the value we want it to go
    // if the value is larger than 512 use 512 if the number is closer to 0 use 0

// Step 3: Compare variables and change the respective actions
    // a) measure wind direction then control stepper motor
    // b) measure booster converter values and control PWM
   //370 560 oohms 800

/*

|------------------------------|
|  CONTROL LOOP 1 - DIRECTION  |
|------------------------------|

*/

int val_direction = analogRead(DIRECPIN);

int step_direction = val_direction - previous_direction;
/*
if (step_direction > 512)
    step_direciton = 512;
if (step direction < 0)
    step_direction = 0;
*/

stepper.step(step_direction);

previous_direction = val_direction;


/*    
    stepper.setSpeed(1); // 1 rpm
    stepper.step(2038); // do 2038 steps -- corresponds to one revolution in one minute
    delay(1000); // wait for one second
    stepper.setSpeed(1); // 1 rpm
    stepper.step(-2038); // do 2038 steps in the other direction */

    // Use for testing motor 

/*
|------------------------------|
|  CONTROL LOOP 2 - PWM BOOST  |
|------------------------------|
*/

// current pseudo code
/*read_voltage = digvolt(analogRead(BC_VOLTAGE));
read_current = analogRead(BC_CURRENT);

bc_current = read_current;
bc_voltage = read_voltage;

power = bc_voltage * bc_current;


if (power > prev_power) {
    duty++;
}
else if (power < prev_power) {
    duty--;
}

Tlc.set(TLC_0,duty);
Tlc.update();

prev_power = power;

*/

/* DEMO 1 code*/

int voltage = analogRead(1);
// PWM value (0 - 4095)

if (voltage <= 200) 
   Tlc.set(0,1000);
else if (voltage <=400 && voltage > 200)
  Tlc.set(0, 2000);
else if (voltage <=600 && voltage > 800)
  Tlc.set(0, 3000);
else
  Tlc.set(0, 4000); 

Tlc.set(0, 4*voltage);
Tlc.set(1, 4*voltage);
Tlc.set(2, 4*voltage);
Tlc.update();
delay(500);   
}

float digvolt(float digital) {
    return digital*5/1023;
}
