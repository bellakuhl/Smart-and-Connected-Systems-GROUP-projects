import * as db from "../database";

async function log_event(event: db.ICrawlerEventRecord) {
    return await db.crawlerEvent.insert(event);
}

async function create_user(uname: string, password: string) {
    try {
        let user = await db.superUser.get(uname, password);
        if (user) {
            console.log(`User Exists: ${uname}`);
        }
        else {
            console.log(`Creating User: ${uname}`);
            return db.superUser.insert(uname, password);
        }
    }
    catch (err) {
        console.error(`Error getting ${uname}: ${err}`);
    }
}

async function testQuery() {
    await db.init();
    let event = await db.crawlerEvent.last("crawler15");
    console.log(`Found Event: ${JSON.stringify(event)}`);
}

async function main() {
    await db.init();
    // await create_user("crawler15", "team15lastquest");
    // await create_user("admin", "team15");
    // /*

    for (let i = 0; i < 30; i++) {
        await log_event({
            crawler_id: "crawler_15",
            time: new Date().getTime(),
            event: "split",
            split_time: Math.floor(Math.random() * 60)
        });
    }

    // await testQuery();
}

main();