let DSItems = [];

let txdata = [];

let rxdata = [];

let loadedParameterPreset = [];

let miscConfigInfo = {};

document.addEventListener("DOMContentLoaded", () => {
    if (window.location.hostname == "rcmgames.github.io") {
        errorConnecting();
    } else {
        loadBoardInfo(() => { });
        loadWifiSettings();
        loadUI();
        loadPresets();
        loadConfig();
        //TODO: handle errors loading from robot
    }
    setInterval(() => {
        document.getElementById("console").innerHTML = "tx: " + txdata + "rx: " + rxdata;
        for (let i = 0; i < DSItems.length; i++) {
            DSItems[i].run(txdata, rxdata);
        }
    }, 20);
});

function errorConnecting() {
    document.getElementById("hostname-box").className = "hostname-error-connecting";
    document.getElementById("wifi-hostname").value = "rcmv3.local";
    let element = document.createElement("span");

    if (window.location.hash == "#new") {
        element.innerHTML = "<br><strong>Welcome to the RCMv3 Driver Station. Connect your computer to the wifi network named 'http://rcmv3.local' then press the connect button below.</strong><br>";
    } else {
        element.innerHTML = "<br>This website isn't connected to a RCM robot. To connect to your robot, enter your robot's hostname then press the connect button.<br>";
    }

    document.getElementById("hostname-box").appendChild(element);
    let element2 = document.createElement("button");
    element2.innerHTML = "connect";
    element2.onclick = () => {
        element.innerHTML = "redirecting you to your robot, please wait...";
        let wifiDataHostname = document.getElementById("wifi-hostname").value;
        if (wifiDataHostname != "") {
            window.location = "http://" + wifiDataHostname;
        }
    }
    document.getElementById("hostname-box").appendChild(element2);
}

keysPressed = new Set([]);

document.addEventListener(
    "keydown",
    (event) => {
        const keyName = event.key;
        keysPressed.add(keyName);
    },
    false,
);

document.addEventListener(
    "keyup",
    (event) => {
        const keyName = event.key;
        keysPressed.delete(keyName);
    },
    false,
);

class DSItem {
    constructor(_dsCanvas, data) {
        this.dsCanvas = _dsCanvas;

        this.type = data["type"];

        this.minorDimension = 50;

        if (data["size"] != undefined) {
            this.size = data["size"];
        } else {
            if (this.type == "button") {
                this.size = 50;
            } else {
                this.size = 250;
            }
        }

        if (data["posX"] != undefined) {
            this.posX = data["posX"];
        } else {
            this.posX = 0;
        }

        if (data["posY"] != undefined) {
            this.posY = data["posY"];
        } else {
            this.posY = 0;
        }

        if (data["dataIndices"] != undefined) {
            this.dataIndices = data["dataIndices"];
        } else {
            if (this.type == "joystick") {
                this.dataIndices = [null, null];
            } else if (this.type == "horiz. slider") {
                this.dataIndices = [null];
            } else if (this.type == "vert. slider") {
                this.dataIndices = [null];
            } else if (this.type == "button") {
                this.dataIndices = [null];
            } else if (this.type == "number indicator") {
                this.dataIndices = [null];
            }
        }

        this.processSize();

        if (this.type == "number indicator") {
            if (data["labelText"] != undefined) {
                this.labelText = data["labelText"];
            } else {
                this.labelText = "";
            }

            if (data["colorHigh"] != undefined) {
                this.colorHigh = data["colorHigh"];
            } else {
                this.colorHigh = "#808080";
            }

            if (data["colorLow"] != undefined) {
                this.colorLow = data["colorLow"];
            } else {
                this.colorLow = "#808080";
            }

            if (data["colorHighVal"] != undefined) {
                this.colorHighVal = data["colorHighVal"];
            } else {
                this.colorHighVal = 1;
            }

            if (data["colorLowVal"] != undefined) {
                this.colorLowVal = data["colorLowVal"];
            } else {
                this.colorLowVal = 0;
            }

            this.indicator = true;
        } else {
            this.indicator = false;
        }

        if (this.indicator == false) {
            if (data["color"] != undefined) {
                this.color = data["color"];
            } else {
                this.color = "#808080"; // must be hex code
            }

            if (data["buttonPressedVal"] != undefined) {
                this.buttonPressedVal = data["buttonPressedVal"];
            } else {
                this.buttonPressedVal = null;
            }

            if (data["buttonReleasedVal"] != undefined) {
                this.buttonReleasedVal = data["buttonReleasedVal"];
            } else {
                this.buttonReleasedVal = null;
            }

            if (data["keyboardKeys"] != undefined) {
                this.keyboardKeys = data["keyboardKeys"];
            } else {
                this.keyboardKeys = [];
            }

            if (data["gamepadAxes"] != undefined) {
                this.gamepadAxes = data["gamepadAxes"];
            } else {
                this.gamepadAxes = [];
            }

            if (data["recenter"] != undefined) {
                this.recenter = data["recenter"];
            } else {
                this.recenter = false;
            }

            this.gamepadActivatedButton = false;
            this.joyx = 0;
            this.joyy = 0;
        }

        this.mousePressed = false;
        this.highlighted = false;
        this.myNameElement = null;


        this.beingEdited = false;


        this.numData = 0;

        this.vars = [];
        if (this.type == "joystick") {
            this.numData = 2;
            this.vars = [0.0, 0.0];
        } else if (this.type == "horiz. slider") {
            this.numData = 1;
            this.vars = [0.0];
        } else if (this.type == "vert. slider") {
            this.numData = 1;
            this.vars = [0.0];
        } else if (this.type == "button") {
            this.numData = 1;
            this.vars = [0];
        } else if (this.type == "number indicator") {
            this.numData = 1;
            this.vars = [0];
        }

        this.onMouseDownBound = this.onMouseDown.bind(this);
        this.onMouseUpBound = this.onMouseUp.bind(this);
        this.onMouseMoveBound = this.onMouseMove.bind(this);
        this.onTouchStartBound = this.onTouchStart.bind(this);
        this.onTouchEndBound = this.onTouchEnd.bind(this);
        this.onTouchMoveBound = this.onTouchMove.bind(this);

        this.dsCanvas.addEventListener('mousedown', this.onMouseDownBound);
        this.dsCanvas.addEventListener('mouseup', this.onMouseUpBound);
        this.dsCanvas.addEventListener('mousemove', this.onMouseMoveBound);
        this.dsCanvas.addEventListener('touchstart', this.onTouchStartBound);
        this.dsCanvas.addEventListener('touchend', this.onTouchEndBound);
        this.dsCanvas.addEventListener('touchmove', this.onTouchMoveBound);

        this.draw();
    }

    jsonify() {
        if (this.indicator) {
            const obj = {
                type: this.type,
                size: this.size,
                colorHigh: this.colorHigh,
                colorLow: this.colorLow,
                colorHighVal: this.colorHighVal,
                colorLowVal: this.colorLowVal,
                posX: this.posX,
                posY: this.posY,
                labelText: this.labelText,
                dataIndices: this.dataIndices
            };
            return obj;
        } else { // control
            const obj = {
                type: this.type,
                size: this.size,
                color: this.color,
                posX: this.posX,
                posY: this.posY,
                buttonPressedVal: this.buttonPressedVal,
                buttonReleasedVal: this.buttonReleasedVal,
                keyboardKeys: this.keyboardKeys,
                gamepadAxes: this.gamepadAxes,
                dataIndices: this.dataIndices,
                recenter: this.recenter
            };
            return obj;
        }
    }

