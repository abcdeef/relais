function onMessage(event) {
    var tmp;
    try {
        tmp = JSON.parse(event.data);
    } catch (e) {
        console.log(e); // error in the above string (in this case, yes)!
        console.log(event.data);
    }
    if (tmp.programme) {
	var elem = document.getElementsByClassName("grid-container")[1];
	elem.innerHTML = "<h4>Programme</h4>";
    	for (let i = 0; i < tmp.programme.length; i++) {
    		var b = document.createElement("button");
    		b.id = tmp.programme[i].id;
    		b.setAttribute("onclick", "toggleButtom(this)");
    		b.className = "button progress-button";
    		
    		var s1 = document.createElement("span");
    		s1.innerHTML = tmp.programme[i].name;
    		s1.className = "content";
    		b.appendChild(s1);
    		
    		var s2 = document.createElement("span");
    		s2.className = "progress-inner";
    		s2.style.width=0;
    		b.appendChild(s2);
    		
    		elem.appendChild(b);
            
    	}
    }
    if (tmp.gpios) {
        document.getElementsByClassName("grid-container")[0].innerHTML = "";
        for (let i = 0; i < tmp.gpios.length; i++) {
            if (tmp.gpios[i].gpio == 4) {
                continue;
            }
            var d = document.createElement("div");
            var h4 = document.createElement("h4");
            h4.innerHTML = tmp.gpios[i].name;
            d.appendChild(h4);

            var label = document.createElement("label");
            label.className = "switch";

            var input = document.createElement("input");
            input.type = "checkbox";
            input.id = tmp.gpios[i].gpio;
            input.setAttribute("onchange", "toggleCheckbox(this)");
            input.checked = (tmp.gpios[i].state == 1) ? true : false;
            label.appendChild(input);

            var s = document.createElement("span");
            s.className = "slider";
            label.appendChild(s);

            d.appendChild(label);
            document.getElementsByClassName("grid-container")[0].appendChild(d);
        }
    }
    if (tmp.VERSION) {
        var element = document.getElementById("version");
        element.innerHTML = "Version: " + tmp.VERSION;
    }
    if (tmp.msg) {
        var asd = document.getElementById("unten");
        asd.innerHTML = "";
        for (var i = 0; i < tmp.msg.length; i++) {
		let div = document.createElement("div");
		div.className = "pillow";
		div.innerText = tmp.msg[i].name + " " + tmp.msg[i].time;
        	asd.appendChild(div);
        	let span = document.createElement("span");
       		div.appendChild(span);
        }
        
        
        /*var t = document.createElement("table");
        for (var i = 0; i < tmp.msg.length; i++) {
            var tr = document.createElement("tr");

            var td1 = document.createElement("td");
            td1.innerText = tmp.msg[i].name;
            var td2 = document.createElement("td");
            td2.innerText = tmp.msg[i].time;

            var loe = document.createElement("i");

            loe.name = tmp.msg[i].id;
            loe.onclick = toggleCRON;
            loe.className = "bi bi-x-circle";
            td2.appendChild(loe);
            tr.appendChild(td1);
            tr.appendChild(td2);
            t.appendChild(tr);
        }
        document.getElementById("debug").appendChild(t);*/
        
    }
    if (tmp.gpio) {
        var we = document.getElementById(tmp.gpio.gpio);
        if (we) {
            we.checked = tmp.gpio.state;
        }
    }
    if (tmp.programm) {
    var asd = document.getElementById("programme").getElementsByTagName("Button");
    for (let i = 0; i < asd.length; i++) {
        if (asd[i].id == tmp.programm.id) {
            asd[i].classList.add("running");
            asd[i].getElementsByClassName("progress-inner")[0].style.width = tmp.programm.progress + "%";
        } else {
            asd[i].classList.remove("running");
            asd[i].getElementsByClassName("progress-inner")[0].style.width = "0%";
        }
    }
    }
}

function toggleCRON(element) {
    websocket.send(new Uint8Array([55, element.currentTarget.name]).buffer);
}

function toggleCheckbox(element) {
    websocket.send((new Uint8Array([71, parseInt(element.id), (element.checked ? 1 : 0)])).buffer);
}

function toggleButtom(element) {
    websocket.send((new Uint8Array([80, parseInt(element.id)])).buffer);
}

function onOpen() {
    websocket.send(new Uint8Array([255]).buffer);
}
