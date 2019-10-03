(function () {
    var dataPoints = {
        thermistor: [],
        rangefinder: [],
        ultrasonic: [],
        battery: []
    };

    var distanceChart = new CanvasJS.Chart("distanceChart", {
        zoomEnabled: true,
        title: {
            text: "Distance"
        },
        axisX: {
            title: "Time"
        },
        axisY: {
            title: "Distance (m)"
        },
        data: [
            {
                type: "line",
                showInLegend: true,
                xValueType: "dateTime",
                xValueFormatString: "YYYY-MM-DD HH:mm:ss",
                name: "IR Rangefinder",
                dataPoints: dataPoints.rangefinder
            },
            {
                type: "line",
                xValueType: "dateTime",
                xValueFormatString: "YYYY-MM-DD HH:mm:ss",
                showInLegend: true,
                name: "Ultrasonic",
                dataPoints: dataPoints.ultrasonic
            }
        ]
    });

    var temperatureChart = new CanvasJS.Chart("temperatureChart", {
        title: {
            text: "Temperature"
        },
        axisX: {
            title: "Time"
        },
        axisY: {
            title: "Temp (degC)"
        },
        data: [
            {
                type: "line",
                showInLegend: true,
                xValueType: "dateTime",
                xValueFormatString: "YYYY-MM-DD HH:mm:ss",
                name: "Thermistor",
                dataPoints: dataPoints.thermistor
            }
        ]
    });

    var batteryChart = new CanvasJS.Chart("batteryChart", {
        title: {
            text: "Battery Voltage"
        },
        axisX: {
            title: "Time"
        },
        axisY: {
            title: "Voltage (V)"
        },
        data: [
            {
                type: "line",
                showInLegend: true,
                xValueType: "dateTime",
                xValueFormatString: "YYYY-MM-DD HH:mm:ss",
                name: "Battery",
                dataPoints: dataPoints.battery
            }
        ]
    });

    /**
     * @param data: SensorReading (see monitor.js)
    */
    var RENDER_WINDOW_SECONDS = 60; 
    function renderData(data) {
        var date = Date.parse(data.timestamp);
        data.sensors.forEach(function (sensor) {
            dataPoints[sensor.name].push({
                x: date,
                y: sensor.value
            });

            // Only show the last 60 seconds of data
            if (RENDER_WINDOW_SECONDS != null) {
                if (dataPoints[sensor.name].length == Math.floor(RENDER_WINDOW_SECONDS/2) + 1) {
                    dataPoints[sensor.name].shift();
                }
            }
        })

        distanceChart.render();
        temperatureChart.render();
        batteryChart.render();
    }

    function fetchCSV(filename) {
        return new Promise(function (resolve, reject) {
            var req = new XMLHttpRequest();

            req.open("GET", "/" + filename);
            req.onreadystatechange = function () {
                if (req.readyState < 4) {
                    return;
                }

                if (req.status != 200 ) {
                    reject("Error getting data: " + req.responseText);
                }
                else {
                    resolve(req.responseText);
                }
                
            };

            req.send();
        });
    }

    function renderCSV(filename) {
        return fetchCSV(filename).then(function (data) {
            var rows = data.split("\n");
            var headers = rows[0].split(",").map(function (d) { return d.trim();});

            rows.slice(1).forEach(function (row) {
                if (!row.trim()) return;
                var columns = row.split(",").map(function(d) {return d.trim();});
                var time, y, name;
                var dataPoint = {
                    timestamp: null,
                    sensors: []
                };

                columns.forEach(function (col, i) {
                    name = headers[i];
                    if (i == 0) { 
                        dataPoint.timestamp = new Date(Date.parse(col));
                    }
                    else {
                        dataPoint.sensors.push({name: name, value: parseFloat(col), units: null});
                    }
                });

                renderData(dataPoint);
            });
        }).catch(function (err) {
            console.error("Error fetching csv: " + err);
            return Promise.reject(err);
        })
    }

    function main() {
        // Make the charts show up
        distanceChart.render();
        temperatureChart.render();
        batteryChart.render();

        var csvToRender = window.location.search.match(/csv=(.*)/);
        if (csvToRender) {
            renderCSV(csvToRender[1]);
        }
        else {
            var renderWindow = window.location.search.match(/window=(.*)/);
            if (renderWindow) {
                var winSizeSec = parseFloat(renderWindow[1]);
                RENDER_WINDOW_SECONDS = winSizeSec == -1 ? null : 
                                        winSizeSec >   5 ? winSizeSec : 
                                        RENDER_WINDOW_SECONDS;
            }

            var socket = io('http://localhost:8080');
            socket.on('data', function (data) {
                if (!data.hasOwnProperty("timestamp") || !data.hasOwnProperty("sensors")) {
                    console.warn("Got unknown data format: ", data);
                    return;
                }

                renderData(data);
            });
        }
    }

    document.addEventListener("DOMContentLoaded", main);
}());

