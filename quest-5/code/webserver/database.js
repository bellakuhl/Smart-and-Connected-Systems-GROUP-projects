const path = require('path');
const crypto = require("crypto");
const fs = require('fs').promises;

const tingodb = require('tingodb');

const Engine = tingodb();
const DATABASE_FOLDER = path.join(__dirname, "fob.db");
const FOB_ACCESS_TABLE = "FobAccessLog";
const DB = new Engine.Db(DATABASE_FOLDER, {});
const COLLECTION_NAMES = ['FobAccessLog', "AuthorizedFob", "SuperUser"];
const collections = {};

// Super simple hashing function so sensitive data
// stored on disk is not in plain text.
function hashValue(string) {
    return crypto.createHmac("sha256", "T15-super-secret")
                        .update(string.toString())
                        .digest("hex");
}


async function init()
{
    try {
        await fs.access(DATABASE_FOLDER);
    }
    catch (err) {
        if (err.code == 'ENOENT') {
            await fs.mkdir(DATABASE_FOLDER);
        }
    }
    
    for (let collection of COLLECTION_NAMES) {
        collections[collection] = DB.collection(collection);
    }
}


function logFobAccess(record)
{
    return new Promise(function (resolve, reject) {
        collections['FobAccessLog'].insert([record], function (err, data) {
            if (err) {
                reject(err);
            }
            else {
                resolve(data[0]);
            }
        });
    });
}

function fobAccess(fob_id, fob_code) 
{
    fob_code = hashValue(fob_code);
    return new Promise(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id, fob_code: fob_code}).toArray(function (err, data) {
            if (err) {
                return reject(err);
            }

            if (data.length != 1) {
                return reject("Unauthorized");
            }

            resolve({username: data[0].username, fob_id: data[0].fob_id});
        });
    });
}


function queryFobAccessLog(query) 
{
    return new Promise(function (resolve, reject) {
        collections['FobAccessLog'].find(query).toArray(function (err, data) {
            if (err) {
                reject(err);
            }
            else {
                resolve(data);
            }
        });
    });
}


function listFobs(query) 
{
    return new Promise(function (resolve, reject) {
        collections.AuthorizedFob.find(query).toArray(function (err, data) {
            if (err) {
                return reject(err);
            }
            else {
                resolve(data.map(function (data) {
                    // only return fob id, codes are secret.
                    return {username: data.username, fob_id: data.fob_id};
                }));
            }
        });
    });
}


function getFob(fob_id) 
{
    return new Promise(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id}, function (err, data) {
            if (err) {
                return reject(err);
            }

            resolve(data[0]);
        });
    });
}


function insertFob(username, fob_id, fob_code)
{
    fob_code = hashValue(fob_code);
    return new Promise(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id}).toArray(function (err, data) {
            if (err) return reject(err);
            if (data.length > 0) return reject("Fob ID already exists.");

            collections.AuthorizedFob.insert([
                {username: username, fob_id: fob_id, fob_code: fob_code}
            ], function (err, data) {
                if (err) {
                    return reject(err);
                }

                resolve(data[0]);
            });
        })
    });
}


function insertSuperUser(username, password, loc) 
{
    password = hashValue(password);
    return new Promise(function (resolve, reject) {
        collections['SuperUser'].insert([
            {username: username, password: password, loc: loc}
        ], function (err, data) {
            if (err) reject(err)
            else resolve();
        })
    });
}


function getSuperUser(username, password) 
{
    password = hashValue(password);
    return new Promise(function (resolve, reject) {
        collections['SuperUser'].find({username, password}).toArray(function (err, data) {
            if (err) reject(err);
            else resolve(data[0]);
        });
    });
}

exports.init = init;
exports.accessLog = { get: queryFobAccessLog, insert: logFobAccess};
exports.superUser = {get: getSuperUser, insert: insertSuperUser};
exports.fobs = {list: listFobs, get: getFob, insert: insertFob, access: fobAccess};

if (require.main == module) {
    init();
}