    onMouseDown(event) {
        const rect = this.dsCanvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        if (mouseX >= this.posX && mouseX <= this.posX + this.width && mouseY >= this.posY && mouseY <= this.posY + this.height) {
            this.offsetX = mouseX - this.posX;
            this.offsetY = mouseY - this.posY;
            this.mousePressed = true;
            this.onMove(event.clientX, event.clientY);
            this.draw();
        }
    }
    onTouchStart(event) {
        const rect = this.dsCanvas.getBoundingClientRect();
        for (let i = 0; i < event.touches.length; i++) {
            const touch = event.touches[i];
            const mouseX = touch.clientX - rect.left;
            const mouseY = touch.clientY - rect.top;
            if (mouseX >= this.posX && mouseX <= this.posX + this.width && mouseY >= this.posY && mouseY <= this.posY + this.height) {
                this.offsetX = mouseX - this.posX;
                this.offsetY = mouseY - this.posY;
                this.mousePressed = true;
                this.activeTouchId = touch.identifier;
                this.onMove(touch.clientX, touch.clientY);
                this.draw();
            }
        }
    }
    onMouseUp() {
        this.mousePressed = false;
        this.highlighted = false;
        if (this.beingEdited == false && this.type == "button") {
            this.vars[0] = 0;
        }
        this.draw();
    }
    onMouseMove(event) {
        this.onMove(event.clientX, event.clientY);
    }
    onMove(x, y) {
        if (this.mousePressed) {
            if (this.beingEdited) {
                const rect = this.dsCanvas.getBoundingClientRect();
                this.posX = x - rect.left - this.offsetX;
                this.posY = y - rect.top - this.offsetY;
                this.posX = Math.floor(this.posX / 10) * 10;
                this.posY = Math.floor(this.posY / 10) * 10;
                if (this.posX < 0) this.posX = 0;
                if (this.posY < 0) this.posY = 0;
                if (this.posX + this.width > this.dsCanvas.width) {
                    this.posX = this.dsCanvas.width - this.width
                };
                if (this.posY + this.height > this.dsCanvas.height) {
                    this.posY = this.dsCanvas.height - this.height
                };
                this.redrawCanvas();
                this.draw();
            } else { // running mode
                if (this.type == "button" || this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider") {// run ui controls
                    const rect = this.dsCanvas.getBoundingClientRect();
                    if (this.type == "joystick" || this.type == "horiz. slider") {
                        this.joyx = (x - rect.left - this.posX - this.width / 2) / (this.width / 2 - this.radius);
                    } else {
                        this.joyx = 0;
                    }
                    if (this.type == "joystick" || this.type == "vert. slider") {
                        this.joyy = -(y - rect.top - this.posY - this.height / 2) / (this.height / 2 - this.radius);
                    } else {
                        this.joyy = 0;
                    }
                    if (this.joyx > 1) this.joyx = 1;
                    if (this.joyx < -1) this.joyx = -1;
                    if (this.joyy > 1) this.joyy = 1;
                    if (this.joyy < -1) this.joyy = -1;
                    if (this.type == "joystick") {
                        this.vars[0] = this.joyx;
                        this.vars[1] = this.joyy;
                    } else if (this.type == "horiz. slider") {
                        this.vars[0] = this.joyx;
                    }
                    else if (this.type == "vert. slider") {
                        this.vars[0] = this.joyy;
                    }
                }
                this.draw();
            }
        }
    }
    redrawCanvas() {
        const ctx = this.dsCanvas.getContext('2d');
        ctx.clearRect(0, 0, this.dsCanvas.width, this.dsCanvas.height);
        for (let i = 0; i < DSItems.length; i++) {
            DSItems[i].draw();
        }
    }
    onTouchEnd(event) {
        for (let i = 0; i < event.changedTouches.length; i++) {
            const touch = event.changedTouches[i];
            if (touch.identifier == this.activeTouchId) {
                if (this.beingEdited == false && this.type == "button") {
                    this.vars[0] = 0;
                }
                this.mousePressed = false;
                this.highlighted = false;
                this.activeTouchId = null;
                this.draw();
                break;
            }
        }
    }
    onTouchMove(event) {
        event.preventDefault();
        for (let i = 0; i < event.touches.length; i++) {
            const touch = event.touches[i];
            if (touch.identifier == this.activeTouchId) {
                this.onMove(touch.clientX, touch.clientY);
                break;
            }
        }
    }

    processSize() {
        if (this.type == "joystick") {
            this.width = this.size;
            this.height = this.size;
            this.radius = this.minorDimension / 2;
        } else if (this.type == "horiz. slider") {
            this.width = this.size;
            this.height = this.minorDimension;
            this.radius = this.minorDimension / 2;
        } else if (this.type == "vert. slider") {
            this.width = this.minorDimension;
            this.height = this.size;
            this.radius = this.minorDimension / 2;
        } else if (this.type == "button") {
            this.width = this.size;
            this.height = this.size;
            this.minorDimension = 10;
            this.radius = 0;
        } else if (this.type == "number indicator") {
            this.width = this.size;
            this.height = this.size / 2;
            this.minorDimension = 60;
            this.radius = 0;
        }
    }

    draw() {
        const ctx = this.dsCanvas.getContext('2d');
        ctx.beginPath();
        if (this.mousePressed) {
            this.highlighted = true;
        }
        if (!this.beingEdited) {
            this.highlighted = false;
        }

        if (this.highlighted) {
            ctx.fillStyle = 'white';
            for (let i = 0; i < document.getElementById("ds-list").children.length; i++) {
                if (this.myNameElement == document.getElementById("ds-list").children[i]) {
                    document.getElementById("ds-list").children[i].className = "highlighted-ds-list-item";
                    document.getElementById("ds-properties").replaceChildren(this.propertiesElement(false));

                } else {
                    document.getElementById("ds-list").children[i].className = "non-highlighted-ds-list-item";
                }
            }
        } else {
            if (this.indicator) {
                if (this.vars[0] > this.colorHighVal) {
                    ctx.fillStyle = this.colorHigh;
                } else if (this.vars[0] < this.colorLowVal) {
                    ctx.fillStyle = this.colorLow;
                } else {
                    // interpolate color
                    const r1 = parseInt(this.colorLow.substring(1, 3), 16);
                    const g1 = parseInt(this.colorLow.substring(3, 5), 16);
                    const b1 = parseInt(this.colorLow.substring(5, 7), 16);
                    const r2 = parseInt(this.colorHigh.substring(1, 3), 16);
                    const g2 = parseInt(this.colorHigh.substring(3, 5), 16);
                    const b2 = parseInt(this.colorHigh.substring(5, 7), 16);
                    const percent = (this.vars[0] - this.colorLowVal) / (this.colorHighVal - this.colorLowVal);
                    const r = Math.round(r1 + (r2 - r1) * percent);
                    const g = Math.round(g1 + (g2 - g1) * percent);
                    const b = Math.round(b1 + (b2 - b1) * percent);
                    let colorString = "#";
                    if (r < 16) {
                        colorString += "0" + r.toString(16);
                    } else {
                        colorString += r.toString(16);
                    }
                    if (g < 16) {
                        colorString += "0" + g.toString(16);
                    } else {
                        colorString += g.toString(16);
                    }
                    if (b < 16) {
                        colorString += "0" + b.toString(16);
                    } else {
                        colorString += b.toString(16);
                    }
                    ctx.fillStyle = colorString;
                }
            } else {
                ctx.fillStyle = this.color;
            }
        }

        ctx.roundRect(this.posX, this.posY, this.width, this.height, [this.radius]);
        ctx.fill();

        if (this.type == "button") {
            if (this.mousePressed || (this.keyboardKeys[0] != null && keysPressed.has(this.keyboardKeys[0])) || this.gamepadActivatedButton) {
                ctx.beginPath();
                ctx.fillStyle = 'white';
                ctx.roundRect(this.posX, this.posY, this.width, this.height, [this.radius]);
                ctx.fill();
            }
        } else if (this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider") {
            ctx.beginPath();
            ctx.fillStyle = 'white';
            ctx.arc(this.posX + this.width / 2 + this.joyx * (this.width / 2 - this.radius), this.posY + this.height / 2 - this.joyy * (this.height / 2 - this.radius), this.radius, 0, 2 * Math.PI);
            ctx.fill();
        } else if (this.type == "number indicator") {
            ctx.font = 50 * this.size / 215 + "px " + "Courier New";
            ctx.textAlign = "right";
            ctx.fillStyle = 'white';
            ctx.textAlign = "left";
            ctx.fillText(this.labelText, this.posX, this.posY + this.height / 2);
            ctx.textAlign = "right";
            ctx.fillText(this.vars[0].toFixed(4).substring(0, 7), this.posX + this.width - 1, this.posY + this.height - 3);
        }
    }

