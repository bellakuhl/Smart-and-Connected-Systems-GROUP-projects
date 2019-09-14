# Retro Clock Outputs Specification

This document outlines the behaviors of the alphanumeric display and
the clock hands of retro clock through the different states. Since they
are solely output mechanisms, they are only responsible for updating their
outputs when update functions are notified of state or data changes


## State Behaviors


### RC_MODE_NOT_SET

In this state, the display should blink: "----". The clock hands
should be pointing to the equivalent of 12:00:00


### RC_MODE_SET_TIME

In this state, the display should blink: "HH.MM" where
"HH" is the current clock time hours and "MM" is the clock
time minutes.

The minute hand should move to the appropriate angle representing
the clock time as the user is inputting it. The second hand should
stay fixed at the angle representing 0 seconds.


### RC_MODE_SET_ALARM

In this state, the display should blink: "HH.MM" where
"HH" is the current alarm time hours and "MM" is the alarm
time minutes.

The minute hand should move to the appropriate angle representing
the clock time as the user is inputting it. The second hand should
stay fixed at the angle representing 0 seconds.


### RC_MODE_IN_ALARM

In this mode, the alpha display will flash the letters:
"ALRM".

The clock hands should continue to display the time as it updates.


### RC_MODE_CLOCK

Per the assignment specification, the display should update with
hours and minutes, while the clock hands display minutes and seconds.

