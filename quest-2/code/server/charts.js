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

    var socket = io('http://localhost:8080');
    socket.on('data', function (data) {
        if (!data.hasOwnProperty("timestamp") || !data.hasOwnProperty("sensors")) {
            console.warn("Got unknown data format: ", data);
            return;
        }

        var date = Date.parse(data.timestamp);
        data.sensors.forEach(function (sensor) {
            dataPoints[sensor.name].push({
                x: date,
                y: sensor.value
            });

            // Only show the last 30 seconds of data
            if (dataPoints[sensor.name].length == 60 /2) {
                dataPoints[sensor.name].shift();
            }
        })

        distanceChart.render();
        temperatureChart.render();
        batteryChart.render();
    });
}());
