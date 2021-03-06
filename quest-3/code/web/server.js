// Author: Joseph Rossi
// Required Packages
const dgram     = require("dgram");
const http      = require("http");
const express   = require("express");
const path      = require("path");
const io        = require("socket.io");

// Settings
const UDP_PORT  = 8080;
const UDP_HOST  = "192.168.1.108";

const WEARABLE_IP = "192.168.1.102";
const WEARABLE_PORT = 8080;
const WEBSERVER_PORT = 8000;

// Create the server
const app = express();
const server =  http.Server(app);
const websocket = io(server);
const udp_socket = dgram.createSocket("udp4");

// Wearable Communication
const WEARABLE_SETTINGS = {
    battery_sensor_enabled: 1,
    temperature_sensor_enabled: 1,
    step_sensor_enabled: 1,
    alert_now: 0,
    alert_period_seconds: 5
};

function serialize_data(settings) {
    var data = new Uint8Array(8);

    data[0] = settings.battery_sensor_enabled;
    data[1] = settings.temperature_sensor_enabled;
    data[2] = settings.step_sensor_enabled;
    data[3] = settings.alert_now;

    data[4] = (settings.alert_period_seconds >> 0)  & 0xFF;
    data[5] = (settings.alert_period_seconds >> 4)  & 0xFF;
    data[6] = (settings.alert_period_seconds >> 8)  & 0xFF;
    data[7] = (settings.alert_period_seconds >> 12) & 0xFF;

    return data;
}

const wearableUpdateSettings = function (data) {
    udp_socket.send(serialize_data(data), WEARABLE_PORT, WEARABLE_IP, function(error) {
        if (error) { console.error("Error sending message!"); }
        websocket.emit("settings", JSON.stringify(WEARABLE_SETTINGS));
    });
};

udp_socket.on("message", function (message, remote) {
    try {
        const msg = JSON.parse(message);
        websocket.emit("data", msg);
    } catch(e) {
        console.error("Error parsing message: " + e);
    }
});

udp_socket.bind(UDP_PORT, UDP_HOST);

// Web API
app.use("/static", express.static("static"));
app.use("/node_modules", express.static("node_modules"));
app.use(express.json());
app.post("/sensor/:name/enable", function (request, response) {
    const sensor = request.params.name;
    if (sensor === "battery") {
        WEARABLE_SETTINGS.battery_sensor_enabled = 1;
    }
    else if (sensor === "temperature") {
        WEARABLE_SETTINGS.temperature_sensor_enabled = 1;
    }
    else if (sensor === "step") {
        WEARABLE_SETTINGS.step_sensor_enabled = 1;
    }
    else {
        return response.status(404).end();
    }

    wearableUpdateSettings(WEARABLE_SETTINGS);
    return response.json(WEARABLE_SETTINGS);
});

app.post("/sensor/:name/disable", function (request, response) {
    const sensor = request.params.name;

    if (sensor === "battery") {
        WEARABLE_SETTINGS.battery_sensor_enabled = 0;
    }
    else if (sensor === "temperature") {
        WEARABLE_SETTINGS.temperature_sensor_enabled = 0;
    }
    else if (sensor == "step") {
        WEARABLE_SETTINGS.step_sensor_enabled = 0;
    }
    else {
        return response.status(404).end();
    }

    wearableUpdateSettings(WEARABLE_SETTINGS);
    return response.json(WEARABLE_SETTINGS);
});

app.post("/alert/trigger", function (request, response) {
    WEARABLE_SETTINGS.alert_now ^= 1;
    wearableUpdateSettings(WEARABLE_SETTINGS);
    return response.json(WEARABLE_SETTINGS);
});

app.post("/alert/schedule", function (request, response) {
    const json = request.body;
    const period_sec = parseInt(json.alert_period_seconds, 10);
    if (!isNaN(period_sec)) {
        WEARABLE_SETTINGS.alert_period_seconds = period_sec;
        wearableUpdateSettings(WEARABLE_SETTINGS);
        return response.json(WEARABLE_SETTINGS);
    }
    else {
        return response.status(422).send("Invalid period: " + json.alert_period_seconds);
    }
});

app.get("/settings", function (reqeust, response) {
    return response.json(WEARABLE_SETTINGS);
});

app.get("/", function (request, response) {
    response.sendFile("templates/index.html", {root: __dirname});
});

// Simulation
var step_count = 0;
function genMessage() {
    step_count += Math.round(Math.random() * 2);
    var msg = {
        battery_volts: 3.3 + Math.random() * 2,
        temperature_degc: 15 + Math.random() * 2,
        steps: step_count
    };

    if (!WEARABLE_SETTINGS.battery_sensor_enabled) {
        msg.battery_volts = -1;
    }
    if (!WEARABLE_SETTINGS.temperature_sensor_enabled) {
        msg.battery_volts = -100;
    }
    if (!WEARABLE_SETTINGS.step_sensor_enabled) {
        msg.steps = -1;
    }

    websocket.emit("data", msg);
}

function startSimulation() {
    setTimeout(function () {
        genMessage();
        startSimulation();
    }, 300);
}

//startSimulation();
server.listen(WEBSERVER_PORT);
