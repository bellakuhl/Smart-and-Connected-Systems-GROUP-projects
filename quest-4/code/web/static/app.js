(function () {
    const PWM_MIN = 900;
    const PWM_MAX = 2400;
    const PWM_STOP = 1500;
    const PWM_STEP = 50;

    const ESC_CONTROL = {param: "esc", value: PWM_STOP};
    const STEER_CONTROL = {param: "steering", value: PWM_STOP};

    function sendUpdate(control) {
        return axios.post("/control", control);
    }

    function logMessage(message) {
        var el = document.createElement("div");
        el.textContent = message;
        document.querySelector("#log").appendChild(el);
    }

    function increment(control) {
        var before = control.value;
        control.value = Math.min(control.value + PWM_STEP, PWM_MAX);
        sendUpdate(control)
            .then(function (resp) {
                logMessage(resp.data);
            })
            .catch(function (resp) {
                logMessage(resp.response.data);
                control.value = before;
            });
    }

    function decrement(control) {
        var before = control.value;
        control.value = Math.max(control.value - PWM_STEP, PWM_MIN);
        sendUpdate(control)
            .then(function (resp) {
                logMessage(resp.data);
            })
            .catch(function (resp) {
                logMessage(resp.response.data);
                control.value = before;
            });
    }

    function reset(control) {
        var before = control.value;
        control.value = PWM_STOP;
        sendUpdate(control)
            .then(function (resp) {
                logMessage(resp.data);
            })
            .catch(function (resp) {
                logMessage(resp.response.data);
                control.value = before;
            });
    }

    function setup_socket() {
        var socket = io(window.location.origin);
        socket.on("message", logMessage);
    }

    document.addEventListener("DOMContentLoaded", function () {
        document.querySelector("#escForward").addEventListener("click", decrement.bind(null, ESC_CONTROL));
        document.querySelector("#escBackward").addEventListener("click", increment.bind(null, ESC_CONTROL));
        document.querySelector("#escStop").addEventListener("click", reset.bind(null, ESC_CONTROL));

        document.querySelector("#steerForward").addEventListener("click", decrement.bind(null, STEER_CONTROL));
        document.querySelector("#steerBackward").addEventListener("click", increment.bind(null, STEER_CONTROL));
        document.querySelector("#steerCenter").addEventListener("click", reset.bind(null, STEER_CONTROL));

        document.querySelector("#startAuto").addEventListener("click", function () {
            axios.post("/start").then(function (resp) {
                logMessage(response.data);
            }).catch(function (xhr) {
                logMessage(xhr.response.data);
            });
        });

        document.querySelector("#stopAuto").addEventListener("click", function () {
            axios.post("/stop").then(function (resp) {
                logMessage(response.data);
            }).catch(function (xhr) {
                logMessage(xhr.response.data);
            });
        });
        setup_socket();
    });
}());
