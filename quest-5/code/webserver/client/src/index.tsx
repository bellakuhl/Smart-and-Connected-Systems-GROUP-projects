import axios from "axios";
import * as React from "react";
import * as ReactDOM from "react-dom";
import {Header} from "./components/Header";
import {AccessLogGrid} from "./components/AccessLogGrid";


ReactDOM.render (
    (<div>
        <Header />
        <AccessLogGrid />
    </div>)
    , document.getElementById("app"));
