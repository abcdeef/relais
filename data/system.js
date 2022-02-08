function print(a) {
    let e = Math.floor(a / 1000);
    return (a ? Math.floor(e / 60).toString().padStart(2, "0") + ":" + (e % 60).toString().padStart(2, "0") : "");
}

function onMessage(event) {
    try {
        var tmp = JSON.parse(event.data);
    } catch (e) {
        console.log(e); // error in the above string (in this case, yes)!
        console.log(event.data);
    }

    if (tmp.system) {
        var elem = document.getElementById("system");
        elem.children[0].children[0].children[1].innerHTML = tmp.system.localtime;
        elem.children[0].children[1].children[1].innerHTML = tmp.system.uptime;

        if (tmp.system.rtc) {
            elem.children[0].children[2].children[1].innerHTML = tmp.system.rtc;
        } else {
            elem.children[0].children[2].hidden = true;
        }
        if (tmp.system.PEGEL) {
            elem.children[0].children[3].children[1].innerHTML = tmp.system.PEGEL;
        } else {
            elem.children[0].children[3].hidden = true;
        }
    }
    if (tmp.gpios) {
        document.getElementById("gpio").innerHTML = "";

        var t = document.getElementById("gpio");

        var header = document.createElement("tr");
        ["GPIO", "Kreislauf", "since", "Dauer", "Verbrauch"].forEach((a) => {
            var th = document.createElement("th");
            th.innerText = a;
            header.appendChild(th);
        });
        t.appendChild(header);

        for (let i in tmp.gpios) {
            var tr = document.createElement("tr");
            tr.className = (tmp.gpios[i].state == 1) ? "sel" : "tr";

            [{
                val: tmp.gpios[i].gpio
            }, {
                val: tmp.gpios[i].name
            }, {
                val: tmp.gpios[i].since,
                func: print
            }].forEach((e) => {
                var td0 = document.createElement("td");
                td0.innerText = (e.func ? e.func(e.val) : e.val);
                tr.appendChild(td0);
            });

            [{
                value: tmp.gpios[i].dauer,
                ws: 83
            }, {
                value: tmp.gpios[i].verbrauch,
                ws: 93
            }].forEach((e) => {
                var td2 = document.createElement("td");
                if (tmp.gpios[i].gpio != 4) {
                    var ip1 = document.createElement("input");
                    ip1.onchange = (a) => {
                        websocket.send(new Uint8Array([e.ws, parseInt(a.currentTarget.name), a.currentTarget.value]).buffer);
                    }
                    ip1.value = e.value;
                    ip1.size = 3;
                    ip1.type = "number";
                    ip1.name = tmp.gpios[i].gpio;
                    td2.appendChild(ip1);
                }
                tr.appendChild(td2);
            });

            t.appendChild(tr);
        }
        document.getElementById("gpio").appendChild(t);
    }
}

function onOpen() {
    websocket.send(new Uint8Array([5]).buffer);
}
