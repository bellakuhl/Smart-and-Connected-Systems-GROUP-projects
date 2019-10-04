const fs = require('fs');

function generate_data(num_rows) {
    var headers = ['timestamp','battery','thermistor','ultrasonic','rangefinder'];
    var startDate = new Date();
    var rows = [];
    for (var i = 0; i < num_rows; i++) {
        rows.push([
            (new Date(startDate.getTime() + 2*i*1000)).toISOString(),
            3 + Math.random()*2,
            15 + Math.random()*10,
            0.3 + Math.random()*20,
            0.3 + Math.random()*20
        ]);
    }
    
    return [headers].concat(rows);
}

function writecsv(filename, rows) {
    rows.forEach(function (row, i) {
        var str = row.join(",") + "\n";
        if (i == 0) {
            fs.writeFileSync(filename, str);
        }
        else {
            fs.appendFileSync(filename, str);
        }
    });
}

if (require.main == module) {
    const args = process.argv.slice(2);
    const filename = args.length > 0 ? args[0] : "sensors.csv";
    writecsv(filename, generate_data(100));
}

