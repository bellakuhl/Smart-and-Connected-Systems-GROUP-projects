# Quest Name
Authors: Isabella Kuhl, Laura Reeves, Joseph Rossi

2019-11-20

## Summary

In this quest, we implemented a device that acts as a security key fob. When a button on the fob is pressed, the user's unique ID and code is sent to a security hub. The hub then checks that the ID and code match one saved in the system and logs this to a server hosted on a Raspberry Pi.

## Evaluation Criteria



## Solution Design

There are three key fobs that we built, each with its own user ID and access code. Each fob transmits a hex encoded code at the press of a button while also continuously receiving a LOCKED or UNLOCKED signal from the hub. The ESP RMT is used to produce a 38kHz signal that is then ANDed via a motor driver with the signal from the transmitter. This is the decoded by the IR receiver and the message is sent to the ESP. If the UNLOCKED signal is received by the fob (sent by the hub), then the green LED on the fob lights up for five seconds indicating it has unlocked and then locks again, indicated by the red LED turning on.

[security hub]

[server]


## Sketches and Photos
<center><img src="./images/example.png" width="70%" /></center>  
<center> </center>


## Supporting Artifacts
- [Link to repo]()
- [Link to video demo]()


## References

* We referenced Espessif's [RMT guide](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/rmt.html#transmit-data) to use the RMT.
* We references Espressif's [UART guide](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/uart.html#_CPPv415uart_read_bytes11uart_port_tP7uint8_t8uint32_t10TickType_t) to use the UART.
-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private
