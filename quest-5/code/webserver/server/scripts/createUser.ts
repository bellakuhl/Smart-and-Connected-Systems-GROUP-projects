import * as db from "../database";

async function create_fob(uname: string, fobid: number, fob_code: number) {
    try {
        return await db.fobs.insert(uname, fobid, fob_code);
    }
    catch (err) {
        console.error(`Fob Already Exists: ${fobid}`);
    }
}

async function create_user(uname: string, password: string, loc: string) {
    try {
        let user = await db.superUser.get(uname, password);
        if (user) {
            console.log(`User Exists: ${uname}`);
        }
        else {
            console.log(`Creating User: ${uname}`);
            return db.superUser.insert(uname, password, loc);
        }
    }
    catch (err) {
        console.error(`Error getting ${uname}: ${err}`);
    }
}

async function main() {
    await db.init();
    await create_user("sechub", "sechub1115", "classroom");
    await create_user("admin", "team15", "office");
    await create_fob("JRFob", 1234, 7890);
}

async function testUpdate() {
    await db.init();
    let fob = await db.fobs.get(1234);
    if (fob == null) {
        console.error(`Fob not found: ${fob}`);
    }

    db.collections.AuthorizedFob.update({fob_id: fob.fob_id}, {$set: {fob_state: "UNLOCKED"}}, function (err: any, data: any) {
        let fob = db.fobs.get(1234);
        console.log(data);
    });
}
main();
//testUpdate();