    nameElement() {
        let element = document.createElement("div");
        element.className = "non-highlighted-ds-list-item";

        let typeLabel = document.createElement("span");
        if (this.type == "vert. slider") {
            typeLabel.innerHTML = "vertical slider";
        } else if (this.type == "horiz. slider") {
            typeLabel.innerHTML = "horizontal slider";
        } else {
            typeLabel.innerHTML = this.type;
        }
        element.appendChild(typeLabel);

        element.onclick = () => { // whole row is clickable
            document.getElementById("ds-properties").replaceChildren(this.propertiesElement());
        }

        let deleteButton = document.createElement("button");
        deleteButton.className = "ds-list-deleteButton";
        deleteButton.innerHTML = "X";
        deleteButton.onclick = (event) => {
            event.stopPropagation();
            deleteDSItem(this);
        }
        element.appendChild(deleteButton);

        this.myNameElement = element;

        return element;
    }
    propertiesElement(redraw = true) {
        let element = document.createElement("div");

        for (let i = 0; i < DSItems.length; i++) {
            DSItems[i].highlighted = false;
        }
        this.highlighted = true;

        if (redraw) {
            this.redrawCanvas();
        }

        let row_data = document.createElement("tr");
        let row_data_label = document.createElement("td");
        row_data_label.innerHTML = this.indicator ? "input variable" : "output variable";
        row_data.appendChild(row_data_label);
        for (let i = 0; i < this.numData; i++) {
            let cell = document.createElement("td");
            let input = document.createElement("input");
            input.type = "number";
            input.min = 0;
            input.style.width = "50px";
            input.value = this.dataIndices[i];
            input.onchange = (event) => {
                this.dataIndices[i] = parseInt(event.target.value);
                input.value = this.dataIndices[i];
            };
            let inputlable = document.createElement("label");
            if (this.type == "joystick") {
                inputlable.innerHTML = (i == 0 ? "X" : "Y");
            } else {
                inputlable.innerHTML = "";
            }
            cell.appendChild(inputlable);
            cell.appendChild(input);
            row_data.appendChild(cell);
        }
        element.appendChild(row_data);
        // size
        let row_size = document.createElement("tr");
        let row_size_label = document.createElement("td");
        row_size_label.innerHTML = "size";
        row_size.appendChild(row_size_label);
        let input_size = document.createElement("input");
        input_size.type = "number";
        input_size.min = this.minorDimension;
        input_size.style.width = "50px";
        input_size.value = this.size;
        input_size.onchange = (event) => {
            let sizeVal = parseInt(event.target.value);
            if (!isNaN(sizeVal)) {
                this.size = sizeVal;
            }
            this.size = Math.max(this.size, this.minorDimension);
            input_size.value = this.size;
            this.processSize();
            this.redrawCanvas();
            this.draw();
        }
        row_size.appendChild(input_size);
        element.appendChild(row_size);

        if (this.type == "number indicator") {
            let row_textlable = document.createElement("tr");
            let row_textlable_label = document.createElement("td");
            row_textlable_label.innerHTML = "label";
            row_textlable.appendChild(row_textlable_label);
            let input_textlable = document.createElement("input");
            input_textlable.type = "text";
            input_textlable.style.width = "50px";
            input_textlable.maxLength = 7;
            input_textlable.value = this.labelText;
            input_textlable.onchange = (event) => {
                this.labelText = event.target.value;
                this.draw();
            }
            row_textlable.appendChild(input_textlable);
            element.appendChild(row_textlable);
        }

        if (this.type == "button") {
            let row_buttonVal = document.createElement("tr");
            let row_buttonVal_label = document.createElement("td");
            row_buttonVal_label.innerHTML = "value when pressed";
            row_buttonVal.appendChild(row_buttonVal_label);
            let cell_buttonVal = document.createElement("td");
            let input_buttonVal = document.createElement("input");
            input_buttonVal.type = "number";
            input_buttonVal.step = "0.01";
            input_buttonVal.value = this.buttonPressedVal;
            input_buttonVal.onchange = (event) => {
                let tempFloat = parseFloat(event.target.value);
                if (!isNaN(tempFloat)) {
                    this.buttonPressedVal = tempFloat;
                } else {
                    this.buttonPressedVal = null;
                }
                input_buttonVal.value = this.buttonPressedVal;
            }
            cell_buttonVal.appendChild(input_buttonVal);
            row_buttonVal.appendChild(cell_buttonVal);
            element.appendChild(row_buttonVal);

            let row_buttonRVal = document.createElement("tr");
            let row_buttonRVal_label = document.createElement("td");
            row_buttonRVal_label.innerHTML = "value when not pressed";
            row_buttonRVal.appendChild(row_buttonRVal_label);
            let cell_buttonRVal = document.createElement("td");
            let input_buttonRVal = document.createElement("input");
            input_buttonRVal.type = "number";
            input_buttonRVal.step = "0.01";
            input_buttonRVal.value = this.buttonReleasedVal;
            input_buttonRVal.onchange = (event) => {
                let tempFloat = parseFloat(event.target.value);
                if (!isNaN(tempFloat)) {
                    this.buttonReleasedVal = tempFloat;
                } else {
                    this.buttonReleasedVal = null;
                }
                input_buttonRVal.value = this.buttonReleasedVal;
            }
            cell_buttonRVal.appendChild(input_buttonRVal);
            row_buttonRVal.appendChild(cell_buttonRVal);
            element.appendChild(row_buttonRVal);
        }

        if (this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider" || this.type == "button") {// keyboard input and gamepad input

            let row_keys = document.createElement("tr");
            let row_keys_label = document.createElement("td");
            row_keys_label.innerHTML = "keyboard";
            row_keys.appendChild(row_keys_label);
            let cell_table = document.createElement("table");

            let r1 = document.createElement("tr");
            let r1d1 = document.createElement("td");
            let r1d2 = document.createElement("td");
            let r1d3 = document.createElement("td");
            r1.appendChild(r1d1);
            r1.appendChild(r1d2);
            r1.appendChild(r1d3);

            let r2 = document.createElement("tr");
            let r2d1 = document.createElement("td");
            let r2d2 = document.createElement("td");
            let r2d3 = document.createElement("td");
            r2.appendChild(r2d1);
            r2.appendChild(r2d2);
            r2.appendChild(r2d3);

            let r3 = document.createElement("tr");
            let r3d1 = document.createElement("td");
            let r3d2 = document.createElement("td");
            let r3d3 = document.createElement("td");
            r3.appendChild(r3d1);
            r3.appendChild(r3d2);
            r3.appendChild(r3d3);

            cell_table.appendChild(r1);
            cell_table.appendChild(r2);
            cell_table.appendChild(r3);

            for (let i = 0; i < this.numData * 2; i++) {
                let input = document.createElement("input");
                input.type = "text";
                input.style.width = "50px";
                if (this.keyboardKeys[i] == undefined) this.keyboardKeys[i] = null;
                input.value = this.keyboardKeys[i];
                input.oninput = (event) => {
                    this.keyboardKeys[i] = event.target.value;
                    this.keyboardKeys[i] = this.keyboardKeys[i].charAt(this.keyboardKeys[i].length - 1);
                    input.value = this.keyboardKeys[i];
                };
                if (this.type == "vert. slider") {
                    row_keys_label.colspan = 2;
                    if (i == 0) {
                        r1d1.appendChild(input);
                    } else if (i == 1) {
                        r2d1.appendChild(input);
                    }
                }
                if (this.type == "horiz. slider") {
                    if (i == 1) {
                        r1d1.appendChild(input);
                    } else if (i == 0) {
                        r1d2.appendChild(input);
                    }
                }
                if (this.type == "joystick") {
                    if (i == 2) {
                        r1d2.appendChild(input);
                    } else if (i == 3) {
                        r3d2.appendChild(input);
                    } else if (i == 1) {
                        r2d1.appendChild(input);
                    } else if (i == 0) {
                        r2d3.appendChild(input);
                    }
                }
                if (this.type == "button") {
                    if (i == 0) {
                        r1d2.appendChild(input);
                    }
                }
            }
            row_keys.appendChild(cell_table);
            element.appendChild(row_keys);

            let row_gamepad = document.createElement("tr");
            let row_gamepad_label = document.createElement("td");
            row_gamepad_label.innerHTML = "gamepad";
            row_gamepad.appendChild(row_gamepad_label);


            if (this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider") {
                for (let i = 0; i < this.numData; i++) {
                    let row_gamepad_axis_cell = document.createElement("td");
                    let row_gamepad_axis_button = document.createElement("button");
                    row_gamepad_axis_button.innerHTML = "gamepad " + this.gamepadAxes[2 * i] + " axis " + this.gamepadAxes[2 * i + 1];
                    row_gamepad_axis_button.onclick = () => {
                        let gp = navigator.getGamepads();
                        if (gp.length == 0) {
                            gp = navigator.getGamepads();
                        }
                        this.gamepadAxes[2 * i] = undefined;
                        this.gamepadAxes[2 * i + 1] = undefined;
                        for (let j = 0; j < gp.length; j++) {
                            if (gp[j]) {
                                for (let k = 0; k < gp[j].axes.length; k++) {
                                    if (Math.abs(gp[j].axes[k]) > 0.5) {
                                        this.gamepadAxes[2 * i] = j;
                                        this.gamepadAxes[2 * i + 1] = k;
                                        break;
                                    }
                                }
                            }
                        }
                        row_gamepad_axis_button.innerHTML = "gamepad " + this.gamepadAxes[2 * i] + " axis " + this.gamepadAxes[2 * i + 1];
                    }
                    row_gamepad_axis_cell.appendChild(row_gamepad_axis_button);
                    row_gamepad.appendChild(row_gamepad_axis_cell);
                }
                element.appendChild(row_gamepad);
            } else if (this.type == "button") {//button special case
                for (let i = 0; i < this.numData; i++) {
                    let row_gamepad_axis_cell = document.createElement("td");
                    let row_gamepad_axis_button = document.createElement("button");
                    row_gamepad_axis_button.innerHTML = "gamepad " + this.gamepadAxes[2 * i] + " button " + this.gamepadAxes[2 * i + 1];
                    row_gamepad_axis_button.onclick = () => {
                        let gp = navigator.getGamepads();
                        if (gp.length == 0) {
                            gp = navigator.getGamepads();
                        }
                        this.gamepadAxes[2 * i] = undefined;
                        this.gamepadAxes[2 * i + 1] = undefined;
                        for (let j = 0; j < gp.length; j++) {
                            if (gp[j]) {
                                for (let k = 0; k < gp[j].buttons.length; k++) {
                                    if (gp[j].buttons[k].value) {
                                        this.gamepadAxes[2 * i] = j;
                                        this.gamepadAxes[2 * i + 1] = k;
                                        break;
                                    }
                                }
                            }
                        }
                        row_gamepad_axis_button.innerHTML = "gamepad " + this.gamepadAxes[2 * i] + " button " + this.gamepadAxes[2 * i + 1];
                    }
                    row_gamepad_axis_cell.appendChild(row_gamepad_axis_button);
                    row_gamepad.appendChild(row_gamepad_axis_cell);
                }
                element.appendChild(row_gamepad);

            }
        }

        if (this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider") {
            let row_recenter = document.createElement("tr");
            let row_recenter_label = document.createElement("td");
            row_recenter_label.innerHTML = "recenter";
            row_recenter.appendChild(row_recenter_label);
            let cell_recenter = document.createElement("td");
            let recenterInput = document.createElement("input");
            recenterInput.type = "checkbox";
            recenterInput.checked = this.recenter;
            recenterInput.onchange = () => {
                this.recenter = recenterInput.checked;
            }
            cell_recenter.appendChild(recenterInput);
            row_recenter.appendChild(cell_recenter);
            element.appendChild(row_recenter);
        }

        if (this.type == "number indicator") {
            // make color pickers for high and low
            let row_colorHigh = document.createElement("tr");
            let row_colorHigh_label = document.createElement("td");
            row_colorHigh_label.innerHTML = "color high";
            row_colorHigh.appendChild(row_colorHigh_label);
            let colorHighInput = document.createElement("input");
            colorHighInput.type = "color";
            colorHighInput.value = this.colorHigh;
            colorHighInput.oninput = () => {
                this.colorHigh = colorHighInput.value;
                this.highlighted = false;
                this.draw();
            }
            let cell_colorHigh = document.createElement("td");
            cell_colorHigh.appendChild(colorHighInput);
            row_colorHigh.appendChild(cell_colorHigh);
            element.appendChild(row_colorHigh);

            let row_colorHighVal = document.createElement("tr");
            let row_colorHighVal_label = document.createElement("td");
            row_colorHighVal_label.innerHTML = "color high value";
            row_colorHighVal.appendChild(row_colorHighVal_label);
            let colorHighValInput = document.createElement("input");
            colorHighValInput.type = "number";
            colorHighValInput.value = this.colorHighVal;
            colorHighValInput.onchange = () => {
                this.colorHighVal = parseFloat(colorHighValInput.value);
                this.draw();
            }
            let cell_colorHighVal = document.createElement("td");
            cell_colorHighVal.appendChild(colorHighValInput);
            row_colorHighVal.appendChild(cell_colorHighVal);
            element.appendChild(row_colorHighVal);

            let row_colorLow = document.createElement("tr");
            let row_colorLow_label = document.createElement("td");
            row_colorLow_label.innerHTML = "color low";
            row_colorLow.appendChild(row_colorLow_label);
            let colorLowInput = document.createElement("input");
            colorLowInput.type = "color";
            colorLowInput.value = this.colorLow;
            colorLowInput.oninput = () => {
                this.colorLow = colorLowInput.value;
                this.highlighted = false;
                this.draw();
            }
            let cell_colorLow = document.createElement("td");
            cell_colorLow.appendChild(colorLowInput);
            row_colorLow.appendChild(cell_colorLow);
            element.appendChild(row_colorLow);

            let row_colorLowVal = document.createElement("tr");
            let row_colorLowVal_label = document.createElement("td");
            row_colorLowVal_label.innerHTML = "color low value";
            row_colorLowVal.appendChild(row_colorLowVal_label);
            let colorLowValInput = document.createElement("input");
            colorLowValInput.type = "number";
            colorLowValInput.value = this.colorLowVal;
            colorLowValInput.onchange = () => {
                this.colorLowVal = parseFloat(colorLowValInput.value);
                this.draw();
            }
            let cell_colorLowVal = document.createElement("td");
            cell_colorLowVal.appendChild(colorLowValInput);
            row_colorLowVal.appendChild(cell_colorLowVal);
            element.appendChild(row_colorLowVal);

        } else {
            let row_color = document.createElement("tr");
            let row_color_label = document.createElement("td");
            row_color_label.innerHTML = "color";
            row_color.appendChild(row_color_label);
            let colorInput = document.createElement("input");
            colorInput.type = "color";
            colorInput.value = this.color;
            colorInput.oninput = () => {
                this.color = colorInput.value;
                this.highlighted = false;
                this.draw();
            }
            let cell_color = document.createElement("td");
            cell_color.appendChild(colorInput);
            row_color.appendChild(cell_color);
            element.appendChild(row_color);
        }

        return element;

    }

