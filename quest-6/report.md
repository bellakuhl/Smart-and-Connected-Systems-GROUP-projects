# Quest Name

Authors: Isabella Kuhl, Laura Reeve, Joseph Rossi

2019-12-08

## Summary

In this final quest, our team implemented autonomous driving and web server control for our crawler by combining and building upon previous skills. More specifically, our crawler is able to detect traffic light beacons (provided by the instructors) and either stop or go, depending on the current light. It is also able to detect possible collisions and turn to avoid incoming obstacles, all without human interference. Finally, we used PID control to drive straight and maintain a desired speed. The final test for this project is a race course (Figure 1) featuring three traffic lights and a QR code finish line. The crawler must be able to complete this track, obeying the traffic lights, and switch to remote driving mode when it reaches the third light. From here the driver must drive up to the QR code finish line, scan and decode the image in order to complete the course.

![Course](https://github.com/BU-EC444/Team15-Kuhl-Reeve-Rossi/blob/bk-quest6-dev/quest-6/images/course2019.png)
Figure 1

## Evaluation Criteria

This quest combines elements of many skills in order to accomplish autonomous driving. These elements are broken up into two sections: web server interface and sensor functions.

#### Web Server Interface
The web server for this project had to be far more robust than previous quests in order to provide the user with the necessary information for autonomous driving. One of the main components is the ability to control speed and steering direction from the server. In addition to these controls, a live stream from a webcam mounted on the crawler has to be displayed for the user to operate the vehicle remotely. It also must include the ability to log the "splits" recorded which are defined as the time it takes to go from one stop light to the next. Lastly, the end of the course is reached when the QR code at the finish line is decoded. This requires the server to also be able to take an image of the code and display the encoded text.

#### Sensor Functions
In order to drive autonomously, it is necessary that it is outfitted with a number of sensors. The type and location of each sensor is up to the discretion of the team although the full system must accomplish the same tasks. One such task is PID control for speed and steering. This ensures the crawler is both driving straight as well as maintaining a constant, set speed. These sensors also must prevent front collisions, make appropriate left or right turns, and keep the crawler approximately 0.75 meters away from side walls. This distance is important as the traffic light beacons will be located this far from each wall. When a traffic light is detected the crawler should interpret the light using an IR receiver and react according to the current state (green–go, red–stop, yellow–slow down). Additionally, the split time between reaching each traffic light should be displayed on the vehicle using an alphanumeric display.


## Solution Design

[webserver]

[beacons]

[sensors]


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
