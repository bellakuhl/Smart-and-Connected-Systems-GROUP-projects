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

interface ICrawlerEventProps {
    since: number|null;
    until: number|null;
}

enum STATE {
    DEFAULT = 0,
    LOADING = 1
}


export interface ICrawlerEventRecord {
    time: number;
    crawler_id: string;
    event: string;
    split_time: number;
}

interface ICrawlerEventRow extends ICrawlerEventRecord {
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

let LogStore: ICrawlerEventRecord[] = [];
const socket = io(window.location.origin);
interface IColumnSort {
    direction: number;
    field: keyof ICrawlerEventRow;
}

export function AccessLogGrid(props: ICrawlerEventProps) {
    const classes = useStyles(undefined);
    const [state, setLoading] = React.useState<STATE>(STATE.LOADING);
    const [rows, setRows] = React.useState<ICrawlerEventRow[]>([]);
    const [sort, setSort] = React.useState<IColumnSort>({direction: -1, field: "time"});

    React.useEffect(function () {
        loadData().then(function () {
            socket.off("crawler-event");
            socket.on("crawler-event", function (data: string) {
                let record: ICrawlerEventRecord = JSON.parse(data).record;
                if (props.since && props.since > record.time) {
                    // too old
                    return
                }
                if (props.until && props.until < record.time) {
                    console.log("too new");
                    return; // too new
                }

                LogStore.push(transformRecord(record));
                setRows(LogStore.slice());
            });
        });
    }, [props]);

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

    function transformRecord(record: ICrawlerEventRecord): ICrawlerEventRow {
        let row: ICrawlerEventRow = Object.assign({}, record);
        row.date = new Date(record.time);
        row.key = `${Math.random() * 1000000}`;
        return row;
    }

    async function loadData() {
        let queries=[];
        let key: keyof ICrawlerEventProps;

        for (key in props) {
            if (props.hasOwnProperty(key) && props[key] !== null) {
                queries.push(`${key}=${props[key]}`)
            }
        }
        let query = "";
        if (queries.length > 0) {
            query = "?" + queries.join("&");
        }

        let response = await axios.get("/crawler-event" + query);
        let rows: ICrawlerEventRecord[] = response.data['events'];
        LogStore = rows.map(transformRecord);
        setRows(LogStore.slice());
        setLoading(STATE.DEFAULT);
    }

    function createSortFunction(id: keyof ICrawlerEventRow) {
        return function (evt: React.MouseEvent) {
            if (sort.field == id) {
                setSort({field: id, direction: sort.direction * -1});
            }
            else {
                setSort({field: id, direction: 1});
            }
        }
    }

    function sortClasses(id: keyof ICrawlerEventRow) {
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


    function renderTable(rows: ICrawlerEventRow[]) {
        rows = rows.sort(function (a: ICrawlerEventRow, b: ICrawlerEventRow) {
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
                            <TableCell onClick={createSortFunction("crawler_id")} className={sortClasses("crawler_id")}>Crawler ID</TableCell>
                            <TableCell onClick={createSortFunction("event")} className={sortClasses("event")}>Event</TableCell>
                            <TableCell onClick={createSortFunction("split_time")} className={sortClasses("split_time")}>Split Time</TableCell>
                        </TableRow>
                        </TableHead>
                        <TableBody>
                        {rows.map(row => (
                            <TableRow key={row.key}>
                                <TableCell component="th" scope="row">
                                    {row.date.toLocaleDateString()} {row.date.toLocaleTimeString()}
                                </TableCell>
                                <TableCell>{row.crawler_id}</TableCell>
                                <TableCell>{row.event}</TableCell>
                                <TableCell>{row.split_time}</TableCell>
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
