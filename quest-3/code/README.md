# Wearable Computer with Remote Access

This repository contains the code for running Wearable Computer with Remote Access.


## Firmware

The firmware to run on the Huzzah32 feather board is in the `firmware` folder.  See the
board setup documentation in the [report](../report.md) for instructions on how to setup
the Huzzah32 board.

One the board is wired up, connect the board and run, from the `firmware` folder:

```
idf.py build
idf.py flash
```


## NodeJS Server

All of the code needed to run the webserver is in the `server` directory.

**Setup**

* Make sure NodeJS is installed on your machine
* From the `server` directory: `npm install`


**Running**

* Make sure your ESP32 is flashed with the code in `firmware` and
   connected to your laptop via USB.
* From the `server` folder: `node index.js <dev/port>` where `<dev/port>` is
  the device identifier for the ESP serial monitor (i.e. COM4, /dev/ttyUSB0, etc)
* Open in a browser: http://localhost:8080
