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

Connect bluetooth module to Arduino
        (Not all pins on the Leonardo and Micro support change interrupts,
        so only the following can be used for RX:
        8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).)

Leo -- bluetooth
PIN 15 -- TXD
PIN 8 -- RXD
*/

#include "Stepper.h" // For the stepper motor
#include <Unistep2.h>
#include "Tlc5940.h" // PWM
#include "tlc_fades.h" // PWM
#include "SoftwareSerial.h" // bluetooth


#define STEPS 1023 // the number of steps in one revolution of your motor (28BYJ-48)

#define BOOSTPIN 0 // TLC SHIELD

#define DIRECPIN 0 // Analog 0
#define BC_VOLTAGE 1 // Analog 1
#define BC_CURRENT 2 // Analog 2

#define MAXDUTY 68 // Dependent on what the switching frequency is

Stepper stepper(STEPS, 2, 6, 4, 7); // Connections: 2- orange/IN1, 4 - blue/IN2, 6 - green/IN3, 7 - yellow/IN4

SoftwareSerial mySerial(15, 8); // RX, TX

int previous_direction = 0;
float bc_current, bc_voltage, read_current, read_voltage;
int duty = MAXDUTY/2;

int currentMillis;
float power = 0;
float maxPower, minPower, avgPower, totalE;
float prev_power = -1;

void setup() {
    Serial.begin(9600); // connect to computer
    Tlc.init(); //PWM init 
    stepper.setSpeed(10); 
      // Open serial communications and wait for port to open:
    Serial1.begin(9600);
    while (!Serial1) {}
     // wait for serial port to connect. Needed for native USB port only
    mySerial.begin(9600);
    mySerial.println("Start");
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
/*
int val_direction = analogRead(DIRECPIN);

int step_direction = val_direction - previous_direction;

if (abs(step_direction) > 40)  {
  stepper.step(step_direction); // blocking function
  previous_direction = val_direction;
}
*/
/*
|------------------------------|
|  CONTROL LOOP 2 - PWM BOOST  |
|------------------------------|
*/

// current pseudo code
read_voltage = digvolt(analogRead(BC_VOLTAGE));
read_current = digvolt(analogRead(BC_CURRENT));

bc_current = read_current;
bc_voltage = read_voltage;

power = bc_voltage * bc_current;

if (power > prev_power && duty != 0) {
    duty--;
}

else if (power < prev_power && duty != MAXDUTY) {
    duty++;
}

else {
  duty = duty; // max or min duty then don't change or if power is the same
}

if(maxPower < power) maxPower = power;
if(minPower > power) minPower = power;

// Update the PWM output
Tlc.set(BOOSTPIN,40);
Tlc.update();

prev_power = power;


/* DEMO 1 code
int voltage0 = analogRead(0);
int voltage1 = analogRead(1);
// PWM value (0 - 68) with switching freq 118kHz


Tlc.set(BOOSTPIN, voltage0/15) ;
Tlc.update();

float v0 = digvolt(float(voltage0));
float v1 = digvolt(float(voltage1));
*/

/* SERIAL PRINTING TO BLUETOOTH */

/* calculations, average wind speed and average direction? Maximum power*/

avgPower = (maxPower + minPower)/2;
currentMillis = millis()/1000;
totalE = power*currentMillis;

/*String data = String(maxPower) + ',' + String(minPower) +  ',' + String(avePower) +  ',' + String(totalE);
mySerial.print(data);*/

/*-------*/

mySerial.print(maxPower);  
mySerial.print(",");  
mySerial.print(minPower);  
mySerial.print(",");  
mySerial.print(avgPower);  
mySerial.print(",");  
mySerial.print(totalE);    
mySerial.print(",");  
mySerial.print(power); 
mySerial.print(",");  
mySerial.print(read_voltage);    
mySerial.print(",");  
mySerial.print(read_current);           
mySerial.print(";");

/*
mySerial.print("Analog Input 0: ");
mySerial.print(v0);
mySerial.print(" Analog Input 1: ");
mySerial.println(v1);
*/
}

float digvolt(float digital) {
    return digital*5/1023;
}
