# Retro Clock Console I/O Specification

Console I/O will be the only way for the user to input inforamtion
into the clock. It must enable the user to:

1. Set Time:  Set the clock time
2. Set Alarm: Set the alarm time
3. Toggle Alarm Enabled: Enable or disable the alarm
4. Dismiss an active alarm.
5. Print Help: Prints commands are available. (This is different depending
   on what state the clock is in.)

The mechanism for interfacing with the console will be using the UART skills
developed in skill 08. The implementation for this mechanism should provide
a main function that can be run as a separate task to not interfere with the
time keeping responsibilities.

## State Behaviors

In addition to the descriptions below, the clock should adhere to the following
state machine:

[!Retro Clock States](../images/state_diagram.jpg)

### RC_MODE_NOT_SET

**Availble Commands**

* Set Time


**Behavior**

The console should display the prompt: `Press S to set the clock time`. After the 's'
character the console should change the clock mode to `RC_MODE_SET_TIME`


### RC_MODE_SET_TIME

**Available Commands**

_(None)_

**Behavior:**

In this mode, the user will be able to enter hours and minutes to set the time.
When first entering this mode, the console will prompt for entering hours as such:

`Enter Hour: `

The user should only be allowed to enter ASCII numerals which should be echoed back
as they type. After the user presses the enter key to set the time, the input should
check that the user entered a valid hour between 0 and 23 (using 24hr time format).
If this validation fails, an error message should be printed to the console and
the user should be re-prompted to enter the hour. Once a valid hour has been set,
the clock time hour will be updated and the user will be prompted to enter the minutes:

`Enter Minutes: `

The console should only allow the user to enter ASCII numerals. After the user presses the
enter key to set the time, the input should check that the user entered a valid minute
between 0 and 59. If this validation fails, an error message should be printed to the console
and the user should be re-prompted to enter the minutes.  After a valid minute has been entered,
the clock time minutes will be updated and the console should change the clock mode to
be `RC_MODE_CLOCK`.

**Nice to have:**

If there's time, it would be nice to build way to cancel setting the time.


### RC_MODE_SET_ALARM

This mode behaves exactly as `RC_MODE_SET_TIME`, except the console should
be updating the alarm time instead of the console time. Additionally,
before switching into `RC_MODE_CLOCK`, the console should notifiy the system
that the alarm time has changes so it can appropriately setup the new alarm
if it is enabled.


### RC_MODE_CLOCK

**Available Commands**

* Set Time
* Set Alarm
* Enable Alarm
* Disable Alarm

**Behavior**

When entering this state the first thing the console should do is prompt teh user
with the available commands. Something like this:

```
Clock is running. Available Commands:

Set clock time - Press 'c'
Set alarm time - Press 'a'
Enable alarm   - Press 'e'
Disable alarm  - Press 'd'
```

When the user presses `c` or `a`, the console should correspondingly set the clock mode
to `RC_MODE_SET_TIME` or `RC_MODE_SET_ALARM`. When `e` or `d` is pressed, the alarm
should enabled or disabled and a confirmation printed to the user.


### RC_MODE_IN_ALARM

**Available Commands**

* Dismiss Alarm

**Behavior**

In this mode, the console should prompt: `The alarm is sounding! Type 'dismiss' to stop: `

If the user does not properly type dismiss, the console should display an error message
and reprompt the user to dismiss. Once properly dismissed, the console should call
the function `retro_clock_dismiss_alarm(retro_clock_t *)`.
