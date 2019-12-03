import axios from "axios";
import * as React from "react";
import * as ReactDOM from "react-dom";
import {Header} from "./components/Header";
import {AccessLogGrid} from "./components/AccessLogGrid";
import {DatePicker} from "./components/DatePicker";

function setHash(props: any) : void {
    let values = [];
    for (let key in props) {
        if (!props.hasOwnProperty(key)) return;
        let name = key;
        let value = props[key];

        if (key != undefined) {
            values.push(`${name}=${value}`);
        }
        else {
            values.push(`${name}`);
        }
    }

    let hash = values.join("&");
    if (hash) {
        window.location.hash = `#${hash}`;
    }
}

function parseHash(): any {
    let hash = window.location.hash;
    let data: any = {};

    if (hash) {
        hash = hash.substr(1);
        let parts = hash.split("&");
        for (let key in parts) {
            if (!parts.hasOwnProperty(key)) return;

            let value = parts[key];
            if (value.indexOf("=") > -1) {
                let [name, v] = value.split("=");
                data[name] = v;
            }
            else {
                data[name] = undefined;
            }
        }
    }

    return data;
}

let URLState: any = {};

function App() {
    URLState = parseHash();
    let defaultSince = URLState.since ? parseInt(URLState.since, 10) : null;
    let defaultUntil = URLState.until ? parseInt(URLState.until, 10) : null;

    const [since, setSince] = React.useState<number|null>(defaultSince);
    const [until, setUntil] = React.useState<number|null>(defaultUntil);

    function updateSince(date: Date) {
        date.setHours(0);
        date.setMinutes(0);
        date.setSeconds(0);
        URLState.since = date.getTime();
        setHash(URLState);
        setSince(date.getTime());
    }
    function updateUntil(date: Date) {
        date.setHours(23);
        date.setMinutes(59);
        date.setSeconds(59);
        URLState.until = date.getTime();
        setHash(URLState);
        setUntil(date.getTime());
    }

    return (<div>
        <Header />
        <div className="content">
            <h2>Crawler Event Log</h2>
            <DatePicker label="Since" onChange={updateSince} defaultDate={since} />
            <DatePicker label="Until" onChange={updateUntil} defaultDate={until} />
            <AccessLogGrid since={since} until={until} />
        </div>
    </div>);
}

ReactDOM.render(<App /> , document.getElementById("app"));
