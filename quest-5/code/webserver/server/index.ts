import http from "http";
import path from "path";
import express from "express";
import io from "socket.io";
import * as db from "./database";

let App = express();
let HttpServer = new http.Server(App);
let WebSocket = io(HttpServer);

App.use("/static", express.static("static"));
App.use("/node_modules", express.static("node_modules"));
App.use("/client", express.static(path.join(__dirname, "../client")));
App.use(express.json());

interface AuthedRequest extends express.Request {
    user?: db.ISuperUser
}

async function RequireAuth(req: AuthedRequest, resp: express.Response, next: any)
{
    // Basic auth middleware adapted from
    // https://jasonwatmore.com/post/2018/09/24/nodejs-basic-authentication-tutorial-with-example-api
    if (!req.headers.authorization || req.headers.authorization.indexOf('Basic ') === -1) {
        return resp.status(401).json({ message: 'Missing Authorization Header' });
    }

    const base64Credentials =  req.headers.authorization.split(' ')[1];
    const credentials = Buffer.from(base64Credentials, 'base64').toString('ascii');
    const [username, password] = credentials.split(':');
    try {
        let data = await db.superUser.get(username, password);
        if (Array.isArray(data) && Array.length == 1) {
            req.user = data;
            req.
            next();
        }
        else if (data) {
            req.user = data;
            next();
        }
        else {
            resp.status(401).json({ message: 'Invalid Authentication Credentials' });
        }
    }
    catch (err) {
        return resp.status(401).json({ message: 'Invalid Authentication Credentials' });
    }
}


App.get("/fob-access/log", async function (req, resp) {
    let query = req.query;
    let dbQuery: db.IFobAccessQuery = {};

    // Translate the request query to a database query.
    if (query.name) {
        dbQuery.person = query.name;
    }

    if (query.fob_id) {
        dbQuery.fob_id = query.fob_id;
    }

    if (query.loc) {
        dbQuery.loc = query.loc;
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
        return resp.json({message: "Error - since must be earlier than until."});
    }

    try {
        let records = await db.accessLog.get(dbQuery);
        resp.json({access_log: records});
    }
    catch(err) {
        console.error("Error querying database: ", err);
        resp.status(500).json({message: err});
    }
});


// Only an authorized user can see a list of authorized fobs.
App.get("/authorized-fobs", RequireAuth, async function (req, resp) {
    try {
        let fobs = await db.fobs.list({});
        resp.json({fobs: fobs});
    }
    catch(err) {
        console.error("/authorized-fobs ", err);
        resp.status(500).json({message: err});
    }
});


// Only an authorized user can add a new authorized fob.
App.post("/authorized-fobs", RequireAuth, async function (req, resp) {
    let data = req.body;
    if (!data.username || !data.fob_id || !data.fob_code) {
        return resp.status(422).json({message: "username, fob_id and fob_code are required!"});
    }

    try {
        let fobs = await db.fobs.insert(data.username, data.fob_id, data.fob_code);
        resp.send(204).end();
    }
    catch(err) {
        console.error(err);
        resp.status(500).json({message: "Error creating user: " + err});
    }
});


// Only an authorized user (i.e. security hub) can try to authroize a hub
// request.
App.post("/fob-access", RequireAuth, async function (req: AuthedRequest, resp) {
    let data = req.body;
    if (!data.fob_id || !data.fob_code) {
        return resp.status(422).json({message: "fob_id and fob_code are required."});
    }

    try {
        let fob = await db.fobs.access(data.fob_id, data.fob_code);
        let record: db.IFobAccessRecord = {
            fob_id: fob.fob_id,
            hub_id: req.user.username,
            person: fob.username,
            time: new Date().getTime(),
            loc: req.user.loc,
            fob_state: fob.fob_state
        };
        let rec = await db.accessLog.insert(record);
        resp.status(200).json({accessRecord: rec});
    }
    catch(err) {
        console.log("/fob-access error ", err);
        resp.status(403).json({message: "Access unauthorized!"});
    }
});


App.get("/", function (req, resp) {
    resp.sendFile("client/index.html", {root: path.join(__dirname, "..")});
});

async function start(port: number)
{
    await db.init();
    HttpServer.listen(port);
    console.log(`Listening on port ${port}`);
}

if (require.main == module) {
    start(8000);
}
