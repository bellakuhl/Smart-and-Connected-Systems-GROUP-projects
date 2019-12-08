import axios from "axios";
import * as React from "react";
import Button from "@material-ui/core/Button";
import Grid from "@material-ui/core/Grid";

interface ICrawlerControlProps {

}


function sendCommandButton(endpoint: string, body?: any)
{
    return async function () {
        const url = window.location.origin + "/crawler-command" + endpoint;
        return await axios.post(url, body);
    };
}

const PWM_MIN = 900;
const PWM_NEUTRAL = 1500;
const PWM_MAX = 2400;

let steeringValue: number = PWM_NEUTRAL;
let escValue: number = PWM_NEUTRAL;

export function CrawlerControls(props: ICrawlerControlProps) {
    let steeringDiv: Element;

    async function postESCValue(value: number) {
        try {
            await sendCommandButton("/control", {
                param: "esc",
                value: value
            })();

            escValue = value;
        } catch(err) {
            console.log("Error updating esc.");
        }
    }
    async function postSteeringValue(value: number) {
        try {
            await sendCommandButton("/control", {
                param: "steering",
                value: value
            })();

            steeringValue = value;
        } catch(err) {
            console.log("Error updating steering.");
        }
    }

    function escStop() {
        postESCValue(PWM_NEUTRAL);
    }

    React.useEffect(function () {
        axios.get("/crawler/state").then(function (resp) {
            steeringValue = resp.data['steering'];
            escValue = resp.data['esc'];
        }).catch ((err) => {
            console.error(`Error getting esc state: ${err}`);
        });

        window.addEventListener("keypress", function (evt: KeyboardEvent) {
            if (evt.key == "f") {
                evt.preventDefault();
                // postESCValue(PWM_MIN);
                postESCValue(Math.max(escValue - 70, PWM_MIN));
            }
            else if (evt.key == "b" || evt.key == "r") {
                evt.preventDefault();
                // postESCValue(PWM_MAX);
                postESCValue(Math.min(escValue + 70, PWM_MAX));
            }
            else if (evt.key == " " || evt.key == "s") {
                evt.preventDefault();
                escStop();
            }
            else if (evt.key == "a") {
                evt.preventDefault();
                sendCommandButton("/start-auto")();
            }
            else if (evt.key == "p") {
                evt.preventDefault();
                sendCommandButton("/stop")();
            }
            else if (evt.key == "g") {
                evt.preventDefault();
                sendCommandButton("/green-light")();
            }
        });

        window.addEventListener("keydown", function (evt) {
            if (evt.key == "ArrowLeft") {
                postSteeringValue(Math.min(steeringValue + 100, PWM_MAX));
                // postSteeringValue(PWM_MAX);
            }

            if (evt.key == "ArrowRight") {
                postSteeringValue(Math.max(steeringValue - 100, PWM_MIN));
                postSteeringValue(PWM_MIN);
            }

            if (evt.key == "ArrowUp") {
                postSteeringValue(PWM_NEUTRAL);
            }
        });
    }, []);

    function updateSteering(evt: React.MouseEvent) {
        let rect = steeringDiv.getBoundingClientRect();
        // -2 => don't include borders in the width calculation
        let center = rect.x + (rect.width - 2)/2;
        let centerWidth = 100;
        let centerRegion = [center - centerWidth/2, center + centerWidth/2];

        let steerRegionWidth = (rect.width - centerWidth) / 2;

        if (evt.clientX > centerRegion[1]) {
            let rightStart =  rect.x + rect.width/2 + steerRegionWidth/2;
            let rightPct = (evt.clientX - rightStart) / steerRegionWidth;

            let rightRange = PWM_NEUTRAL - PWM_MIN;
            let right = PWM_NEUTRAL - rightRange*rightPct;
            postSteeringValue(right);
        }
        else if (evt.clientX < centerRegion[0]) {
            let leftStart = rect.x + (rect.width/2) - (steerRegionWidth/2);
            let leftPct = (leftStart - evt.clientX)/(steerRegionWidth);

            let leftRange = PWM_MAX - PWM_NEUTRAL;
            let left = PWM_NEUTRAL + leftRange*leftPct;
            axios.post("/crawler-command/control", {param: "steering", value: left});
            postSteeringValue(left);
        }
        else {
            postSteeringValue(PWM_NEUTRAL);
        }
    }

    function throttle(func: Function, time: number) {
        let timeout: number|null = null;

        return function (evt: React.SyntheticEvent) {
            if (timeout != null) { return; }

            evt.persist();
            let self = this;
            timeout = setTimeout(function () {
                func.call(self, evt);
                timeout = null;
            }, time);
        }
    }


   return (
        <div>
            <Grid container spacing={3}>
                <Grid item xs={12}>
                    <table>
                        <thead>
                            <tr>
                                <th>Action</th>
                                <th>Keys</th>
                                <th>Mode</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td>Auto Mode</td>
                                <td>a</td>
                                <td>Manual</td>
                            </tr>
                            <tr>
                                <td>Stop Auto Mode</td>
                                <td>p</td>
                                <td>Auto</td>
                            </tr>
                            <tr>
                                <td>Increment Forward</td>
                                <td>f</td>
                                <td>Manual</td>
                            </tr>
                            <tr>
                                <td>Reverse</td>
                                <td>r or b</td>
                                <td>Manual</td>
                            </tr>
                            <tr>
                                <td>Stop</td>
                                <td>s or space</td>
                                <td>Manual</td>
                            </tr>
                        </tbody>
                    </table>
                </Grid>
                {/* <Grid item xs={6}>
                    <h3>Mode Controls</h3>
                    <Button variant="contained" onClick={sendCommandButton("/start-auto")}>
                        Start Auto
                    </Button>
                    <Button variant="contained" onClick={sendCommandButton("/stop")}>
                        Stop
                    </Button>
                </Grid>
                <Grid item xs={6}>
                    <h3>Steering Controls</h3>
                    <div className="crawler-steering" ref={(el) => steeringDiv = el} onMouseMove={throttle(updateSteering, 300)}>
                        <div className="steering-label left"></div>
                        <div className="steering-label center"></div>
                        <div className="steering-label right"></div>
                    </div>
                </Grid> */}
            </Grid>
        </div>
   );
}