    run(allTXData, allRXData) {
        if (this.beingEdited == false) { // running
            if (this.indicator == false) {
                const gamepad = navigator.getGamepads();
                if (this.type == "joystick" || this.type == "horiz. slider" || this.type == "vert. slider") {
                    if (this.mousePressed) {
                        for (let i = 0; i < this.numData; i++) {
                            if (this.dataIndices[i] != null) {
                                allTXData[this.dataIndices[i]] = this.vars[i];
                            }
                        }
                    } else { //axis not activated by mouse or touchscreen, it can still be controlled by the keyboard or gamepad
                        for (let i = 0; i < this.numData; i++) {
                            if (this.dataIndices[i] != null && allTXData[this.dataIndices[i]] != undefined) {
                                this.vars[i] = allTXData[this.dataIndices[i]];
                            }
                            if (this.recenter) {
                                this.vars[i] = 0;
                                allTXData[this.dataIndices[i]] = this.vars[i];
                            }
                            for (let j = 0; j < gamepad.length; j++) {
                                const gp = gamepad[j];
                                if (gp) {
                                    if (this.gamepadAxes[2 * i] == j && this.gamepadAxes[2 * i + 1] != null && gp.axes[this.gamepadAxes[2 * i + 1]] != undefined) {
                                        this.vars[i] = gp.axes[this.gamepadAxes[2 * i + 1]];
                                        if (this.type == "joystick" && i == 1) { // reverse y axis
                                            this.vars[i] = -this.vars[i];
                                        }
                                        if (this.type == "vert. slider") {
                                            this.vars[i] = -this.vars[i]; // reverse vert. slider
                                        }
                                        if (this.dataIndices[i] != null) {
                                            allTXData[this.dataIndices[i]] = this.vars[i];
                                        }
                                    }
                                }
                            }

                            if (this.keyboardKeys[i * 2] != null && keysPressed.has(this.keyboardKeys[i * 2])) {
                                this.vars[i] = 1;
                                if (this.dataIndices[i] != null) {
                                    allTXData[this.dataIndices[i]] = this.vars[i];
                                }
                            }
                            if (this.keyboardKeys[i * 2 + 1] != null && keysPressed.has(this.keyboardKeys[i * 2 + 1])) {
                                this.vars[i] = -1;
                                if (this.dataIndices[i] != null) {
                                    allTXData[this.dataIndices[i]] = this.vars[i];
                                }
                            }

                        }
                    }
                } else if (this.type == "button") { // button special case
                    if (this.mousePressed) {
                        if (this.dataIndices[0] != null && !isNaN(this.buttonPressedVal) && this.buttonPressedVal != null) {
                            allTXData[this.dataIndices[0]] = this.buttonPressedVal;
                        }
                    } else { // not activated by mouse or touchscreen, it can still be controlled by the keyboard

                        // if button is connected to a variable and controlled by a gamepad
                        this.gamepadActivatedButton = false;
                        if (this.gamepadAxes[0] != null && this.gamepadAxes[1] != null) {
                            for (let j = 0; j < gamepad.length; j++) {
                                const gp = gamepad[j];
                                if (gp) {
                                    if (this.gamepadAxes[0] == j && this.gamepadAxes[1] != null && gp.buttons[this.gamepadAxes[1]].value === 1) {
                                        this.gamepadActivatedButton = true;
                                        if (this.dataIndices[0] != null && !isNaN(this.buttonPressedVal) && this.buttonPressedVal != null) {
                                            allTXData[this.dataIndices[0]] = this.buttonPressedVal;
                                        }
                                    }
                                }
                            }
                        }

                        if (this.dataIndices[0] != null) {// if button is connected to a variable and controlled by a key
                            if (!isNaN(this.buttonReleasedVal) && this.buttonReleasedVal != null && this.gamepadActivatedButton == false && !keysPressed.has(this.keyboardKeys[0])) {
                                allTXData[this.dataIndices[0]] = this.buttonReleasedVal;
                            }
                            if (!isNaN(this.buttonPressedVal) && this.buttonPressedVal != null && this.keyboardKeys[0] != null && keysPressed.has(this.keyboardKeys[0])) {
                                allTXData[this.dataIndices[0]] = this.buttonPressedVal;
                            }
                        }

                    }
                }

                if (this.type == "joystick") {
                    if (this.vars[0] > 1) this.vars[0] = 1;
                    if (this.vars[0] < -1) this.vars[0] = -1;
                    if (this.vars[1] > 1) this.vars[1] = 1;
                    if (this.vars[1] < -1) this.vars[1] = -1;
                    this.joyx = this.vars[0];
                    this.joyy = this.vars[1];
                } else if (this.type == "horiz. slider") {
                    if (this.vars[0] > 1) this.vars[0] = 1;
                    if (this.vars[0] < -1) this.vars[0] = -1;
                    this.joyx = this.vars[0];
                }
                else if (this.type == "vert. slider") {
                    if (this.vars[0] > 1) this.vars[0] = 1;
                    if (this.vars[0] < -1) this.vars[0] = -1;
                    this.joyy = this.vars[0];
                }

            } else {//indicator
                for (let i = 0; i < this.numData; i++) {
                    if (this.dataIndices[i] != null && allRXData[this.dataIndices[i]] != undefined) {
                        this.vars[i] = allRXData[this.dataIndices[i]];
                    }
                }
            }

            this.draw();
        }
    }

