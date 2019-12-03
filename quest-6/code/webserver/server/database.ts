/* Isabella Kuhl, Laura Reeve, Joseph Rossi
* Module for database model interactions.
*/
import path from "path";
import crypto from "crypto";
import {promises as fs} from "fs";

const tingodb = require("tingodb");

const Engine = tingodb();
const DATABASE_FOLDER = path.join(__dirname, "crawler.db");
const DB = new Engine.Db(DATABASE_FOLDER, {});
const COLLECTION_NAMES = ["CrawlerEvent", "SuperUser"];
export const collections: {[key: string]: any} = {};

type CrawlerEvent = "START" | "BEACON" | "STOP";

export interface ICrawlerEventRecord {
    time: number;
    crawler_id: string;
    event: CrawlerEvent;
}

export interface ICrawlerEventQuery {
    time?: number | any;
    crawler_id?: string;
    event?: CrawlerEvent;
}

export interface ISuperUser {
    username: string; // use as crawler ID
    password: string;
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

function createCrawlerEvent(event: ICrawlerEventRecord)
{
    return new Promise<ICrawlerEventRecord>(function (resolve, reject) {
        collections['CrawlerEvent'].insert([event], function (err: Error|null, data: ICrawlerEventRecord[]) {
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


function  queryCrawlerEvent(query: ICrawlerEventQuery)
{
    return new Promise<ICrawlerEventQuery[]>(function (resolve, reject) {
        collections['CrawlerEvent'].find(query).toArray(function (err: cbError, data: ICrawlerEventRecord[]) {
            if (err) {
                reject(err);
            }
            else {
                resolve(data);
            }
        });
    });
}

function getLatestCrawlerEvent(crawler_id: string) {
    return new Promise<ICrawlerEventRecord|null>(function (resolve, reject) {
        collections['CrawlerEvent'].find({crawler_id}).sort({time: -1}).limit(1).toArray(function (err: cbError, data: ICrawlerEventRecord[]) {
            if(err) return reject(err);

            if (data.length == 0) return resolve(null);

            resolve(data[0]);
        });
    });
}


function insertSuperUser(username: string, password: string)
{
    password = hashValue(password);
    return new Promise<ISuperUser>(function (resolve, reject) {
        collections['SuperUser'].insert([
            {username: username, password: password}
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

export const superUser = {get: getSuperUser, insert: insertSuperUser};
export const crawlerEvent = {query: queryCrawlerEvent, insert: createCrawlerEvent, last: getLatestCrawlerEvent};

if (require.main == module) {
    init();
}
