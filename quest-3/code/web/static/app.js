!function () {
    var SETTINGS;
    var CHARTS = {};
    var CHART_DATA = {};
    var TOGGLE_BUTTONS = {};

    function debounce(func, ms) {
        var timeout = setTimeout(func, ms);

        return function () {
            clearTimeout(timeout);
            timeout = setTimeout(func, ms);
        };
    }

    function getAlertElements() {
        return {
            input: document.querySelector("input.alert-value"),
            trigger: document.querySelector("#triggerAlert")
        };
    }

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
        var options = [
            {minValue: 0, maxValue: 5.0},
            {minValue: 10, maxValue: 30},
            {minValue: 0}
        ];
        keys.forEach(function (key, i, arr) {
            var selector = "#" + key + "-canvas";
            var el = document.querySelector(selector);
            var chartOptions = options[i];
            chartOptions.responsive = true; 
            chartOptions.interpolation= "linear";
            chartOptions.tooltip= true;
            chartOptions.timestampFormatter= SmoothieChart.timeFormatter

            CHARTS[key] = new SmoothieChart(chartOptions);
            CHARTS[key].streamTo(el);
            CHART_DATA[key] = new TimeSeries();
            CHARTS[key].addTimeSeries(CHART_DATA[key], {lineWidth:2, strokeStyle:'#00ff00'});
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
        // Chart Toggles
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

        // Alert Period
        getAlertElements().input.value = SETTINGS.alert_period_seconds;
        if (SETTINGS.alert_now) {
            getAlertElements().trigger.classList.add("on");
        }
        else {
            getAlertElements().trigger.classList.remove("on");
        }
    }

    function getSettings() {
        return axios.get("/settings");
    }


    function setupSocket() {
        var socket = io(window.location.origin);

        socket.on("data", function (data) {
            updateCharts(data);
        });
    }

    function chart_enable_clicked(event) {
        var sensor = event.target.id.split("-")[0]; 
        var state = !!SETTINGS[sensor + "_sensor_enabled"];
        setSensorState(sensor, !state);
    }

    function alertInputChanged() {
        var alertElements = getAlertElements();
        var value = parseInt(alertElements.input.value, 10);
        if (!isNaN(value) && value !== SETTINGS.alert_period_seconds) {
            setAlertPeriodSeconds(value);
        }
    }
    
    function init() {
        var alertElements = getAlertElements();
        document.querySelectorAll(".chart-wrapper button").forEach(function (btn) {
            btn.addEventListener("click", chart_enable_clicked);
        });

        alertElements.input.addEventListener("input", debounce(alertInputChanged, 300));
        alertElements.trigger.addEventListener("click", triggerAlert);

        getSettings().then(function (response) { SETTINGS = response.data;
            initCharts(); 
            updateActions();
            setupSocket();
            setLoading(false);
        });
    }

    document.addEventListener("DOMContentLoaded", init);
}();