    removeEventListeners() {
        this.dsCanvas.removeEventListener('mousedown', this.onMouseDownBound);
        this.dsCanvas.removeEventListener('mouseup', this.onMouseUpBound);
        this.dsCanvas.removeEventListener('mousemove', this.onMouseMoveBound);
        this.dsCanvas.removeEventListener('touchstart', this.onTouchStartBound);
        this.dsCanvas.removeEventListener('touchend', this.onTouchEndBound);
        this.dsCanvas.removeEventListener('touchmove', this.onTouchMoveBound);
    }

}

var driverstationEditable = false;
function toggleEditDriverstation() {
    keysPressed.clear();
    driverstationEditable = !driverstationEditable;
    document.getElementById("toggleEditDriverstation").innerHTML = !driverstationEditable ? "Edit Driverstation" : "Run Driverstation";
    document.getElementById("ds-edit-ui").hidden = !driverstationEditable;
    if (driverstationEditable) {
        document.getElementById("robot-enabled").disabled = true;
        document.getElementById("robot-enabled").checked = false;
    } else {
        document.getElementById("robot-enabled").disabled = false;
    }
    for (let i = 0; i < DSItems.length; i++) {
        DSItems[i].beingEdited = driverstationEditable;
        DSItems[i].draw();
    }
}
var configEditable = false;
function toggleEditConfig() {
    configEditable = !configEditable;
    document.getElementById("toggleEditConfig").innerHTML = !configEditable ? "Configure Robot" : "Close Robot Configuration";
    document.getElementById("config-edit").style.visibility = configEditable ? "visible" : "hidden";
    document.getElementById("config-status").innerHTML = "";
    document.getElementById("config-status").style.backgroundColor = "lightgrey";

}

function getAndDownloadData(path) {
    fetch(path)
        .then(response => response.text())
        .then(data => downloadFile(data, 'data.txt'));
}

function downloadFile(input, fileName) {
    const link = document.createElement('a');
    link.href = URL.createObjectURL(new Blob([input]));
    link.download = fileName;
    document.body.append(link);
    link.click();
    link.remove();
}

function deleteDSItem(item) {
    DSItems.splice(DSItems.indexOf(item), 1);
    document.getElementById("ds-properties").replaceChildren([]);
    item.removeEventListeners();

    clearDSItems(false);
    refreshDSItems();
}

function addDSItem(_type) {
    const item = new DSItem(document.getElementById("ds"), { type: _type });
    item.beingEdited = driverstationEditable;
    DSItems.push(item);
    document.getElementById("ds-list").appendChild(item.nameElement())
    document.getElementById("ds-properties").replaceChildren(item.propertiesElement());
}
function clearDSItems(deletePermanently = true) {
    const list = document.getElementById("ds-list");
    while (list.firstChild) {
        list.removeChild(list.firstChild);
    }

    const plist = document.getElementById("ds-properties");
    while (plist.firstChild) {
        plist.removeChild(plist.firstChild);
    }

    if (deletePermanently) {
        for (let i = 0; i < DSItems.length; i++) {
            DSItems[i].removeEventListeners();
        }
        DSItems = [];
    }
    const ctx = document.getElementById("ds").getContext('2d');
    ctx.clearRect(0, 0, document.getElementById("ds").width, document.getElementById("ds").height);
}
function refreshDSItems() {
    for (let i = 0; i < DSItems.length; i++) {
        document.getElementById("ds-list").appendChild(DSItems[i].nameElement());
        DSItems[i].draw();
    }
}

function downloadUIData() {
    let data = { "UIdata": [] };
    for (let i = 0; i < DSItems.length; i++) {
        data["UIdata"].push(DSItems[i].jsonify());
    }
    downloadFile(JSON.stringify(data), 'UIdata.json');
}

function uploadUIData() {
    if (driverstationEditable == false) {
        return;
    }
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = e => {
        const file = e.target.files[0];
        const reader = new FileReader();
        reader.onload = e => {
            const data = JSON.parse(e.target.result);
            clearDSItems();
            for (let i = 0; i < data.UIdata.length; i++) {
                const item = new DSItem(document.getElementById("ds"), data.UIdata[i]);
                item.beingEdited = driverstationEditable;
                DSItems.push(item);
                document.getElementById("ds-list").appendChild(item.nameElement());
            }
        }
        reader.readAsText(file);
    }
    input.click();
}

let webs;

function disconnect() {
    if (webs != null) {
        webs.close();
    }
    webs = null;
    document.getElementById("robot-enabled").checked = false;
    document.getElementById("connection-status").innerHTML = "Disconnected";
}
let lastPingTime;
function connect() {
    document.getElementById("connection-status").innerHTML = "Connecting...";
    if (webs != null) {
        if (webs != null) {
            webs.close();
        }
        webs = null;
    }
    webs = new WebSocket('/control');
    webs.onerror = function (event) {
        document.getElementById("robot-enabled").checked = false;
        document.getElementById("connection-status").innerHTML = "connection lost! try pressing connect again";
    };
    lastPingTime = Date.now();
    webs.onmessage = function (event) {
        eventData = event;
        event.data.bytes().then(function (data) {
            var rxByteArray = new Uint8Array(data);
            var newrxdata = new Float32Array(rxByteArray.buffer);
            rxdata = [];
            for (var i = 0; i < newrxdata.length; i++) {
                rxdata[i] = newrxdata[i];
            }

            pingTime = Date.now() - lastPingTime;
            lastPingTime = Date.now();
            document.getElementById("connection-status").innerHTML = "Connected, ping: " + pingTime + "ms";
        });
        txMessage();
    };
    setTimeout(() => {
        txMessage();
    }, 5000);
}

// TODO: handle lost websocket connections

function txMessage() {
    if (webs.readyState) {

        // set datatxlen to the largest value of a DSItem.dataIndices
        let datatxlen = 0;
        for (let i = 0; i < DSItems.length; i++) {
            for (let j = 0; j < DSItems[i].dataIndices.length; j++) {
                if (DSItems[i].dataIndices[j] + 1 > datatxlen) {
                    datatxlen = DSItems[i].dataIndices[j] + 1;
                }
            }
        }
        if (datatxlen > 255) {
            datatxlen = 255;
        }

        var txdatafloats = new Float32Array(datatxlen);
        for (var i = 0; i < datatxlen; i++) {
            if (txdata[i] == undefined) {
                txdatafloats[i] = 0;
            } else {
                txdatafloats[i] = txdata[i];
            }
        }
        var txByteArray = new Uint8Array(txdatafloats.buffer);
        var newTxByteArray = new Uint8Array(txByteArray.length + 2);
        newTxByteArray[0] = document.getElementById("robot-enabled").checked && !driverstationEditable; // ENABLED
        newTxByteArray[1] = datatxlen;
        newTxByteArray.set(txByteArray, 2); // Copy the existing data
        webs.send(newTxByteArray);
    } else {
        document.getElementById("robot-enabled").checked = false;
        document.getElementById("connection-status").innerHTML = "error, try pressing connect again";
    }
}

