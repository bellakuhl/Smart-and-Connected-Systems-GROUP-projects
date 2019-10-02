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
 * Transforms output of serial reading into the data format needed
 * by the client. This function may raise an exception if the data
 * provided is not a JSON string. The caller is responsible for handling
 * any errors.
 *
 * @params The serial data to transform. This should be one line of sensor
 * readings
 *
 * @returns Reading
 */
function parseData(data) {
    var json = JSON.parse(data); 
    var sensorValues = [
        {"name": "thermistor", "units": "degC", "value": json.thermistor_degc},
        {"name": "battery", "units": "volts", "value": json.battery_volts},
        {"name": "ultrasonic", "units": "m", "value": json.ultrasonic_m},
        {"name": "rangefinder", "units": "m", "value": json.rangefinder_m}
    ];

    return {
        timestamp: (new Date()).toISOString(),
        sensors: sensorValues
    };
}

module.exports = {
    /**
     * Start monitoring the serial device.
     *
     * @param dev  The serial device to monitor
     * @param baudRate The baudRate of the serial device
     */
    start: function (device, baudRate) {
        console.info("Start monitor with: ", device, baudRate);
        const esp32 = new SerialPort(device, {baudRate: baudRate});
        esp32.on("data", function (data) {
            try {
                DataEmitter.emit("data", parseData(data));
            } catch (err) {
                console.error("Error parsing data: ", err);
            }
        });
    },
    /**
     * Register event handlers to be notified of 'data'
     *
     */
    on: DataEmitter.on.bind(DataEmitter)
}


// If invoking this script from the CLI directly
if (require.main == module) {
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

