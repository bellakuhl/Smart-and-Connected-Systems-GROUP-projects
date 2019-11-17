import * as React from 'react';
import { createStyles, Theme, makeStyles } from '@material-ui/core/styles';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import Paper from '@material-ui/core/Paper';
import * as io from "socket.io-client";
import axios from "axios";

interface IAccessLogProps {}

enum STATE {
    DEFAULT = 0,
    LOADING = 1
}

interface IAccessLogRecord {
    fob_id: number;
    hub_id: string;
    person: string;
    time: number;
    loc: string;
    fob_state: string;
}

interface IAccessLogRow extends IAccessLogRecord {
    date?: Date;
    key?: string;
}

const useStyles = makeStyles((theme: Theme) =>
    createStyles({
        root: {
            width: '100%',
        },
        paper: {
            marginTop: theme.spacing(3),
            width: '100%',
            overflowX: 'auto',
            marginBottom: theme.spacing(2),
        },
        table: {
            minWidth: 650,
        },
    }),
);

let LogStore: IAccessLogRecord[] = [];
const socket = io(window.location.origin);
export function AccessLogGrid(props: IAccessLogProps) {
    const classes = useStyles(undefined);
    const [state, setLoading] = React.useState<STATE>(STATE.LOADING);
    const [rows, setRows] = React.useState<IAccessLogRow[]>([]);

    React.useEffect(function () {
        loadData().then(function () {
            socket.on("access-request", function (data: string) {
                let record: IAccessLogRecord = JSON.parse(data).record;
                LogStore.push(transformRecord(record));
                setRows(LogStore.slice());
            });
        });
    }, []);

    function renderEmpty() {
        return (
            <div className="empty-table">
                No records available
            </div>
        );
    }

    function renderLoading() {
        return (
            <div>Loading</div>
        );
    }

    function transformRecord(record: IAccessLogRecord): IAccessLogRow {
        let row: IAccessLogRow = Object.assign({}, record);
        row.date = new Date(record.time);
        row.key = `${record.time}_${record.hub_id}`;
        return row;
    }

    async function loadData() {
        let response = await axios.get("/fob-access/log");
        let rows: IAccessLogRecord[] = response.data['access_log'];
        LogStore = rows.map(transformRecord);
        setRows(LogStore.slice());
        setLoading(STATE.DEFAULT);
    }

    function renderTable(rows: IAccessLogRow[]) {
        return (
            <div className={classes.root}>
            <Paper className={classes.paper}>
                <Table className={classes.table} size="small" aria-label="a dense table">
                    <TableHead>
                        <TableRow>
                            <TableCell>Date</TableCell>
                            <TableCell>Person</TableCell>
                            <TableCell>Location</TableCell>
                            <TableCell>Action</TableCell>
                            <TableCell>Fob ID</TableCell>
                            <TableCell>Hub ID</TableCell>
                        </TableRow>
                        </TableHead>
                        <TableBody>
                        {rows.map(row => (
                            <TableRow key={row.time}>
                            <TableCell component="th" scope="row">
                                {row.date.toLocaleDateString()} {row.date.toLocaleTimeString()}
                            </TableCell>
                            <TableCell>{row.person}</TableCell>
                            <TableCell>{row.loc}</TableCell>
                            <TableCell>{row.fob_state}</TableCell>
                            <TableCell>{row.fob_id}</TableCell>
                            <TableCell>{row.hub_id}</TableCell>
                            </TableRow>
                        ))}
                        </TableBody>
                </Table>
            </Paper>
            </div>
        );
    }

    let data: JSX.Element;
    if (state == STATE.LOADING) {
        data = renderLoading();
    }
    else if (rows.length == 0) {
        data = renderEmpty();
    }
    else {
        data = renderTable(rows);
    }

    return (
        <div className="wrapper">
            {data}
        </div>
    );
}