function saveUI() {
    const UIDataToSend = JSON.stringify({ "UIdata": DSItems });

    const UIDataToSendEncoded = new URLSearchParams({ UIdata: UIDataToSend });

    fetch('/saveUI', {
        method: "post",
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: UIDataToSendEncoded
    }).then(response => {
        console.log(response);
    });
}
function loadUI(fromURL = '/loadUI.json') {
    fetch(fromURL)
        .then(response => {
            if (response.ok) {
                return response.text();
            } else {
                throw new Error();
            }
        })
        .then(data => {
            return JSON.parse(data.substring(0, data.lastIndexOf('}') + 1));
        })
        .then(data => {
            console.log(data);
            if (data.UIdata != undefined) {
                clearDSItems();
                for (let i = 0; i < data.UIdata.length; i++) {
                    const item = new DSItem(document.getElementById("ds"), data.UIdata[i]);
                    item.beingEdited = driverstationEditable;
                    DSItems.push(item);
                    document.getElementById("ds-list").appendChild(item.nameElement());
                }
            }
        })
        .catch(() => {
        });
}

function saveWifiSettings() {
    let wifiData = {
        ssid: document.getElementById("wifi-ssid").value,
        password: document.getElementById("wifi-password").value,
        hostname: document.getElementById("wifi-hostname").value,
        mode: document.getElementById("wifi-mode").checked ? 0 : 1
    };
    const wifiDataToSendEncoded = new URLSearchParams(wifiData);
    fetch('/saveWifiSettings', {
        method: "post",
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: wifiDataToSendEncoded
    }).then(response => {
        console.log(response);
        if (wifiData.hostname != "") {
            window.location = "http://" + wifiData.hostname;
        }
    });
}

function loadWifiSettings() {
    fetch('/loadWifiSettings.json')
        .catch((error) => {
        })
        .then(response => response.json())
        .catch((error) => {
        })
        .then(data => {
            if (data.ssid == undefined || data.password == undefined || data.hostname == undefined || data.mode == undefined) {
                document.getElementById("wifi-ssid").value = "";
                document.getElementById("wifi-password").value = "";
                document.getElementById("wifi-hostname").value = "rcmv3";
                document.getElementById("wifi-mode").checked = false;
            } else {
                document.getElementById("wifi-ssid").value = data.ssid;
                document.getElementById("wifi-password").value = data.password;
                document.getElementById("wifi-hostname").value = data.hostname;
                document.getElementById("wifi-mode").checked = data.mode != 1;
            }
        }).catch((error) => {
        });
}

var boardInfo = {};

var activeComponentList = [];

class ActiveComponent {
    constructor(jsonData, index, setParameterVals) {
        this.index = index;
        this.typeid = jsonData.type;
        this.typename = jsonData.name;
        this.username = jsonData.username == undefined ? "" : jsonData.username;

        if (setParameterVals == false) {
            this.inputs = Array(jsonData.num_inputs).fill(-1);
            this.outputs = Array(jsonData.num_outputs).fill(-1);

            this.parameters = [];
            for (let i = 0; i < boardInfo.potential_components[this.typeid]["parameters"].length; i++) {
                switch (boardInfo.potential_components[this.typeid]["parameters"][i].type) {
                    // TODO: if there is a parameter_preset with a matching mask maybe use it instead?
                    case "pin":
                        this.parameters.push(null);
                        break;
                    case "ComponentIndex":
                        this.parameters.push(-1);
                        break;
                    case "ComponentInputIndex":
                        this.parameters.push(-1);
                        break;
                    case "WhichWire":
                        this.parameters.push(0);
                        break;
                    case "int":
                        this.parameters.push(0);
                        break;
                    case "float":
                        this.parameters.push(0.0);
                        break;
                    case "bool":
                        this.parameters.push(false);
                        break;
                    case "TMC7300IC":
                        this.parameters.push(0);
                        break;
                    case "BSED":
                        this.parameters.push(0);
                        break;
                    case "WhichWire":
                        this.parameters.push(0);
                        break;
                    default:
                        console.log("unknown parameter type");
                        this.parameters.push(null);
                        break;
                }
            }
        } else {
            this.parameters = jsonData.parameters;
            this.inputs = jsonData.inputs;
            this.outputs = jsonData.outputs;
        }

        this.element = document.createElement("div");

        this.updateHTMLElement();

        document.getElementById("active-components").appendChild(this.element);
    }
    updateHTMLElement() {
        this.element.replaceChildren();

        this.element.className = "component-element";

        let indexSpan = document.createElement("span");
        indexSpan.innerHTML = this.index;
        this.element.appendChild(indexSpan);

        this.element.onclick = () => { this.openProperties(); };

        let upButton = document.createElement("button");
        upButton.innerHTML = "&#9650;";
        upButton.onclick = (event) => {
            event.stopPropagation();
            document.getElementById("component-properties").replaceChildren();
            if (this.index > 0) {
                // swap this.element with the one above it in activeComponentList and this.element.parentElement update the indices
                let above = activeComponentList[this.index - 1];
                activeComponentList[this.index - 1] = this;
                activeComponentList[this.index] = above;
                this.index--;
                above.index++;
                this.updateHTMLElement();
                above.updateHTMLElement();
                this.element.parentElement.insertBefore(this.element, above.element);
            }
        }
        this.element.appendChild(upButton);

        let downButton = document.createElement("button");
        downButton.innerHTML = "&#9660;";
        downButton.onclick = (event) => {
            event.stopPropagation();
            document.getElementById("component-properties").replaceChildren();
            if (this.index < activeComponentList.length - 1) {
                let below = activeComponentList[this.index + 1];
                activeComponentList[this.index + 1] = this;
                activeComponentList[this.index] = below;
                this.index++;
                below.index--;
                this.updateHTMLElement();
                below.updateHTMLElement();
                this.element.parentElement.insertBefore(this.element, below.element.nextSibling);
            }
        }
        this.element.appendChild(downButton);

        let deleteButton = document.createElement("button");
        deleteButton.className = "component-delete-button";
        deleteButton.innerHTML = "X";
        deleteButton.onclick = (event) => {
            event.stopPropagation();
            this.element.remove();
            activeComponentList.splice(activeComponentList.indexOf(this), 1);
            document.getElementById("component-properties").replaceChildren();
        }
        this.element.appendChild(deleteButton);

        let nameSpan = document.createElement("span");
        nameSpan.style = "float: right";
        nameSpan.innerHTML = this.username;
        this.element.appendChild(nameSpan);

        let typeSpan = document.createElement("div");
        typeSpan.innerHTML = this.typename;
        this.element.appendChild(typeSpan);
    }
    // componentType is an array of strings that the component needs to be compatible with, or [true] if it can be compatible with any component
    createHelperForComponentThatNeedsComponent(componentType, element, constructorParameter, i) {
        let label = document.createElement("label");
        let input = document.createElement("input");
        input.style.width = "50px";
        label.innerHTML = constructorParameter.name;
        input.type = "number";
        input.step = "1";
        input.value = this.parameters[i];
        input.onchange = (event) => {
            this.parameters[i] = parseInt(event.target.value);
            let ComponentInputIndexHelperRefreshButton = document.getElementById("ComponentInputIndexHelperRefreshButton");
            if (ComponentInputIndexHelperRefreshButton) {
                ComponentInputIndexHelperRefreshButton.click();
            }
        }
        element.appendChild(label);
        element.appendChild(document.createElement("br"));
        element.appendChild(input);

        let helper = document.createElement("select");
        let defaultOption = document.createElement("option");
        defaultOption.value = null;
        defaultOption.textContent = "select";
        helper.appendChild(defaultOption);
        for (let j = 0; j < activeComponentList.length; j++) {
            if (componentType.includes(activeComponentList[j].typename) || componentType.includes(true)) {
                let option = document.createElement("option");
                option.value = j;
                option.textContent = activeComponentList[j].username;
                helper.appendChild(option);
            }
        }
        helper.value = this.parameters[i];
        helper.onchange = (event) => {
            if (event.target.value) {
                this.parameters[i] = parseInt(event.target.value);
                input.value = this.parameters[i];
                let ComponentInputIndexHelperRefreshButton = document.getElementById("ComponentInputIndexHelperRefreshButton");
                if (ComponentInputIndexHelperRefreshButton) {
                    ComponentInputIndexHelperRefreshButton.click();
                }
            }
        }
        element.appendChild(helper);
    }
    openProperties() {
        document.getElementById("component-properties").replaceChildren();
        // user name
        let usernameInputElement = document.createElement("input");
        usernameInputElement.type = "text";
        usernameInputElement.value = this.username;
        usernameInputElement.onchange = (event) => {
            this.username = event.target.value;
            this.updateHTMLElement();
        }

        document.getElementById("component-properties").appendChild(usernameInputElement);


        let componentHelperText = document.createElement("div");
        switch (this.typename) {
            case "Mixer": {
                componentHelperText.innerHTML = "output = Ax + By + Cz";
            } break;
            case "Servo Controller": {
                // add link to https://joshua-8.github.io/JMotor/class_j_servo_controller.html to the componentHelperText and open in a new tab
                let link = document.createElement("a");
                link.href = "https://joshua-8.github.io/JMotor/class_j_servo_controller.html";
                link.target = "_blank";
                link.rel = "noopener noreferrer";
                link.innerHTML = "Servo Controller Documentation";
                componentHelperText.appendChild(link);
            } break;
        }
        document.getElementById("component-properties").appendChild(componentHelperText);

        // display all constructor parameters
        for (let i = 0; i < boardInfo.potential_components[this.typeid]["parameters"].length; i++) {
            let constructorParameter = boardInfo.potential_components[this.typeid]["parameters"][i];
            let element = document.createElement("div");
            switch (constructorParameter.type) {
                case "TMCChipAddress":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        input.style.width = "50px";
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.step = "1";
                        input.min = "0";
                        input.max = "3";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseInt(event.target.value);
                        }
                        element.appendChild(label);
                        element.appendChild(input);
                    }
                    break;
                case "pin":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.step = "1";
                        input.min = "0";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseInt(event.target.value);
                        }
                        element.appendChild(label);
                        element.appendChild(input);

