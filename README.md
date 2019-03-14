# WIND TURBINE
Arduino powered wind turbine using MPPT and PWM


## Final Requirements
Use the Arduino to collect data from a wind direction sensor, voltage and current from a boost converter. This should then use Maximum Power Point Tracking. 


This includes using PWM to control the switches in a boost converter

## MPPT

* Trial and Error Method: continue increasing until the power decreases, then start decreasing or increaseing PWM duty cycle to reach a maximum
* coeff of power vs tip speed ratio graph 

## Set up
### Equipment Needed
- Arduino Leonardo
- HC-06 Bluetooth Module
- Stepper motor and driver

**Connecting the Stepper Motor**

Connections with the L298 Driver
- red wire to green A
- blue wire to green B
- green wire to red A
- black wire to red B

Stepper motor 4 pins on the L298 left to right Pins 2,3,6,7

**Connecting the bluetooth module**

Pin 5 is bluetooth RXD

Power and ground to respective places

**Pin 13 is PWM output**

**Generator Stator**

PIN 10, 11,12 for switching


## Code/Serial Setup
Dependencies: ```numpy, SidePy, qtpygraph```

Run the python script for serial grapher: 

```python receiver.py```