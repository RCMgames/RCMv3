class DSItem {
    constructor(_dsCanvas, _dsItems, _type) {
        this.type = _type;
        this.dsItems = _dsItems;
        this.dsCanvas = _dsCanvas;
        this.size = 250;

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
            this.width = this.size;
            this.height = this.size;
            this.radius = 0;
        }

        this.x = 0;
        this.y = 0;

        this.mousePressed = false;

        this.beingEdited = true;

        this.joyx = 0;
        this.joyy = 0;

        this.vars = [];
        if (this.type === "joystick") {
            this.vars = [0.0, 0.0];
        } else if (this.type === "hslider") {
            this.vars = [0.0];
        } else if (this.type === "vslider") {
            this.vars = [0.0];
        } else if (this.type === "button") {
            this.vars = [0];
        }

        this.dsCanvas.addEventListener('mousedown', this.onMouseDown.bind(this));
        this.dsCanvas.addEventListener('mouseup', this.onMouseUp.bind(this));
        this.dsCanvas.addEventListener('mousemove', this.onMouseMove.bind(this));
        this.dsCanvas.addEventListener('touchstart', this.onTouchStart.bind(this));
        this.dsCanvas.addEventListener('touchend', this.onTouchEnd.bind(this));
        this.dsCanvas.addEventListener('touchmove', this.onTouchMove.bind(this));

        this.draw();

    }

    onMouseDown(event) {
        const rect = this.dsCanvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        if (mouseX >= this.x && mouseX <= this.x + this.width && mouseY >= this.y && mouseY <= this.y + this.height) {
            this.offsetX = mouseX - this.x;
            this.offsetY = mouseY - this.y;
            this.mousePressed = true;
            if (this.beingEdited === false && this.type === "button") {
                this.vars[0] = 1;
            }
            this.draw();
        }
    }
    onTouchStart(event) {
        const rect = this.dsCanvas.getBoundingClientRect();
        for (let i = 0; i < event.touches.length; i++) {
            const touch = event.touches[i];
            const mouseX = touch.clientX - rect.left;
            const mouseY = touch.clientY - rect.top;
            if (mouseX >= this.x && mouseX <= this.x + this.width && mouseY >= this.y && mouseY <= this.y + this.height) {
                this.offsetX = mouseX - this.x;
                this.offsetY = mouseY - this.y;
                this.mousePressed = true;
                if (this.beingEdited === false && this.type === "button") {
                    this.vars[0] = 1;
                }
                this.activeTouchId = touch.identifier;
                this.draw();
            }
        }
    }
    onMouseUp() {
        this.mousePressed = false;
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
                this.x = x - rect.left - this.offsetX;
                this.y = y - rect.top - this.offsetY;
                this.x = Math.floor(this.x / 10) * 10;
                this.y = Math.floor(this.y / 10) * 10;
                if (this.x < 0) this.x = 0;
                if (this.y < 0) this.y = 0;
                if (this.x + this.width > this.dsCanvas.width) {
                    this.x = this.dsCanvas.width - this.width
                };
                if (this.y + this.height > this.dsCanvas.height) {
                    this.y = this.dsCanvas.height - this.height
                };
                this.redrawCanvas();
                this.draw();
            } else {
                const rect = this.dsCanvas.getBoundingClientRect();
                if (this.type === "joystick" || this.type === "hslider") {
                    this.joyx = (x - rect.left - this.x - this.width / 2) / (this.width / 2 - this.radius);
                } else {
                    this.joyx = 0;
                }
                if (this.type === "joystick" || this.type === "vslider") {
                    this.joyy = -(y - rect.top - this.y - this.height / 2) / (this.height / 2 - this.radius);
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
                } else if (this.type === "hslider" || this.type === "vslider") {
                    this.vars[0] = this.joyx;
                }
                this.draw();
            }
        }
    }
    redrawCanvas() {
        const ctx = this.dsCanvas.getContext('2d');
        ctx.clearRect(0, 0, this.dsCanvas.width, this.dsCanvas.height);
        for (let i = 0; i < this.dsItems.length; i++) {
            this.dsItems[i].draw();
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
        ctx.fillStyle = 'green';
        ctx.roundRect(this.x, this.y, this.width, this.height, [this.radius]);
        ctx.fill();

        if (this.type === "button") {
            if (this.vars[0] === 1) {
                ctx.beginPath();
                ctx.fillStyle = 'white';
                ctx.roundRect(this.x, this.y, this.width, this.height, [this.radius]);
                ctx.fill();
            }
        } else {
            ctx.beginPath();
            ctx.fillStyle = 'white';
            ctx.arc(this.x + this.width / 2 + this.joyx * (this.width / 2 - this.radius), this.y + this.height / 2 - this.joyy * (this.height / 2 - this.radius), this.radius, 0, 2 * Math.PI);
            ctx.fill();
        }
    }
}

async function setLed(status) {
    for (let i = 0; i < DSItems.length; i++) {
        DSItems[i].beingEdited = status;
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

let DSItems = [];

function addDSItem(type) {
    DSItems.push(new DSItem(document.getElementById("ds"), DSItems, type));
}
function clearDSItems() {
    DSItems = [];
    const ctx = document.getElementById("ds").getContext('2d');
    ctx.clearRect(0, 0, document.getElementById("ds").width, document.getElementById("ds").height);
}