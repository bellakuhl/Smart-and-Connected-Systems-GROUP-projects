# Wearable Websever

This folder contains the code for running the web portal for monitoring and control the wearable.

## Setup
**Pre-requisites:**

* NodeJS v10.13

1) Install dependencies: `npm install`
2) Run: `node server.js`

## Network Setup

If running on the Raspberry Pi, the your IP will be statically assigned to
be: `192.168.1.108`

* UDP Port: 8080
* HTTP Port: 8000
* Websocket Port: 8000

These settings can be changed in [servers.js](./server.js)

## Deploy

The Raspberry Pi is setup to deploy via git. To update to the latest
code:

0) Push your latest changes and make sure you are connected to the router.
1) `ssh pi@192.168.1.108`
2) `cd Team15-Kuhl-Reeve-Rossi`
3) `git pull`
4) Restart the webserver: `sudo systemctl restart ec444-q3.service`
