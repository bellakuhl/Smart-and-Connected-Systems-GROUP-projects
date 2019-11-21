import axios from "axios";
import * as React from "react";
import * as ReactDOM from "react-dom";
import {Header} from "./components/Header";
import {AccessLogGrid} from "./components/AccessLogGrid";


ReactDOM.render (
    (<div>
        <Header />
        <div className="content">
            <h2>Fob Access Log</h2>
            <AccessLogGrid />
        </div>
    </div>)
    , document.getElementById("app"));
