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


### Setup

Prerequisite: NodeJS v10.13

1) `cd web`
2) `npm install`
3) `node server.js`

### Network Setup

If running on the Raspberry Pi, the your IP will be statically assigned to
be: `192.168.1.108`

* UDP Port: 8080
* HTTP Port: 8000
* Websocket Port: 8000

These settings can be changed in [server.js](./web/server.js)

### Deploy

The Raspberry Pi is setup to deploy via git. To update to the latest
code:

0) Push your latest changes and make sure you are connected to the router.
1) `ssh pi@192.168.1.108`
2) `cd Team15-Kuhl-Reeve-Rossi`
3) `git pull`
4) Restart the webserver: `sudo systemctl restart ec444-q3.service`

