# Quest Name
Authors: Isabella Kuhl, Laura Reeve, Joseph Rossi

2019-10-01

## Summary

For this quest we divided the work into two segments, the hardware layout and firmware 
development running on the ESP32 and the webserver/live plotting application for graphing
the readings. Bella and Laura paired up to work on the hardware layout and firmware
development and Joe wrote the code to read the console output of the sensor and graph
it on a webpage.

The ESP32 reads sensor data every two seconds, and writes a JSON string to the console,
which the NodeJS server reads through the serial interface and forwards the data along
to the webpage which live-plots the data.


## Evaluation Criteria



## Solution Design

The data flow for this implementation is depicted below:

<center><img src="./images/dataflow-sequence-diagram.png" width="80%"></center>


### Hardware

> TODO: Hardware Documentation


### Firmware

> TODO: Firmware Report 


### Software 

The webserver for this project was written in NodeJS. It provides a single webpage
(index.html) that loads the scripts necessary to plot the data in real time.

The server uses the `serialport` npm package to read the serial
output from the ESP32. Since the ESP32 is outputting the data as JSON strings,
it was trivial to javascript's built-in function `JSON.parse` to read the data. 
The sensor data  is then transform the data into a data structure  suitable for the
client code.

> Note: The webpage runs on port 8080 by default: http://localhost:8080

The webpage, on load, opens up a websocket back to the webserver and listens for the
`data` event. When received it expects data to be published in the following format:

```
interface SensorValue {
    name: string,
    units: string,
    value: float
}

interface SensorReading {
    timestamp: string // UTC Date String
    sensorValues: SensorValue[]
}
```

> Note: The webserver is responsible for time-stamping the data when it was received, the
> client expects the timestamp value to be filled out.  If the payload received is not in this format, 
> the message will be ignored. 

When a message is received, the JS running in the browser  adds the data to the appropriate 
chart based on the `name` of the `SensorValue.`

There are three charts on the page. Since the Ultrasonic and IR Rangefinder sensors are both
measuring distance, they are plotted against each other on the same chart. The battery and 
thermistor data are plotted on separately labeled charts. 

> The units field was intended to provide a dynamic way of grouping all data that shared
> the same units onto a single chart, rather than hard-coding which chart gets which data.
> The implementation details behind this got a bit unwieldy, but the field remains.

The webpage can also render static CSV files of data. When the webserver starts up, 
it starts recording data to a file named `sensors.csv`.  If, for whatever reason, 
you want to view the historical content of `sensors.csv`, simply add `?csv=sensors.csv` 
to the URL and the page will render with the contents of the file. 

> Live streaming **will not** occur when viewing the historical data.


## Sketches and Photos

## Investigative Question


### Ultrasonic Sensor

Based on the [datasheet][1] the ultrsonic sensor will update its 
readings every 50 milliseconds, so should not be sampled faster 
than 20 Hz.


### Infrared Distance Sensor

The timing chart in the [datasheet][2]
(page 4) shows that the diststance sensor has a measurement period
of approximately 38.3ms ± 9.6ms, so the maximum  sampling rate should
be between 21Hz and 34Hz.

Additionally, the timing chart indicates shows the first measurement
as being unuseable, so the initial sample should be delayed 43ms ± 9.6ms 
after being powered on.


### Battery & Thermistor

The batter and thermistor readings are both limited by the sample rate
of the ADCs provided by the ESP32. From page 23 of the [ESP32 
datasheet][3], The maximum sampling rate of the inputs on the RTC 
Controller are 20ksps inditcating a maximum sampling rate of 20kHz.

[1]: https://www.maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf
[2]: https://www.sparkfun.com/datasheets/Sensors/Infrared/gp2y0a02yk_e.pdf
[3]: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf#page=30


## Supporting Artifacts
- [Link to repo]()
- [Link to video demo]()


## References

* [MF52 Thermistor Datasheet](https://www.eaa.net.au/PDF/Hitech/MF52type.pdf)
* [MaxSonar Ez Datasheet](https://www.maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf)
* [Sharp Rangefinder Datasheet](https://www.sparkfun.com/datasheets/Sensors/Infrared/gp2y0a02yk_e.pdf)


-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private
