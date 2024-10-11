let DSItems = [];
class DSItem {
    constructor(_dsCanvas, _type) {
        this.dsCanvas = _dsCanvas;

        this.type = _type;
        this.size = 250;
        this.color = "#808080"; // must be hex code

        if (this.type === "joystick") {
            this.width = this.size;
            this.height = this.size;
            this.radius = 25;
        }
        else if (this.type === "hslider") {
            this.width = this.size;
            this.height = 50;
            this.radius = 25;
        }
        else if (this.type === "vslider") {
            this.width = 50;
            this.height = this.size;
            this.radius = 25;
        } else if (this.type === "button") {
            this.size = 50;//TODO: DELETE
            this.width = this.size;
            this.height = this.size;
            this.radius = 0;
        }

        //TODO: add recenter option
        //TODO: add min and max options?

        this.buttonPressedVal = null;
        this.buttonReleasedVal = null;

        this.posX = 0;
        this.posY = 0;

        this.mousePressed = false;
        this.highlighted = false;
        this.myNameElement = null;

        this.beingEdited = false;

        this.joyx = 0;
        this.joyy = 0;

        this.numData = 0;
        this.dataIndices = [];

        this.vars = [];
        if (this.type === "joystick") {
            this.numData = 2;
            this.vars = [0.0, 0.0];
            this.dataIndices = [null, null];
        } else if (this.type === "hslider") {
            this.numData = 1;
            this.vars = [0.0];
            this.dataIndices = [null];
        } else if (this.type === "vslider") {
            this.numData = 1;
            this.vars = [0.0];
            this.dataIndices = [null];
        } else if (this.type === "button") {
            this.numData = 1;
            this.vars = [0];
            this.dataIndices = [null];
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
        if (this.beingEdited === false && this.type === "button") {
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
            } else {
                const rect = this.dsCanvas.getBoundingClientRect();
                if (this.type === "joystick" || this.type === "hslider") {
                    this.joyx = (x - rect.left - this.posX - this.width / 2) / (this.width / 2 - this.radius);
                } else {
                    this.joyx = 0;
                }
                if (this.type === "joystick" || this.type === "vslider") {
                    this.joyy = -(y - rect.top - this.posY - this.height / 2) / (this.height / 2 - this.radius);
                } else {
                    this.joyy = 0;
                }
                if (this.joyx > 1) this.joyx = 1;
                if (this.joyx < -1) this.joyx = -1;
                if (this.joyy > 1) this.joyy = 1;
                if (this.joyy < -1) this.joyy = -1;
                if (this.type === "joystick") {
                    this.vars[0] = this.joyx;
                    this.vars[1] = this.joyy;
                } else if (this.type === "hslider") {
                    this.vars[0] = this.joyx;
                }
                else if (this.type === "vslider") {
                    this.vars[0] = this.joyy;
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
            if (touch.identifier === this.activeTouchId) {
                if (this.beingEdited === false && this.type === "button") {
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
            if (touch.identifier === this.activeTouchId) {
                this.onMove(touch.clientX, touch.clientY);
                break;
            }
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
                if (this.myNameElement === document.getElementById("ds-list").children[i]) {
                    document.getElementById("ds-list").children[i].className = "highlighted-ds-list-item";
                    document.getElementById("ds-properties").replaceChildren(this.propertiesElement(false));

                } else {
                    document.getElementById("ds-list").children[i].className = "non-highlighted-ds-list-item";
                }
            }
        } else {
            ctx.fillStyle = this.color;
        }

        ctx.roundRect(this.posX, this.posY, this.width, this.height, [this.radius]);
        ctx.fill();

        if (this.type === "button") {
            if (this.mousePressed) {
                ctx.beginPath();
                ctx.fillStyle = 'black';
                ctx.roundRect(this.posX, this.posY, this.width, this.height, [this.radius]);
                ctx.fill();
            }
        } else {
            ctx.beginPath();
            ctx.fillStyle = 'white';
            ctx.arc(this.posX + this.width / 2 + this.joyx * (this.width / 2 - this.radius), this.posY + this.height / 2 - this.joyy * (this.height / 2 - this.radius), this.radius, 0, 2 * Math.PI);
            ctx.fill();
        }
    }

    nameElement() {
        let element = document.createElement("div");

        let typeLabel = document.createElement("span");
        if (this.type === "vslider") {
            typeLabel.innerHTML = "vertical slider";
        } else if (this.type === "hslider") {
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
        row_data_label.innerHTML = "output variable";
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
            if (this.type === "joystick") {
                inputlable.innerHTML = (i === 0 ? "X" : "Y");
            } else {
                inputlable.innerHTML = "";
            }
            cell.appendChild(inputlable);
            cell.appendChild(input);
            row_data.appendChild(cell);
        }
        element.appendChild(row_data);

        if (this.type === "button") {
            let row_buttonVal = document.createElement("tr");
            let row_buttonVal_label = document.createElement("td");
            row_buttonVal_label.innerHTML = "value when pressed";
            row_buttonVal.appendChild(row_buttonVal_label);
            let cell_buttonVal = document.createElement("td");
            let input_buttonVal = document.createElement("input");
            input_buttonVal.type = "number";
            input_buttonVal.step = "0.05";
            input_buttonVal.value = this.buttonPressedVal;
            input_buttonVal.onchange = (event) => {
                this.buttonPressedVal = parseFloat(event.target.value);
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
            input_buttonRVal.step = "0.05";
            input_buttonRVal.value = this.buttonReleasedVal;
            input_buttonRVal.onchange = (event) => {
                this.buttonReleasedVal = parseFloat(event.target.value);
                input_buttonRVal.value = this.buttonReleasedVal;
            }
            cell_buttonRVal.appendChild(input_buttonRVal);
            row_buttonRVal.appendChild(cell_buttonRVal);
            element.appendChild(row_buttonRVal);

        }


        let row_color = document.createElement("tr");
        let row_color_label = document.createElement("td");
        row_color_label.innerHTML = "Color";
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

        return element;

    }

    run(allData) {
        if (this.beingEdited === false) {
            if (this.type !== "button") {
                if (this.mousePressed) {
                    for (let i = 0; i < this.numData; i++) {
                        if (this.dataIndices[i] !== null) {
                            allData[this.dataIndices[i]] = this.vars[i];
                        }
                    }
                } else {
                    for (let i = 0; i < this.numData; i++) {
                        if (this.dataIndices[i] !== null && allData[this.dataIndices[i]] != undefined) {
                            this.vars[i] = allData[this.dataIndices[i]];
                        }
                    }
                }
            } else { // button special case
                if (this.mousePressed) {
                    if (this.dataIndices[0] !== null && !isNaN(this.buttonPressedVal) && this.buttonPressedVal !== null) {
                        allData[this.dataIndices[0]] = this.buttonPressedVal;
                    }
                } else {
                    if (this.dataIndices[0] !== null && !isNaN(this.buttonReleasedVal) && this.buttonReleasedVal !== null) {
                        allData[this.dataIndices[0]] = this.buttonReleasedVal;
                    }
                }
            }

            if (this.type === "joystick") {
                if (this.vars[0] > 1) this.vars[0] = 1;
                if (this.vars[0] < -1) this.vars[0] = -1;
                if (this.vars[1] > 1) this.vars[1] = 1;
                if (this.vars[1] < -1) this.vars[1] = -1;
                this.joyx = this.vars[0];
                this.joyy = this.vars[1];
            } else if (this.type === "hslider") {
                if (this.vars[0] > 1) this.vars[0] = 1;
                if (this.vars[0] < -1) this.vars[0] = -1;
                this.joyx = this.vars[0];
            }
            else if (this.type === "vslider") {
                if (this.vars[0] > 1) this.vars[0] = 1;
                if (this.vars[0] < -1) this.vars[0] = -1;
                this.joyy = this.vars[0];
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
    driverstationEditable = !driverstationEditable;
    document.getElementById("toggleEditDriverstation").innerHTML = !driverstationEditable ? "Edit" : "Run";
    document.getElementById("ds-edit-ui").hidden = !driverstationEditable;

    for (let i = 0; i < DSItems.length; i++) {
        DSItems[i].beingEdited = driverstationEditable;
        DSItems[i].draw();
    }
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

function addDSItem(type) {
    const item = new DSItem(document.getElementById("ds"), type);
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


txdata = [];
setInterval(() => {
    document.getElementById("console").innerHTML = txdata;
    for (let i = 0; i < DSItems.length; i++) {
        DSItems[i].run(txdata);
    }
}, 20);