const SerialPort = require("serialport");
const EventEmitter = require("events");

// Used to emit Readings as sensors report data
const DataEmitter = new EventEmitter();

/*
Types:

interface SensorValue {
    name: string,
    units: string,
    value: float
}

interface Reading {
    timestame: string // UTC Date String
    sensorValues: SensorValue[]
}
 */

/**
 * Converts serial data received into a javascript object
 *
 * @params The serial data to transform. This should be one line of sensor
 * readings
 *
 * @returns Reading
 */
function parseData(data) {
    var sensorsValues = [];

    // TODO: Parse data string into sensor values

    return {
        timestamp: (new Date()).toISOString(),
        sensors: sensorValues
    };
}

module.exports = {
    start: function (device, baudRate) {
        console.log("Start monitor with: ", dev, baudRate);
        const esp32 = new SerialPort(device, {baudRate: baudRate});


        esp32.on("data", function (data) {
            DataEmitter.emit("data", parseReading(data));
        });
    },
    on: DataEmitter.on.bind(module.exports)
}


if (require.main == module) 
{
    var dev = "/dev/ttyUSB0";
    var baudRate = 115200;
    var args = process.argv.slice();

    if (args.length > 2) {
        console.log("Dev set");
        dev = args[2];
    }

    if (args.length >= 3) {
        baudRate = parseInt(args[3], 10);
    }

    module.exports.start(dev, baudRate);
}

