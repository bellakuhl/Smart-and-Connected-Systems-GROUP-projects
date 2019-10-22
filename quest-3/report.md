# Quest 3: Wearable Computer with Remote Access
Authors: Joseph Rossi, Isabelle Kuhl, Laura Reeve

2019-10-22

## Summary

In this quest, we created a fitness tracker that allows the user to track their steps, body temperature, and phone battery. The user can also be alerted; a blue LED blinks to remind the user to drink water and a red LED blinks to locate the device. The data collected is displayed on a web server hosted on a DDNS, allowing the user's friends and family to view the fitness metrics.


## Evaluation Criteria

Measurements for step, body temperature, and battery level - The firmware reads in the data from the thermistor, battery, and accelerometer to continuously poll for these measurements.

Regularly scheduled alerts [to drink water] (blinks blue LED)

An online portal displaying a real-time status report

Ability to find your device from web portal (blinks red LED)

Remote access to turn off features (i.e., only measure steps but no temperature) and view the portal

## Solution Design

### Hardware

Add schematic

### Firmware

The firmware was broken up into parts for the temperature, battery level, step counter, and alert system. The temperature and battery level files are based on the ADC example code. For the battery monitor, we measure the voltage across a voltage divider consisting of two 10k reistors in series using ADC. For the thermistor, we again use a voltage divider to measure the voltage drop across the thermistor, then use this to calculate the resistance. From there, we calculate the temperature using the values given on the spec sheets for the NTC thermistor and convert those values from Kelvin to Celsius.

For the step counter, we used the ADXL343 Accelerometer and an i2c master-slave configuration to read in the acceleration in the x, y, and z directions. From here, we used the acceleration on the z-axis to measure steps. We do this because the vertical direction is where you get the greatest fluctuations when a person is walking. We have a threshold variable that that is updated every second that takes the mean of the max and min values from that second. When the z acceleration goes above that threshold, it is seen as a 'step'.

For the alert system, we have two different alerts that the user can implement. The first alert is a scheduled reminder to drink water. This is scheduled to flash the blue LED on a regular interval (every x seconds). The second alert is an immediate on/off switch for the user to find the device. When on, it continuously flashes the red LED until the user switches it to off.

### Software

## Sketches and Photos

<center><img src="./images/example.png" width="70%" /></center>  
<center> </center>


## Supporting Artifacts

- [Link to repo]()
- [Link to video demo]()


## References

-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private
