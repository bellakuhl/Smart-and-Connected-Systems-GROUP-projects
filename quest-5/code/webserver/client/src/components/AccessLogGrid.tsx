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
            maxHeight: '600px',
            overflowY: 'auto',
            marginBottom: theme.spacing(2),
        },
        table: {
            minWidth: 650,
        },
    }),
);

let LogStore: IAccessLogRecord[] = [];
const socket = io(window.location.origin);
interface IColumnSort {
    direction: number;
    field: keyof IAccessLogRow;
}


export function AccessLogGrid(props: IAccessLogProps) {
    const classes = useStyles(undefined);
    const [state, setLoading] = React.useState<STATE>(STATE.LOADING);
    const [rows, setRows] = React.useState<IAccessLogRow[]>([]);
    const [sort, setSort] = React.useState<IColumnSort>({direction: -1, field: "time"});

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

    function createSortFunction(id: keyof IAccessLogRow) {
        return function (evt: React.MouseEvent) {
            if (sort.field == id) {
                setSort({field: id, direction: sort.direction * -1});
            }
            else {
                setSort({field: id, direction: 1});
            }
        }
    }

    function sortClasses(id: keyof IAccessLogRow) {
        let classes = [];
        if (sort.field == id) {
            classes.push("sorted");
            if (sort.direction == 1) {
                classes.push("asc");
            }
            else {
                classes.push("desc");
            }
        }

        return classes.join(" ");
    }


    function renderTable(rows: IAccessLogRow[]) {
        rows = rows.sort(function (a: IAccessLogRow, b: IAccessLogRow) {
            let aVal = a[sort.field];
            let bVal = b[sort.field];
            let dir = sort.direction;

            if (aVal < bVal) {
                return dir * -1;
            }
            else if (aVal > bVal) {
                return 1 * dir;
            }
            else {
                // Secondary sort by time asc/desc
                return a.time < b.time ? -1 * dir :
                       b.time < a.time ? 1  * dir : 0;
            }
        });

        return (
            <div className={classes.root}>
            <Paper className={classes.paper}>
                <Table className={classes.table} size="small" aria-label="a dense table"  stickyHeader>
                    <TableHead>
                        <TableRow>
                            <TableCell key="time"onClick={createSortFunction("time")} className={sortClasses("time")}>Date</TableCell>
                            <TableCell onClick={createSortFunction("person")} className={sortClasses("person")}>Person</TableCell>
                            <TableCell onClick={createSortFunction("loc")} className={sortClasses("loc")}>Location</TableCell>
                            <TableCell onClick={createSortFunction("fob_id")} className={sortClasses("fob_id")}>Fob ID</TableCell>
                            <TableCell onClick={createSortFunction("hub_id")} className={sortClasses("hub_id")}>Hub ID</TableCell>
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
