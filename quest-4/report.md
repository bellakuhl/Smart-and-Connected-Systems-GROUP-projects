# Crawler
Authors: Isabella Kuhl, Laura Reeve, Joseph Rossi

2019-11-06

## Summary

In this quest, we interfaced with the provided crawlers to create an autonomous driving system. The criteria for this project is that the vehicle has to drive straight along a designated line, maintaining a speed of 0.1 - 0.4 m/s. The system also needs to rely on PID control to control both the speed and the steering as well as stop within 10cm of an obstacle. We executed this by using skills learned previous; the microLIDAR and ultrasonic sensors were mounted on the side and front of the vehicle and allow us to sense how far away obstacles are. Specifically, the microLIDAR sensors were mounted on one side of the crawler and their readings determined how much the steering was adjusted in order to maintain a consistent distance from the wall. The ultrasonic was then mounted on the front, detecting how far objects were, and stopping the vehicle before it crashed.


We also setup a webserver to control the crawler's speed, direction, and steering wirelessly. Along with controlling the movement, a log console allows the user to continuously see the values outputted by the various sensors as the device operates. Finally, we have an alphanumeric display showing the user what the current speed is, based on the calculated wheel speed. Wheel speed is calculated by using an optical encoder pointed at a repeating patter stuck to the rear wheel. As the motors turn, the pattern turns as well and the optical encoder sends back varying voltages in a pulse wave form that can then be used (along with the diameter of the tire) to determine how far the car has gone. This information is used to calculate the error through PID control, thus updating the motor speed.


## Evaluation Criteria

**Uses PID for speed control at 0.1 m/s**

Using the P in PID control, we periodically measure the speed
of left right wheel and the PID control will adjust the wheel speed.

**Stops within 10cm of the end**

The ultrasonic sensor is placed such that the front of the car + 10cm is
the minimum range of the sensor (30cm). When the car detects an object
less than 35cm in front of it, the car will stop.


**Uses alpha display to show current postion or speed**

The speed is printing to the alpha display every measurement period.


**Successfully transverses A-B in one go, no hits or nudges**

In our testing, it did so.


## Solution Design

### Speed and PID Control (Bella)

### Distance Sensing (Joe)

### Steering Control (Laura)

### Web Controls (Joe)


## Sketches and Photos
<center><img src="./images/example.png" width="70%" /></center>  
<center> </center>


## Supporting Artifacts
- [Link to repo](https://github.com/BU-EC444/Team15-Kuhl-Reeve-Rossi/tree/master/quest-4)
- [Link to video demo]()


## Investigative Question (Laura?)


## References


-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private
