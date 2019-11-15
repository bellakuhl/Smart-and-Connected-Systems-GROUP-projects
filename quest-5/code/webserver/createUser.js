const db = require('./database');
async function create_user(uname, password, loc) {
    db.init().then(function () {
        return db.superUser.get(uname, password);
    }).then(function (user) {
        if (user) {
            console.log("User Exists ", user);
        }
        else {
            console.log("Creating User", uname);
            return db.superUser.insert(uname, password, loc);
        }
    }).catch(function (err) {
        console.error("Error getting user: ", uname, err);
    });
}

async function main() {
    await create_user("admin", "team15", "office"); 
    await create_user("sechub", "sechub1115", "classroom"); 
}
main();

