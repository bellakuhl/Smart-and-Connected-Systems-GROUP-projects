/* Isabella Kuhl, Laura Reeve, Joseph Rossi
* Module for database model interactions.
*/
import path from "path";
import crypto from "crypto";
import {promises as fs} from "fs";

const tingodb = require("tingodb");

const Engine = tingodb();
const DATABASE_FOLDER = path.join(__dirname, "fob.db");
const FOB_ACCESS_TABLE = "FobAccessLog";
const DB = new Engine.Db(DATABASE_FOLDER, {});
const COLLECTION_NAMES = ['FobAccessLog', "AuthorizedFob", "SuperUser"];
export const collections: {[key: string]: any} = {};

export interface IFob {
    username: string;
    fob_id: number;
    fob_code: string;
}

export interface ISanitizedFob {
    username: string;
    fob_id: number;
}

export interface IFobAccessRecord {
    time: number;
    fob_id: number;
    person: string;
    loc: string;
    hub_id: string;
}

export interface IFobAccessQuery {
    person?: any;
    fob_id?: any;
    loc?: any;
    since?: any;
    until?: any;
    time?: any;
}

export interface IFobQuery {
    fob_id?: string;
    person?: string;
}

export interface ISuperUser {
    username: string;
    password: string;
    loc: string;
}


// Super simple hashing function so sensitive data
// stored on disk is not in plain text.
function hashValue(string: string) {
    return crypto.createHmac("sha256", "T15-super-secret")
                        .update(string.toString())
                        .digest("hex");
}


export async function init()
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

function logFobAccess(record: IFobAccessRecord)
{
    return new Promise<IFobAccessRecord>(function (resolve, reject) {
        collections['FobAccessLog'].insert([record], function (err: Error|null, data: IFobAccessRecord[]) {
            if (err) {
                reject(err);
            }
            else {
                resolve(data[0]);
            }
        });
    });
}

type cbError = Error|null;

async function fobAccess(fob_id: number, fob_code: number)
{
    let hashedFobCode =  hashValue(fob_code.toString());
    return new Promise<ISanitizedFob>(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id, fob_code: hashedFobCode})
            .toArray(function (err: cbError, data: IFob[]) {
                if (err) {
                    console.error(`FobAccess Error: ${err}`);
                    return reject(err);
                }

                if (data.length != 1) {
                    console.error(`FobAccess: Too many entries`);
                    return reject("Unauthorized");
                }

                resolve(sanitizeFob(data[0]));
            });
    });
}


function queryFobAccessLog(query: IFobAccessQuery)
{
    return new Promise<IFobAccessRecord[]>(function (resolve, reject) {
        collections['FobAccessLog'].find(query).toArray(function (err: cbError, data: IFobAccessRecord[]) {
            if (err) {
                reject(err);
            }
            else {
                resolve(data);
            }
        });
    });
}

function sanitizeFob(fob: IFob) : ISanitizedFob {
    return Object.keys(fob).reduce((acc: any, key) : any => {
        if (key !== "fob_code") {
            acc[key] = (<any>fob)[key];
        }

        return acc;
    }, {});
}


function listFobs(query: IFobQuery)
{
    if ((<any>query).fob_code) delete (<any>query).fob_code; // Don't allow queries by code.
    return new Promise<ISanitizedFob[]>(function (resolve, reject) {
        collections.AuthorizedFob.find(query).toArray(function (err: cbError, data: IFob[]) {
            if (err) {
                return reject(err);
            }
            else {
                resolve(data.map(sanitizeFob));
            }
        });
    });
}


function getFob(fob_id: number)
{
    return new Promise<ISanitizedFob|null>(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id}).toArray(function (err: cbError, data: IFob[]) {
            if (err) {
                return reject(err);
            }

            if (data.length > 1) {
                return reject(`Got multiple results for fob: ${fob_id}`);
            }

            if (data.length == 0) {
                return resolve(null);
            }

            console.log(data);
            resolve(sanitizeFob(data[0]));
        });
    });
}

function insertFob(username: string, fob_id: number, fob_code: number, )
{
    let hashedFobCode = hashValue(fob_code.toString());
    return new Promise<ISanitizedFob>(function (resolve, reject) {
        collections.AuthorizedFob.find({fob_id: fob_id}).toArray(function (err: cbError, data: IFob[]) {
            if (err) return reject(err);
            if (data.length > 0) return reject("Fob ID already exists.");

            let fob: IFob = {
                username: username,
                fob_id: fob_id,
                fob_code: hashedFobCode
            };
            collections.AuthorizedFob.insert([fob], function (err: cbError, data: IFob[]) {
                if (err) {
                    return reject(err);
                }

                resolve(sanitizeFob(data[0]));
            });
        })
    });
}

function insertSuperUser(username: string, password: string, loc: string)
{
    password = hashValue(password);
    return new Promise<ISuperUser>(function (resolve, reject) {
        collections['SuperUser'].insert([
            {username: username, password: password, loc: loc}
        ], function (err: cbError, data: ISuperUser[]) {
            if (err) reject(err);
            else resolve();
        })
    });
}


function getSuperUser(username: string, password: string)
{
    password = hashValue(password);
    return new Promise<ISuperUser>(function (resolve, reject) {
        collections['SuperUser'].find({username, password}).toArray(function (err: cbError, data: ISuperUser[]) {
            if (err) reject(err);
            else resolve(data[0]);
        });
    });
}

export const accessLog = { get: queryFobAccessLog, insert: logFobAccess};
export const superUser = {get: getSuperUser, insert: insertSuperUser};
export const fobs = {list: listFobs, get: getFob, insert: insertFob, access: fobAccess};

if (require.main == module) {
    init();
}
