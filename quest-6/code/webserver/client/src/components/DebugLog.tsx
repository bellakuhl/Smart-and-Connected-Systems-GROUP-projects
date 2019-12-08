import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as io from "socket.io-client";

interface IDebugLogProps {}

const socket = io(window.location.origin);
const messages: string[] = [];
export function DebugLog(props: IDebugLogProps) {
    let end: Element;
    const [m, setMessages] = React.useState<string[]>([]);

    React.useEffect(function () {
        socket.on("crawler-log", function (data: string) {
            messages.unshift(data)
            setMessages([data]);
        });
    }, []);

    return (
        <div className="crawler-log">
            {messages.map(function (msg) {
                return <span className="log-msg">{msg}</span>
            })}
            <span ref={(el) => end = el}></span>
        </div>
    );
}
