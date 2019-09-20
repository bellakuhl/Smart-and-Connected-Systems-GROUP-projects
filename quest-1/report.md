# Quest 1 - Retro Alarm Clock

Authors: Joe Rossi, Isabella Kuhl, Laura Reeve

2019-09-19

## Summary

For this quest we designed the clock by breaking it down into three subsystems:

* Inputs - Laura Reeve
* Time Management - Joseph Rossi
* Outputs - Isabella Kuhl

The input subsystem used a console interface to allow the user to take the following
actions:

* Set Time
* Set Alarm Time
* Enable Alarm
* Disable Alarm (also used to dismiss an active alarm)

The input system would then translate these actions into API calls into the timekeeping 
system which would propaage state and data changes to the outputs.  We used the UART 
controller to allow the user to take action via the console. The time keeping subsystem
used a hardware timer to keep time with a timer alarm triggering an interrupt to sound
the alarm. The timekeeping subsystem setup tasks to notify observers of time or mode
changes using callbacks.

## Evaluation Criteria

We tested the functionality by following this test protocol:

1. Power up the micro
    * Check the display blinks '----' and the hands are at 0
2. Follow the console prompt to set the time
    * The display should be blinking
    * The hours and minutes should display
    * The display should stop blinking when set
3. Via the console, set an alarm time 1 minute after the current alarm.
    * The display should be blinking.
    * Once, set, the clock should return to displaying the time.
4. Via the console, enable the alarm.
5. Wait one minute
    * The display should flash "ALRM"
6. Follow the prompt on the console to dismiss the alarm.
    * The time should go back to displaying the time without 
      flashing.

## Solution Design

The design is detailed in the specification documents in the [specs](./specs) folder. 
A good place to start is with the [Overview](./specs/Overview.md).

The user performs atction on the clock, the I/O logic call time keeping functions. The time keeping
subsystem can then propagate changes in state to the outputs which can adjust their displays 
accordingly. Outputs register callbacks with the time keeping system in `app_main` with the promise
they will get called as time progresses or as the user takes action.

The time keeping system uses a hardware timer to keep time with a task that poll the timer every second 
to update the internal. The program uses a single global struct that stores all of the clocks latest state,
including the current time, the alarm time, etc. You can see the type definitions in 
[retro_clock.h](./code/main/retro_clock.h).  When running, a background task is responsible for polling
the hardware timer to update the clock time.

Outputs simply need to register a callback with the clock via the `retro_clock_regiser_update_callback` with
a function pointer and they will get notified. This solution is scalable so long as the total time it takes
to notify the callbacks is less than a second.


## Investigative Question
How can you synchronize multiple ESP clocks with each other? Elaborate

If we want to synchronize multiple clocks with each other, the two best protocols to use would be Network Time Protocol (NTP) or Precision Time Protocol (PTP). PTP is incredibly accurate due to hardware timestamping, using multiple timestamps to sync a slave clock to a master clock. NTP is slightly less accurate but does not require hardware timestamping. In the ESP-32 specifications, they claim to have PTP hardware support, so PTP could be an option, but unless extremely high precision is necessary, NTP is simpler to implement. To start, we can set up a LAN server that all of the ESP devices can use to communicate. Both of the protocols send and recieve timestamps to establish a common time. NTP uses a hierarchical structure to establish proper time (generally UTC), but if we want these clocks to synchronize without using UTC, we can establish our own server times. Then all of the clocks will poll these higher clocks by sending and recieving timestamps (via UDP), using these to calculate the offset and delay and update its time to match up with those higher level clocks. 

One of the potential issues that we'd face would be knowing which clock(s) from the group to use to set our server clock values. If our goal is to use a user-inputted time to sync all of the clocks, that could be used as our base server clocktime. Otherwise, finding the mean of the clock times (excluding outliers) using, for example, the Berkeley algorithm, which doesn't require any seed time and simply finds an average time across multiple clocks and polls the slave clocks. Then we could use this time as the server time for our NTP implementation.

## Sketches and Photos

<center><p>Clock with time set</p><img src="./images/time-set.png" width="70%" /></center>  
<center><p>Alarm sounding</p><img src="./images/alarm-sounding.png" width="70%" /></center>  


## Supporting Artifacts

- [Link to repo](https://github.com/BU-EC444/Team15-Kuhl-Reeve-Rossi/tree/master/quest-1)
- [Demo Video](https://drive.google.com/file/d/19ph8pJgsHekSAIAexwMMzVPk7IlnPV5K/view?usp=sharing)


## References

-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private
