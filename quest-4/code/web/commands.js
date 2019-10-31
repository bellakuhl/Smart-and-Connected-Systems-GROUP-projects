const dgram     = require("dgram");
const http      = require("http");
const io        = require("socket.io");
const express   = require("express");

const CRAWLER_IP = "192.168.1.102";
const CRAWLER_PORT = 8080;

const UDP_PORT  = 8080;
//const UDP_HOST  = "192.168.1.108";
const UDP_HOST = "192.168.1.197";

const app = express();
const server =  http.Server(app);
const websocket = io(server);
const udp_socket = dgram.createSocket("udp4");

const CMD_ESC = 0;
const CMD_STEERING = 1;
const CMD_START_AUTO = 2;
const CMD_STOP_AUTO = 3;

const PWM_MIN = 900;
const PWM_NEUTRAL = 1500;
const PWM_MAX = 2400;

function serialize_data(data) {
    var data = new Uint8Array(2);

    data[0] = data.command;
    data[1] = data.value >> 0 & 0xFF;
    data[2] = data.value >> 4 & 0xFF;

    return data;
}

function send_command(command, value) {
    return new Promise(function (resolve, reject) {
        var data = {command: type, value: value};
        udp_socket.send(serialize_data(data), CRAWLER_PORT, CRAWLER_IP, function(error) {
            if (error) {
                reject("Error setting value.");
            }
            else { resolve(value);
            }
        });
    });
}

udp_socket.on("message", function (message, remote) {
    websocket.emit("message", message);
});
udp_socket.bind(UDP_PORT, UDP_HOST);

app.use(express.json());
app.use("/static", express.static("static"));
app.use("/node_modules", express.static("node_modules"));
app.post("/control", function (request, response) {
    const body = request.body;
    const param = body.param;
    const value = body.value;

    if (value < PWM_MIN || value > PWM_MAX) {
        response.status(422).send("Invalid range");
        return;
    }

    if (param == "esc") {
        send_command(CMD_ESC, value).then(function (value) {
            response.status(200).send("Set ESC PWM to " + value);
        }).catch(function (msg) {
            response.status(500).send(msg);
        });
    }
    else if (param == "steering") {
        send_command(CMD_STEERING, value).then(function (value) {
            response.status(200).send("Set Steering PWM to " + value);
        }).catch(function (msg) {
            response.status(500).send(msg);
        });
    }
    else {
        response.status(422).send("Invalid control param.");
    }
});

app.post("/start", function (request) {
    send_command(CMD_START_AUTO, 0).then(function (value) {
        response.status(200).send("Started Autonomous Routine");
    }).catch(function (err) {
        response.status(500).send("Error Starting Autonomous Routine");
    });
});

app.post("/stop", function (request) {
    send_command(CMD_STOP_AUTO, 0).then(function (value) {
        response.status(200).send("Stop command sent.");
    }).catch(function (err) {
        response.status(500).send("Error sending stop command.");
    });
});

app.get("/", function (request, response) {
    response.sendFile("index.html", {root: __dirname});
});

server.listen(8000);

