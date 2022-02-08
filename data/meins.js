var gateway = `ws://${window.location.hostname}:81/`;
var websocket;

/*
var a = "0 0 8,10,12,14,16,18 * * *";
var b = new Uint8Array(a.length+3);
b[0]=1;
b[1]=103;
[...a].map(function(e,i) {b[2+i]=e.charCodeAt()});
websocket.send(b.buffer);

var a = "0 30 6 * * *";
var b = new Uint8Array(a.length+3);
b[0]=1;
b[1]=100;
[...a].map(function(e,i) {b[2+i]=e.charCodeAt()});
websocket.send(b.buffer);
*/
function debug(msg) {
	console.log(msg);
	document.getElementById("debug").append(msg);
}
function docReady(fn) {
    if (document.readyState === "complete" || document.readyState === "interactive") {
        setTimeout(fn, 1);
    } else {
        document.addEventListener("DOMContentLoaded", fn);
    }
}

function connect() {
    websocket = new WebSocket(gateway);
	websocket.binaryType = "arraybuffer";
	
    websocket.onmessage = onMessage;

    websocket.onopen = onOpen;
    
    websocket.onclose = function(event) {    	
    	debug("close");
        setTimeout(function() {
            debug("reconnect");
            connect();
        }, 500);
    };
    websocket.onerror = function(event) {
    	debug("error");
        websocket.close();
    }
}
docReady(function() {
    connect();
});
