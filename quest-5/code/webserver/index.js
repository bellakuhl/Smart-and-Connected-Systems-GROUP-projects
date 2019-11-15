const http = require("http");
const express = require("express");
const io = require("socket.io");
const db = require("./database");

// Init
let App = express();
let HttpServer = http.Server(App);
let Websocket = io(HttpServer);

App.use("/static", express.static("static"));
App.use("/node_modules", express.static("node_modules"));
App.use(express.json());

// Only allow an authorized user to add fobs
function SuperUserOnly (req, resp, next) {
    // Basic auth middleware adapted from 
    // https://jasonwatmore.com/post/2018/09/24/nodejs-basic-authentication-tutorial-with-example-api
    if (!req.headers.authorization || req.headers.authorization.indexOf('Basic ') === -1) {
        return resp.status(401).json({ message: 'Missing Authorization Header' });
    }

    const base64Credentials =  req.headers.authorization.split(' ')[1];
    const credentials = Buffer.from(base64Credentials, 'base64').toString('ascii');
    const [username, password] = credentials.split(':');
    db.superUser.get(username, password).then(function (data) {
        if (Array.isArray(data) && Array.length == 1) {
            req.user = username;
            next();
        }
        else if (data) {
            req.user = data;
            next();
        }
        else {
            resp.status(401).json({ message: 'Invalid Authentication Credentials' });
        }
    }).catch(function (err) {
        return resp.status(401).json({ message: 'Invalid Authentication Credentials' });
    });
}

App.get("/fob-access/log", function (req, resp) {
    let query = req.query;
    let dbQuery = {};
    
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

    db.accessLog.get(dbQuery).
        then(function (records) {
            resp.json({access_log: records});
        })
        .catch(function (error) {
            console.error("Error querying database: ", error);
            resp.status(500).json({message: error});
        });
});


// Only an authorized user can see a list of authorized fobs.
App.get("/authorized-fobs", SuperUserOnly, function (req, resp) {
    db.fobs.list({}).then(function (fobs) {
        resp.json({fobs: fobs});
    }).catch(function (err) {
        console.error("/authorized-fobs ", err);
        resp.status(500).json({message: err});
    })
});


// Only an authorized user can add a new authorized fob.
App.post("/authorized-fobs", SuperUserOnly, function (req, resp) {
    let data = req.body;
    if (!data.username || !data.fob_id || !data.fob_code) {
        return resp.status(422).json({message: "username, fob_id and fob_code are required!"});
    }

    db.fobs.insert(data.username, data.fob_id, data.fob_code)
        .then(function (){
            resp.send(204).end();
        })
        .catch(function (err) {
            console.error(err);
            resp.status(500).json({message: "Error creating user: " + err});
        })

});


// Only an authorized user (i.e. security hub) can try to authroize a hub
// request.
App.post("/fob-access", SuperUserOnly, function (req, resp) {
    let data = req.body;
    if (!data.fob_id || !data.fob_code) {
        return resp.status(422).json({message: "fob_id and fob_code are required."});
    }

    db.fobs.access(data.fob_id, data.fob_code)
        .then(function (data) {
            let record = {
                fob_id: data.fob_id,
                hub_id: req.user.username,
                person: data.username,
                time: new Date().getTime(),
                loc: req.user.loc
            };
            return db.accessLog.insert(record);
        })
        .then(function (rec) {
            resp.status(200).json({accessRecord: rec});
        })
        .catch(function (err) {
            resp.status(401).json({message: "Access unauthorized!"});
        });
});


App.get("/", function (req, resp) {
    resp.sendFile("static/index.html", {root: __dirname});
});

async function start(port)
{
    await db.init(); 
    HttpServer.listen(port);
}


if (require.main == module) {
    start(8000);
}