                        let helper = document.createElement("select");
                        let defaultOption = document.createElement("option");
                        defaultOption.value = null;
                        defaultOption.textContent = "select pin";
                        helper.appendChild(defaultOption);
                        for (let j = 0; j < loadedParameterPreset.length; j++) {
                            if (loadedParameterPreset[j].type == "pin") {
                                let option = document.createElement("option");
                                option.value = loadedParameterPreset[j].value;
                                option.textContent = loadedParameterPreset[j].name;
                                helper.appendChild(option);
                            }
                        }
                        helper.value = this.parameters[i];
                        helper.onchange = (event) => {
                            if (event.target.value) {
                                this.parameters[i] = parseInt(event.target.value);
                                input.value = this.parameters[i];
                            }
                        }
                        element.appendChild(helper);
                    }
                    break;
                case "TMC7300IC":
                    {
                        this.createHelperForComponentThatNeedsComponent(["TMC7300 IC"], element, constructorParameter, i);
                    }
                    break;
                case "ComponentIndex":
                    {
                        this.createHelperForComponentThatNeedsComponent([true], element, constructorParameter, i);
                    }
                    break;
                case "ComponentInputIndex":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.step = "1";
                        input.style.width = "50px";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseInt(event.target.value);
                            let ComponentInputIndexHelperRefreshButton = document.getElementById("ComponentInputIndexHelperRefreshButton");
                            if (ComponentInputIndexHelperRefreshButton) {
                                ComponentInputIndexHelperRefreshButton.click();
                            }
                        }
                        element.appendChild(label);
                        element.appendChild(document.createElement("br"));
                        element.appendChild(input);

                        let refreshHelperButton = document.createElement("button");
                        let helper = document.createElement("select");
                        refreshHelperButton.hidden = true;
                        refreshHelperButton.id = "ComponentInputIndexHelperRefreshButton";
                        refreshHelperButton.innerHTML = "refresh helper";
                        refreshHelperButton.onclick = () => {
                            if (boardInfo.potential_components[this.typeid]["parameters"][i - 1].type === "ComponentIndex") {
                                let componentIndex = this.parameters[i - 1];
                                if (activeComponentList[componentIndex] != undefined) {
                                    helper.replaceChildren();
                                    let defaultOption = document.createElement("option");
                                    defaultOption.value = -1;
                                    defaultOption.textContent = "select input";
                                    helper.appendChild(defaultOption);
                                    for (let j = 0; j < boardInfo.potential_components[activeComponentList[componentIndex].typeid]["inputs"].length; j++) {
                                        let option = document.createElement("option");
                                        option.value = j;
                                        option.textContent = boardInfo.potential_components[activeComponentList[componentIndex].typeid]["inputs"][j].name;
                                        helper.appendChild(option);
                                    }
                                    helper.value = this.parameters[i];
                                    helper.onchange = (event) => {
                                        if (event.target.value) {
                                            this.parameters[i] = parseInt(event.target.value);
                                            input.value = this.parameters[i];
                                        }
                                    }
                                }
                            }
                        }
                        element.appendChild(helper);
                        element.appendChild(refreshHelperButton);
                    }
                    break;
                case "bool":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "checkbox";
                        input.checked = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = event.target.checked;
                        }
                        element.appendChild(label);
                        element.appendChild(input);
                    }
                    break;
                case "int":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.step = "1";
                        input.style.width = "50px";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseInt(event.target.value);
                        }
                        element.appendChild(label);
                        element.appendChild(input);

                        let helper = document.createElement("select");
                        let defaultOption = document.createElement("option");
                        defaultOption.value = null;
                        defaultOption.textContent = "select";
                        helper.appendChild(defaultOption);
                        for (let j = 0; j < loadedParameterPreset.length; j++) {
                            if (loadedParameterPreset[j].type == "int" && loadedParameterPreset[j].mask.includes(constructorParameter.name)) {
                                let option = document.createElement("option");
                                option.value = loadedParameterPreset[j].value;
                                option.textContent = loadedParameterPreset[j].name;
                                helper.appendChild(option);
                            }
                        }
                        helper.value = this.parameters[i];
                        helper.onchange = (event) => {
                            if (event.target.value) {
                                this.parameters[i] = parseInt(event.target.value);
                                input.value = this.parameters[i];
                            }
                        }
                        if (helper.children.length > 1) {
                            element.appendChild(helper);
                        }

                    }
                    break;
                case "float":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.style.width = "50px";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseFloat(event.target.value);
                        }
                        element.appendChild(label);
                        element.appendChild(input);


                        let helper = document.createElement("select");
                        let defaultOption = document.createElement("option");
                        defaultOption.value = null;
                        defaultOption.textContent = "select";
                        helper.appendChild(defaultOption);
                        for (let j = 0; j < loadedParameterPreset.length; j++) {
                            if (loadedParameterPreset[j].type == "float" && loadedParameterPreset[j].mask.includes(constructorParameter.name)) {
                                let option = document.createElement("option");
                                option.value = loadedParameterPreset[j].value;
                                option.textContent = loadedParameterPreset[j].name;
                                helper.appendChild(option);
                            }
                        }
                        helper.value = this.parameters[i];
                        helper.onchange = (event) => {
                            if (event.target.value) {
                                this.parameters[i] = parseFloat(event.target.value);
                                input.value = this.parameters[i];
                            }
                        }
                        if (helper.children.length > 1) {
                            element.appendChild(helper);
                        }
                    }

                    break;
                case "BSED":
                    {
                        this.createHelperForComponentThatNeedsComponent(["ByteSizedEncoderDecoder"], element, constructorParameter, i);
                    }
                    break;
                case "WhichWire":
                    {
                        let label = document.createElement("label");
                        let input = document.createElement("input");
                        label.innerHTML = constructorParameter.name;
                        input.type = "number";
                        input.step = "1";
                        input.value = this.parameters[i];
                        input.onchange = (event) => {
                            this.parameters[i] = parseInt(event.target.value);
                        }
                        element.appendChild(label);
                        element.appendChild(input);
                    }
                    break;
                case "Servo Driver":
                    {
                        this.createHelperForComponentThatNeedsComponent(["Motor Driver Servo ESP32"], element, constructorParameter, i);
                    }
                    break;
                default:
                    console.log("unknown parameter input type");
                    break;
            }
            document.getElementById("component-properties").appendChild(element);
        }

        if (this.typename == "Motor Driver HBridge ESP32") {
            let helper = document.createElement("select");
            let defaultOption = document.createElement("option");
            defaultOption.value = null;
            defaultOption.textContent = "select port";
            helper.appendChild(defaultOption);
            for (let j = 0; j < loadedParameterPreset.length; j++) {
                if (loadedParameterPreset[j].type == "HBridgeESP32") {
                    let option = document.createElement("option");
                    option.value = loadedParameterPreset[j].value;
                    option.textContent = loadedParameterPreset[j].name;
                    helper.appendChild(option);
                }
            }
            helper.onchange = (event) => {
                if (event.target.value) {
                    let params = event.target.value.split(",");
                    this.parameters = [];
                    for (let i = 0; i < params.length; i++) {
                        this.parameters.push(parseInt(params[i]));
                    }
                    this.openProperties();
                }
            }
            document.getElementById("component-properties").appendChild(helper);
        }

        // display all inputs
        let inputsElement = document.createElement("div");
        let inputsTitle = document.createElement("label");
        inputsTitle.innerHTML = "control variables: ";
        inputsElement.appendChild(inputsTitle);
        for (let i = 0; i < this.inputs.length; i++) {
            let element = document.createElement("div");
            let label = document.createElement("label");
            label.innerHTML = boardInfo.potential_components[this.typeid]["inputs"][i].name + ": ";
            element.appendChild(label);
            let input = document.createElement("input");
            input.type = "number";
            input.value = this.inputs[i];
            input.onchange = (event) => {
                this.inputs[i] = parseInt(event.target.value);
            }
            element.appendChild(input);
            inputsElement.appendChild(element);
        }
        document.getElementById("component-properties").appendChild(inputsElement);


        // display all outputs
        let outputsElement = document.createElement("div");
        let outputsTitle = document.createElement("label");
        outputsTitle.innerHTML = "telemetry variables: ";
        outputsElement.appendChild(outputsTitle);
        for (let i = 0; i < this.outputs.length; i++) {
            let element = document.createElement("div");
            let label = document.createElement("label");
            label.innerHTML = boardInfo.potential_components[this.typeid]["outputs"][i].name + ": ";
            element.appendChild(label);
            let input = document.createElement("input");
            input.type = "number";
            input.value = this.outputs[i];
            input.onchange = (event) => {
                this.outputs[i] = parseInt(event.target.value);
            }
            element.appendChild(input);
            outputsElement.appendChild(element);
        }
        document.getElementById("component-properties").appendChild(outputsElement);

    }
    jsonify() {
        let processedParameters = [];
        for (let i = 0; i < this.parameters.length; i++) {
            processedParameters.push(this.parameters[i]);
        }

        return {
            type: this.typeid,
            name: this.typename,
            username: this.username,
            inputs: this.inputs,
            outputs: this.outputs,
            parameters: processedParameters
        };
    }
};

