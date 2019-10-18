!function () {
    var SETTINGS;
    var CHARTS = {};
    var CHART_DATA = {};
    var TOGGLE_BUTTONS = {};

    function setLoading(loading) {
        if (loading) {
            document.querySelector("#loading").classList.remove("hidden");
            document.querySelector("#content").classList.add("hidden");
        }
        else {
            document.querySelector("#loading").classList.add("hidden");
            document.querySelector("#content").classList.remove("hidden");
        }
    }

    function setSensorState(sensorName, enabled) {
        var promise; 
        if (enabled) {
            promise = axios.post("/sensor/" + sensorName + "/enable");
        }
        else {
            promise = axios.post("/sensor/" + sensorName + "/disable");
        }

        return promise.then(function (new_settings) {
            SETTINGS = new_settings.data;
            updateActions();
        });
    }

    function setAlertPeriodSeconds(period) {
        return axios.post("/alert/schedule", {
            alert_period_seconds: period
        }).then(function (new_settings) {
            SETTINGS = new_settings.data;
            updateActions();
        })
    }

    function triggerAlert() {
        return axios.post("/alert/trigger").then(function (new_settings) {
            SETTINGS = new_settings.data;
            updateActions();
        });
    }

    function initCharts() {
        var keys = ["battery", "temperature", "steps"];
        keys.forEach(function (key) {
            var selector = "#" + key + "-canvas";
            var el = document.querySelector(selector);

            CHARTS[key] = new SmoothieChart({responsive: true});
            CHARTS[key].streamTo(el);
            CHART_DATA[key] = new TimeSeries();
            CHARTS[key].addTimeSeries(CHART_DATA[key]);
        });
    }

    function updateCharts(reading) {
        var date = new Date().getTime();

        if (reading.hasOwnProperty('battery_volts') && reading.battery_volts != -1) {
            CHART_DATA.battery.append(date, reading.battery_volts);
        }

        if (reading.hasOwnProperty('temperature_degc') && reading.temperature_degc != -100) {
            CHART_DATA.temperature.append(date, reading.temperature_degc);
        }

        if (reading.hasOwnProperty('steps') && reading.steps != -1) {
            CHART_DATA.steps.append(date, reading.steps);
        }
    }

    function updateActions() {
        var keys = ["temperature", "step", "battery"];
        keys.forEach(function (key) {
            var el = document.querySelector("#" + key + "-enable");
            var enabled = !!SETTINGS[key + "_sensor_enabled"]; 
            if (enabled) {
                el.classList.add("on"); 
                el.textContent = "On";
            }
            else {
                el.classList.remove("on"); 
                el.textContent = "Off";
            }
        });
    }

    function getSettings() {
        return axios.get("/settings");
    }


    function setupSocket() {
        var host = window.location.host;
        var socket = io("http://"+ host + ":8000");
        socket.on("data", function (data) {
            updateCharts(data);
        });
    }

    function button_clicked(event) {
        console.log("Clicked");
        var sensor = event.target.id.split("-")[0]; 
        var state = !!SETTINGS[sensor + "_sensor_enabled"];
        setSensorState(sensor, !state);
    }
    
    function init() {
        document.querySelectorAll("button").forEach(function (btn) {
            btn.addEventListener("click", button_clicked);
        });
        getSettings().then(function (response) {
            SETTINGS = response.data;

            initCharts(); 
            updateActions();
            setupSocket();
            setLoading(false);
        });
    }

    document.addEventListener("DOMContentLoaded", init);
}();

