# Retro Clock Time Keeping Specification 

## Keeping Time

Time is keep using one of the hardware timers available on the ESP32. The timer's 
counter value can be set and translated into seconds using the following formula:

```
SECONDS = <TIME SCALE> * <COUNTER VALUE>
```

Where `<TIME SCALE>` is `80e6/<DIVIDED VALUE>`. Then, seconds is used to keep track 
of the current time. Zero seconds indicates midnight while 86399 seconds (3600 * 24),
indicates 23:59:59.  Once the timer is started, the clock queries the timer at some 
interval to update it's stored clock time. This task should reset the timer's counter 
to 0 when the clock reaches midnight.


### RC_MODE_NOT_SET

The timer should be initialized and ready to go but not running.


### RC_MODE_SET_TIME

The timer should be paused and the alarm disabled. The Console I/O will update
the time via `retro_clock_set_clock_time` when the user has finished.


### RC_MODE_CLOCK

The timer should running paused and the alarm state set appropriately as described
in [Triggering Alarms](#triggering-alarms).


### RC_MODE_SET_ALARM

The timer should be running and the alarm should be disabled as not to go off
while setting the alarm.


### RC_MODE_IN_ALARM

The timer should continue running and the timer alarm should be disabled so the 
alarm does not constantly get triggered. The `clock_mode` variable will be set
in the interrupt when the alarm is triggered and will remain in alarm until the 
user dismisses it.


## Triggering Alarms

Alarm time can be set by the user (see the [Console I/O Spec](./console_io.md)) and should 
be enabled when the user has enabled the alarm and the timer counter value is LESS than
what the translated counter value for the alarm time should be. If the counter value of the
timer is greater than the alarm value, the alarm will be triggered.


## Inputs 

The only input to the system is the console I/O over UART. This peripheral is responsible
for giving a user the ability to take certain actions on the clock. To allow this, the 
clock allows the following actions via public API methods: 

* Change the clock's mode
* Set the clock's time
* Set the alarm time
* Enable the alarm's 
* Dismiss the alarm

See [retro_clock.h](../code/main/retro_clock.h) for the specific APIs.


## Outputs

The clock is responsible for updating any registered callbacks when the time is updated,
or the clock's mode changes, so that all outputs can react accordingly.  In a dedicated 
task, the clock updates all registered outputs every second with its current state (including 
the time, alarm time, etc.). 

See [retro_clock.h](../code/main/retro_clock.h) for more information about the callback API.

> The thinking behind this being a separate task is that one of the callbacks could take
> too long to run (i.e. the servos), delaying any subsequent tasks from updating in a timely
> manner. Perhaps a better mechanism would be to use queues to notify outputs when things. 
> change. Next time!