function updateBoardInfoUI() {
    document.getElementById("active-components").replaceChildren();
    document.getElementById("component-properties").replaceChildren();

    document.getElementById("potential-components").replaceChildren();
    for (let i = 0; i < boardInfo.potential_components.length; i++) {
        let component = boardInfo.potential_components[i];
        let element = document.createElement("div");
        element.innerHTML = component.name;
        element.onclick = () => {
            document.getElementById("component-properties").replaceChildren();
            activeComponentList.push(new ActiveComponent(component, activeComponentList.length, false));
        }
        document.getElementById("potential-components").appendChild(element);
    }
}

function clearConfig() {
    activeComponentList = [];
    document.getElementById("active-components").replaceChildren();
    document.getElementById("component-properties").replaceChildren();
}

function saveConfigToFile() {
    let componentDataToSend = { "components": [] };
    for (let i = 0; i < activeComponentList.length; i++) {
        componentDataToSend["components"].push(activeComponentList[i].jsonify());
    }
    componentDataToSend = JSON.stringify(componentDataToSend);
    downloadFile(componentDataToSend, 'config.json');
}
function loadConfigFromFile() {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = e => {
        const file = e.target.files[0];
        const reader = new FileReader();
        reader.onload = e => {
            const dataFile = JSON.parse(e.target.result);
            const data = dataFile["components"];
            for (let i = 0; i < data.length; i++) {
                activeComponentList.push(new ActiveComponent(data[i], activeComponentList.length, true));
            }
        }
        reader.readAsText(file);
    }
    input.click();
}

function robotSaveConfig() {
    document.getElementById("config-status").innerHTML = "Saving...";
    document.getElementById("config-status").style.backgroundColor = "yellow";

    fetch('/saveConfigToMemory').then(response => {
        response.text().then((text) => {
            if (text == "OK") {
                document.getElementById("config-status").innerHTML = "Saved";
                document.getElementById("config-status").style.backgroundColor = "lightgreen";
            }
        }
        );
    });
}

function loadBoardInfo(errorCallback) {
    fetch('/loadBoardInfo.json')
        .then(response => response.json())
        .then(data => {
            boardInfo = data;
            activeComponentList = [];
            updateBoardInfoUI()
        }).catch((error) => {
            errorCallback();
        });
}

function loadConfig(fromURL = '/loadConfig.json') {
    document.getElementById("config-status").innerHTML = "Loading...";
    document.getElementById("config-status").style.backgroundColor = "yellow";
    fetch(fromURL)
        .then(response => response.json())
        .then(data => {
            document.getElementById("config-status").innerHTML = "reloaded";
            document.getElementById("config-status").style.backgroundColor = "lightgreen";
            document.getElementById("active-components").replaceChildren();
            document.getElementById("component-properties").replaceChildren();
            activeComponentList = [];
            for (let i = 0; i < data.components.length; i++) {
                activeComponentList.push(new ActiveComponent(data.components[i], activeComponentList.length, true));
            }
        }).catch((error) => {
        });
}
function saveConfig() {
    document.getElementById("config-status").innerHTML = "saving...";
    document.getElementById("config-status").style.backgroundColor = "yellow";

    let componentDataToSend = [];
    for (let i = 0; i < activeComponentList.length; i++) {
        componentDataToSend.push(activeComponentList[i].jsonify());
    }
    componentDataToSend = JSON.stringify(componentDataToSend);
    const robotDataToSendEncoded = new URLSearchParams({ components: componentDataToSend });

    fetch('/saveConfig', {
        method: "post",
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: robotDataToSendEncoded
    }).then(response => {
        response.text().then((text) => {
            if (text == "OK") {
                robotSaveConfig();
                document.getElementById("config-status").innerHTML = "Sent config";
                document.getElementById("config-status").style.backgroundColor = "lightgreen";
            } else {
                document.getElementById("config-status").style.backgroundColor = "#ff9999";
                document.getElementById("config-status").innerHTML = text;
            }
        });
    });
}

async function loadPresets() {
    fetch('/presets/presets.json').then(response => response.json()).then(presets => {

        document.getElementById("config-presets-list").replaceChildren();

        config_presets = presets.presets.config_presets;
        for (let i = 0; i < config_presets.length; i++) {
            let element = document.createElement("div");
            element.innerHTML = config_presets[i].substring(0, config_presets[i].length - 5);// remove .json

            element.onclick = () => {
                document.getElementById("config-status").innerHTML = "Loading...";
                document.getElementById("config-status").style.backgroundColor = "yellow"; //TODO: CLEAN UP CONFIG-STATUS WINDOW SETTING CODE
                fetch('/presets/config_presets/' + config_presets[i]).then(response => response.json()).then(configComponents => {
                    for (let j = 0; j < configComponents.length; j++) {
                        activeComponentList.push(new ActiveComponent(configComponents[j], activeComponentList.length, true));
                    }
                    document.getElementById("config-status").innerHTML = "Loaded Preset";
                    document.getElementById("config-status").style.backgroundColor = "lightgreen";
                });
            }
            document.getElementById("config-presets-list").appendChild(element);
        }

        document.getElementById("board-type-selector").replaceChildren();
        let parameter_presets = presets.presets.parameter_presets;
        let option = document.createElement("option");
        option.textContent = "select RCM Board";
        option.value = null;
        document.getElementById("board-type-selector").appendChild(option);
        for (let i = 0; i < parameter_presets.length; i++) {
            let option = document.createElement("option");
            option.textContent = parameter_presets[i].substring(0, parameter_presets[i].length - 5);
            option.value = parameter_presets[i];
            document.getElementById("board-type-selector").appendChild(option);
        }
        document.getElementById("board-type-selector").onchange = (event) => {
            let preset = event.target.value;
            if (preset && preset != "null") {
                fetch('/presets/parameter_presets/' + preset).then(response => response.json()).then(data => {
                    loadedParameterPreset = data;
                    document.getElementById("component-properties").replaceChildren();
                    miscConfigInfo["board"] = preset;
                    saveMiscConfigInfo();
                });
            } else {
                loadedParameterPreset = [];
            }
        }
    });

    fetch('/loadMiscConfigInfo.json')
        .then(response => response.text())
        .then(data => {
            data = data.substring(0, data.lastIndexOf('}') + 1);
            return JSON.parse(data);// trim null character from end of string
        }
        ).then(data => {
            miscConfigInfo = data;
            if (miscConfigInfo["board"] != undefined) {
                document.getElementById("board-type-selector").value = miscConfigInfo["board"];
                document.getElementById("board-type-selector").dispatchEvent(new Event('change'));
            }
        });
}

function saveMiscConfigInfo() {
    document.getElementById("config-status").innerHTML = "saving misc config info...";
    document.getElementById("config-status").style.backgroundColor = "yellow";

    dataToSend = JSON.stringify(miscConfigInfo);
    const dataToSendEncoded = new URLSearchParams({ miscConfigInfo: dataToSend });

    fetch('/saveMiscConfigInfo', {
        method: "post",
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: dataToSendEncoded
    }).then(response => {
        response.text().then((text) => {
            if (text == "OK") {
                robotSaveConfig();
                document.getElementById("config-status").innerHTML = "Sent config info";
                document.getElementById("config-status").style.backgroundColor = "lightgreen";
            } else {
                document.getElementById("config-status").style.backgroundColor = "#ff9999";
                document.getElementById("config-status").innerHTML = text;
            }
        });
    });
}

function loadProject() {
    let projectName = document.getElementById("project-name").value;
    // TODO: option to append instead of replace
    let projectUrl = document.getElementById("projecturl").value;
    let projectFullURL = projectUrl + projectName;
    loadUI(projectFullURL + "/UIdata.json");
    loadConfig(projectFullURL + "/config.json");
}
