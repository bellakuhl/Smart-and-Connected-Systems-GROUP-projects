const http = require("http");
const url = require("url");
const io = require("socket.io");
const monitor = require("./monitor");
const fs = require("fs").promises;

const mimeMap = {
    ".js": "application/javascript",
    ".html": "text/html"
};

const contentType = function (path) {
    const match = /^.*(\..+)$/.exec(path);
    if (!match) return "text/plain";
    const extension = match[1];
    return mimeMap[extension] || "text/plain";
};

const server = http.createServer(function (req, resp) {
    var path = url.parse(req.url).path;
    if (!path || path == "/") {
        path = "/index.html";
    }
    
    const file = path.substr(1);
    fs.readFile(file, function (err, data) {
        if (err) {
            resp.writeHead(404, {"Content-Type": "text/plain"});
            resp.write("Not Found");
        }
        else {
            const type = contentType(file);
            resp.writeHead(200, {"Content-Type": type});
            resp.write(data);
        }
        resp.end(); 
    });
});

const websocket = io(server);
let COLUMNS = ["timestamp", "battery", "rangefinder", "ultrasonic", "thermistor"];
function appendRow(fd, data) {
    let row = [data.timestamp];

    data.sensors.forEach(function (sensor) {
        let idx = COLUMNS.indexOf(sensor.name);
        if (idx < 0) {
            console.error("Unknown column: ", sensor.name);
            return;
        }

        row[idx] = sensor.value
    });

    fs.appendFile(fd, row.join(",") + "\n");
}

function start(devicePath) {
    const logFilename = "sensors.csv";

    return fs.stat(logFilename).catch(function () {
        // Write the csv header if the file doesn't exist 
        return fs.writeFile(logFilename, COLUMNS.join(",") + "\n");
    }).then(function () {
        websocket.on("connection", function (socket) {
            console.log("Websocket client connected.");
        });

        monitor.on("data", function (reading) {
            websocket.emit("data", reading);
            appendRow(logFilename, reading);
        });

        monitor.start(devicePath, 115200);
        server.listen(8080);
    });
}

if (require.main == module) {
    var args = process.argv.slice();
    if (args.length == 2) {
        throw new Error("Must provide serial port to connect to: /dev/ttyUSB0, COM4, etc");
    }

    const dev = args[2];
    start(dev).catch(function (err) {
        console.error("Error starting webserver: ", err);
    });
}

