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


CONNECT THE 
*/

#include <Stepper.h> // For the stepper motor

#define STEPS 1023 // the number of steps in one revolution of your motor (28BYJ-48)

Stepper stepper(STEPS, 2, 4, 3, 6);

int previous_direction = 0;
float bc_current, bc_voltage, read_current, read_current;

direcpin = 0 //

void setup() {
    Serial.begin(9600); // connect to computer
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

val_direction = analogRead(direcPin);

step_direction = previous_direction - val_direction; 

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


//PWM stuff

read_voltage = digvolt(analogRead(1));
read_current = analogRead(2)

bc_current = read_current;
bc_voltage = read_voltage;

power = bc_voltage * bc_current;


if (power > prev_power) {
    //decrease duty cycle
}

if (power < prev_power) {
    //increase duty cycle
}

prev_power = power;

}

float digvolt(float digital) {
    return digital*5/1023;
}





  