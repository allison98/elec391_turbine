/*

*/

#include <Stepper.h> // For the stepper motor

#define STEPS 2038 // the number of steps in one revolution of your motor (28BYJ-48)

Stepper stepper(STEPS, 8, 10, 9, 11);
int previous_direction = 0;

void setup() {
    Serial.begin(9600); // connect to computer
}

void loop() {

// Step 1: Measure all the variable

// Step 2: Manipulate the variables back to the original units or whatever
    // convert poteniometer value to a direction from 0 to 512 (90 degrees, calculated 1/4 of 2048)
    // thats the value we want it to go
    // if the value is larger than 512 use 512 if the number is closer to 0 use 0

val_direction = // measured value derivation

// Step 3: Compare variables and change the respective actions
    // a) measure wind direction then control stepper motor
    // b) measure booster converter values and control PWM

step_direction = previous_direction - val_direction; 
if (step_direction > 512)
    step_direciton = 512;
if (step direction < 0)
    step_direction = 0; 

// not the best idea
/* some how create a way that relates step to resistance so we can directly do
stepper.step(step_direction);
*/
switch (winddirection) {
  case 1ohm:
        stepper.step(step_direction);
    break;
  case 2ohm:
        stepper.step(step_direction);    
    break;
  default:
    // statements
    break;
}    

previous_direction = val_direction




/*    
    stepper.setSpeed(1); // 1 rpm
    stepper.step(2038); // do 2038 steps -- corresponds to one revolution in one minute
    delay(1000); // wait for one second
    stepper.setSpeed(1); // 1 rpm
    stepper.step(-2038); // do 2038 steps in the other direction */

    // Use for testing motor  
}





  