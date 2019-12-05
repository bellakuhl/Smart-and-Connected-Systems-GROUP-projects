import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as io from "socket.io-client";

interface IDebugLogProps {

}

const socket = io(window.location.origin);
export function DebugLog(props: IDebugLogProps) {
    let end: Element;
    const [messages, setMessages] = React.useState<string[]>([]);

    React.useEffect(function () {
        socket.on("crawler-log", function (data: string) {
            setMessages(messages.concat([data]));
        });
    }, []);

    React.useEffect(function () {
        end.scrollIntoView({behavior: "smooth"});
    });

    return (
        <div className="crawler-log">
            {messages.map(function (msg) {
                return <span className="log-msg">{msg}</span>
            })}
            <span ref={(el) => end = el}></span>
        </div>
    );
}
