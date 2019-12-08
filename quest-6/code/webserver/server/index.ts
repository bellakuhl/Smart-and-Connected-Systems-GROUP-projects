/* Isabella Kuhl, Laura Reeve, Joseph Rossi
* Module providing HTTP API
*/
import http from "http";
import path from "path";
import express, { response } from "express";
import io from "socket.io";
import * as db from "./database";
import dgram from "dgram";

const App = express();
const HttpServer = new http.Server(App);
const WebSocket = io(HttpServer);
const UDP_Socket = dgram.createSocket("udp4");

App.use("/static", express.static("static"));
App.use("/node_modules", express.static("node_modules"));
App.use("/client", express.static(path.join(__dirname, "../client")));
App.use("/qrcodes", express.static("/Users/jrossi/Downloads/qrcodes"));
App.use(express.json());

/**
 * Crawler Event API
 */
interface AuthedRequest extends express.Request {
    user?: db.ISuperUser
}

enum WEBSOCKET_EVENT {
    CRAWLER_EVENT = "crawler-event",
    CRAWLER_LOG = "crawler-log"
};

App.get("/crawler-event", async function (req, resp) {
    let query = req.query;
    let dbQuery: db.ICrawlerEventQuery = {};

    // Translate the request query to a database query.
    if (query.crawler_id) {
        dbQuery.crawler_id = query.crawler_id;
    }

    // Time range query.
    if (query.since) {
        dbQuery.time = dbQuery.time || {};
        dbQuery.time['$gte'] = query.since;
    }

    if (query.until) {
        dbQuery.time = dbQuery.time || {};
        dbQuery.time['$lte'] = query.until;
    }

    if (query.since && query.until && query.since > query.until) {
        return resp.status(422).json({message: "Error - since must be earlier than until."});
    }

    try {
        let records = await db.crawlerEvent.query(dbQuery);
        resp.json({events: records});
    }
    catch(err) {
        console.error("Error querying database: ", err);
        resp.status(500).json({message: err});
    }
});

// Only an authorized crawler can log events.
App.post("/crawler-event", async function (req: AuthedRequest, resp) {
    let data = req.body;
    if (!data.beacon_id || data.split_time == undefined) {
        console.log(`Invalid request body: ${data}`);
        return resp.status(422).json({message: "beacon_id and split_time are required"});
    }

    try {
        let record: db.ICrawlerEventRecord = {
            crawler_id: "crawler15",
            time: new Date().getTime(),
            event: `Beacon: ${data.beacon_id}`,
            split_time: data.split_time
        };

        console.log(`Inserting split time: ${JSON.stringify(record)}`);
        let rec = await db.crawlerEvent.insert(record);
        resp.status(200).json();
        WebSocket.emit(WEBSOCKET_EVENT.CRAWLER_EVENT, JSON.stringify({record: rec}));
    }
    catch(err) {
        console.log(`Error: ${req.path} - ${err}`);
        resp.status(403).json({message: "Access unauthorized!"});
    }
});


/* Crawler Log Messages */
UDP_Socket.on("message", function (message, remote) {
    WebSocket.emit(WEBSOCKET_EVENT.CRAWLER_LOG, message.toString());
});


/*
    Crawler Control Endpoints
*/
interface ICrawlerCommand {
    command: number;
    value: number;
}

const CRAWLER_IP = "192.168.1.102";
const CRAWLER_PORT = 9000;

export enum CrawlerCommand {
    CMD_ESC = 0,
    CMD_STEERING = 1,
    CMD_START_AUTO = 2,
    CMD_STOP_AUTO = 3,
    CMD_CALIBRATE = 4,
    CMD_GREEN_LIGHT = 5
};


function serialize_data(data: ICrawlerCommand) {
    var bits = new Uint8Array(3);
    bits[0] = data.command;
    bits[1] = data.value >> 0 & 0xFF;
    bits[2] = data.value >> 8 & 0xFF;
    return bits;
}

const PWM_MIN = 900;
const PWM_NEUTRAL = 1500;
const PWM_MAX = 2400;

let LAST_ESC_VALUE = PWM_NEUTRAL;
let LAST_STEER_VALUE = PWM_NEUTRAL;

