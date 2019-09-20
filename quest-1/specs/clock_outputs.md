# Retro Clock Outputs Specification

This document outlines the behaviors of the alphanumeric display and
the clock hands throughout the different states the clock can be in. Since they
are solely output mechanisms, they are responsible for implementing a function
that adheres to the `clock_update_callback` function prototype, and should be
registered as callbacks during the app's initialization.

Since the servos used for the hands are not continuous, they should behave
line [impact sprinklers](https://en.wikipedia.org/wiki/Impact_sprinkler).
The hands should start at the most counter clockwise position possible and
rotate 100% of the servo's range in the clockwise direction, corresponding
to 0 and 59 minutes or seconds depending on which hands. There should be a
rapid return cycle when resetting to 0.


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

In this mode, the alpha display will flash the letters: "ALRM"

The clock hands should continue to display the time as it updates.


### RC_MODE_CLOCK

Per the project specification, the display should update with
hours and minutes, while the clock hands rotate to indicate minutes
and seconds.
