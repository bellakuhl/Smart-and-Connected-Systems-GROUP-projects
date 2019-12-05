import axios from "axios";
import * as React from "react";
import * as ReactDOM from "react-dom";
import {Header} from "./components/Header";
import {AccessLogGrid} from "./components/AccessLogGrid";
import {DatePicker} from "./components/DatePicker";
import {DebugLog} from "./components/DebugLog";
import {CrawlerControls} from "./components/CrawlerControls";

import { makeStyles, createStyles, Theme } from '@material-ui/core/styles';
import Grid from "@material-ui/core/Grid";
import Paper from '@material-ui/core/Paper';
import Button from "@material-ui/core/Button";

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

const useStyles = makeStyles((theme: Theme) =>
  createStyles({
    root: {
      flexGrow: 1,
    },
    paper: {
      padding: theme.spacing(2),
      color: theme.palette.text.secondary,
    },
  }),
);

function App() {
    URLState = parseHash();
    let defaultSince = URLState.since ? parseInt(URLState.since, 10) : null;
    let defaultUntil = URLState.until ? parseInt(URLState.until, 10) : null;

    const [since, setSince] = React.useState<number|null>(defaultSince);
    const [until, setUntil] = React.useState<number|null>(defaultUntil);
    const [scanResult, setScanResult] = React.useState<string>("");

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

    async function scanImage () {
        try {
            let resp = await axios.post("/scan-qr-code");
            setScanResult("Scan result: " + resp.data['result']);
        } catch (xhr) {
            setScanResult("QR Code could not be decoded.");
        }
    }

    const classes = useStyles({});

    return (<div>
        <Header />
        <div className={classes.root}>
            <Grid container spacing={3}>
                <Grid item xs={6}>
                    <Paper className={classes.paper}>
                        <h2>CrawlCam<sup>&trade;</sup></h2>
                        <iframe src="http://localhost:8081" width="100%" height="300px"></iframe>
                        <Grid container>
                            <Grid item xs={8}>
                                <p>{scanResult}</p>
                            </Grid>
                            <Grid item xs={4}>
                                <Button variant="contained" onClick={scanImage}>Scan Code</Button>
                            </Grid>
                        </Grid>
                    </Paper>
                </Grid>
                <Grid item xs={6}>
                    <Paper className={classes.paper}>
                        <h2>Crawler Controls</h2>
                        <CrawlerControls />
                    </Paper>
                </Grid>
                <Grid item xs={6}>
                    <div className={classes.paper}>
                        <h2>Events</h2>
                        <DatePicker label="Since" onChange={updateSince} defaultDate={since} />
                        <DatePicker label="Until" onChange={updateUntil} defaultDate={until} />
                        <AccessLogGrid since={since} until={until} />
                    </div>
                </Grid>
                <Grid item xs={6}>
                    <Paper className={classes.paper}>
                        <h2>Log</h2>
                        <DebugLog />
                    </Paper>
                </Grid>
            </Grid>
        </div>
    </div>);
}

ReactDOM.render(<App /> , document.getElementById("app"));