function send_command(command: CrawlerCommand, value: number) {
    var data = {command: command, value: value};
    return new Promise(function (resolve, reject) {
        var data = {command: command, value: value};
        UDP_Socket.send(serialize_data(data), CRAWLER_PORT, CRAWLER_IP, function(error: Error) {
            if (error) {
                reject("Error setting value.");
            }
            else {
                if (command == CrawlerCommand.CMD_ESC) {
                    LAST_ESC_VALUE = value;
                }

                if (command == CrawlerCommand.CMD_STEERING) {
                    LAST_STEER_VALUE = value;
                }

                resolve(value);
            }
        });
    });
}

App.get("/crawler/state", function (request, response) {
    response.status(200).json({
        esc: LAST_ESC_VALUE,
        steering: LAST_STEER_VALUE
    });
});

App.post("/crawler-command/start-auto", function (request, response) {
    send_command(CrawlerCommand.CMD_START_AUTO, 0).then(function (value) {
        response.status(200).send("Started Autonomous Routine");
    }).catch(function (err) {
        response.status(500).send("Error Starting Autonomous Routine");
    });
});


App.post("/crawler-command/stop", function (request, response) {
    send_command(CrawlerCommand.CMD_STOP_AUTO, 0).then(function (value) {
        response.status(200).send("Stop command sent.");
    }).catch(function (err) {
        response.status(500).send("Error sending stop command.");
    });
});

App.post("/crawler-command/green-light", function (request, response) {
    send_command(CrawlerCommand.CMD_GREEN_LIGHT, 0).then(function (value) {
        response.status(200).send("Stop command sent.");
    }).catch(function (err) {
        response.status(500).send("Error sending stop command.");
    });
});

App.post("/crawler-command/control", function (request, response) {
    const body = request.body;
    const param = body.param;
    const value = body.value;

    if (value < PWM_MIN || value > PWM_MAX) {
        response.status(422).send("Invalid range");
        return;
    }

    if (param == "esc") {
        send_command(CrawlerCommand.CMD_ESC, value).then(function (value) {
            response.status(200).send("Set ESC PWM to " + value);
        }).catch(function (msg) {
            response.status(500).send("Error setting esc");
        });
    }
    else if (param == "steering") {
        send_command(CrawlerCommand.CMD_STEERING, value).then(function (value) {
            response.status(200).send("Set Steering PWM to " + value);
        }).catch(function (msg) {
            response.status(500).send(msg);
        });
    }
    else {
        response.status(422).send("Invalid control param.");
    }
});

// QR Code Scanner
import Jimp from "jimp";
import * as fs from "fs";
const QRCode = require("qrcode-reader");

App.post("/scan-qr-code", async function (req, resp) {

    // let filepath = "/Users/jrossi/Downloads/qrcodes/13.jpg";
    let filepath = "/home/pi/webcam/webcam-image.jpg";

    try {
        let image = await Jimp.read(fs.readFileSync(filepath));
        let qr =new QRCode();
        qr.callback = function (err: string, value: any) {
            if (err) {
                console.error("Error reading qr code: ", err);
                resp.status(500).send({message: err});
                return;
            }
            console.log(value);
            resp.status(200).json(value);
        };
        qr.decode(image.bitmap);
    } catch(err) {
        console.error("Error parsing image: ", err);
        resp.status(500).send({message: err});
    }
});

// Index .html
App.get("/", function (req, resp) {
    resp.sendFile("client/index.html", {root: path.join(__dirname, "..")});
});


async function start(http_port: number, udp_port: number, udp_host: string)
{
    await db.init();
    HttpServer.listen(http_port);
    console.log(`Listening on for http messages on port ${http_port}`);

    UDP_Socket.bind(udp_port, udp_host)
    console.log(`Listening on for udp messages on ${udp_host}:${udp_port}`);
}

if (require.main == module) {
    let args = process.argv.slice(2);

    let http_port = 8000;
    if (args.length > 0) {
        http_port = parseInt(args[0], 10);
        if (!http_port) throw new Error(`Invalid http_port argument: ${http_port}`);
    }

    let udp_port = 9000;
    if (args.length > 1) {
        udp_port = parseInt(args[1], 10);
        if (!udp_port) throw new Error(`Invalid udp_port argument: ${udp_port}`);
    }

    let udp_host = "0.0.0.0";
    if (args.length > 2) {
        udp_host = args[2].trim();
    }

    start(http_port, udp_port, udp_host);
}
