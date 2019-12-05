import * as React from "react";

import Grid from "@material-ui/core/Grid";
import jsQR from "jsqr";

let img: HTMLImageElement;
let imageLoaded = false;

export function CrawlCam() {
    const [scanResult, setScanResult] = React.useState<string>(" ");

    React.useEffect(function () {
        // Scan for qr codes ever 200ms
        const canvas = document.createElement("canvas");
        const ctx = canvas.getContext("2d");
        let interval = setInterval(function () {
            if (!img || !imageLoaded) { return; } // Not ready

            const width = img.getBoundingClientRect().width;
            const height = img.getBoundingClientRect().height;

            canvas.width =  width;
            canvas.height = height;

            ctx.drawImage(img, 0, 0, width, height);
            let imageData = ctx.getImageData(0, 0, width, height);
            let code = jsQR(imageData.data, width, height);
            if (code) {
                let num = img.src.substr(-5);
                setScanResult(`Scan result (${num}): ${code.data}`);
            }
        }, 200);

        // JR - Debug
        let count = -1;
        let timeout = -1;
        (function drawImage () {

            setScanResult("No Code");
            count = (count+1) % 13;
            if (count == 0) count++;

            let ext = (count == 1 || count == 2 || count == 4) ? "png" : "jpg";
            let url = `/qrcodes/${count}.${ext}`;
            img.onload = function () { timeout = setTimeout(drawImage, 2000); };
            img.src = url;
        }());

        return () => {
            clearInterval(interval);
            clearTimeout(timeout);
        }
    }, []);

    return (
        <div>
            <img src="http://192.168.1.131:8081/stream"
                width="300px" height="300px"
                ref={(el) => img = el}
                onLoad={() => imageLoaded = true }/>
            {/* <img src="/qrcodes/1.png"
                width="300px" height="300px"
                ref={(el) => img = el}
                onLoad={() => imageLoaded = true }/> */}
            <Grid container>
                <Grid item xs={12}>
                    <p>{scanResult}</p>
                </Grid>
            </Grid>
        </div>
    );
}