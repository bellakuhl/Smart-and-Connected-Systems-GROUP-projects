# Crawler!!

To run the ESP32 firmware:

1) Connect the circuit as depicted in the [circuit diagram](../images/circuit.jpg).
2) Using idftools: `cd firmware && idf.py build && idf.py flash`

## Webserver

**Dependencies**
NodeJS

1) `cd web`
2) `npm install`
3) `node commands.js`
4) Open http://127.0.0.1:8000

> If not using the Group 15 router, you will need to update the destination IP address
> in the firmware and the ESP's IP address in commands.js

