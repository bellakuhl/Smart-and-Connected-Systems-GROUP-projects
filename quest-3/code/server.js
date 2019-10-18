var dgram = require("dgram");

var PORT = 8080;
var HOST = "192.168.1.64";
var WEARABLE_IP = "192.168.1.124";
var WEARABLE_PORT = 8080;

var server = dgram.createSocket("udp4");

server.on("listening", function () {
    var address = server.address();
});

server.on("message", function (message, remote) {
    console.log(remote.address + ":" + remote.port +" - " + message);
});

function serialize_data(settings) {
    var data = new Uint8Array(8);
    data[0] = settings.batt_en;
    data[1] = settings.temp_en;
    data[2] = settings.step_en;
    data[3] = settings.alert_now;

    data[4] = (settings.alert_period >> 0)  & 0xFF;
    data[5] = (settings.alert_period >> 4)  & 0xFF;
    data[6] = (settings.alert_period >> 8)  & 0xFF;
    data[7] = (settings.alert_period >> 12) & 0xFF;
    return data;
}

exports.sendMessage = function (data) {
    server.send(serialize_data(data), WEARABLE_PORT, WEARABLE_IP, function(error) {
        if (error) { console.error("Error sending message!"); }
        else { console.log("Sent Message"); }
    });
};

server.bind(PORT, HOST);
