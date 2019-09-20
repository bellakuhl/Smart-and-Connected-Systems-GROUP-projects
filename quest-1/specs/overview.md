# Retro Alarm Clock

This document outlines the different input and output components of the retro
alarm clock and how they are synchronized with the timekeeping system.


## Hardware Setup

To assemble the clock, connect the alphanumeric display via I2C. Connect the
minutes servo to GPIO 12 and the seconds servo to GPIO 27. The console I/O
is setup to run UART over USB.  Once the peripherals are setup and the chip is
flashed, the servos will rotate to the '0' position and will rotate clockwise
~90° from the starting position. Place the hands on the servos.


## Software Overview

The software for this clock is organized to run the state machine diagrammed below:

![Retro Clock Diagram](../images/state_diagram.jpg)


### Time Keeping

The data managed by the system includes:

* The clock time
* The alarm time
* The current state
* Whether the alarm is enabled or disabled

When in clock mode, the timekeeping using a hardware timer trigger updates.
The full specification is in [Retro Clock Time Keeping Specification](./time_keeping.md)


### Input

The only external input available to a user is through a UART over USB console.
The console allows the user to take the following actions on the clock:

* Set the clock time
* Set the alarm time
* Enable or disable the alarm
* Dismiss an active alarm

See the [Retro Clock Console I/O Specification](./console_io.md) for details.


### Outputs

There are two outputs in this system, the clock hands and the alphanumeric display.
See the [Retro Clock Output Specification](./clock_outputs.md) for details.
