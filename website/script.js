class DSItem {
    constructor(_dsCanvas, _dsItems, _type) {
        this.type = _type;
        this.dsItems = _dsItems;
        this.dsCanvas = _dsCanvas;

        this.width = 300;
        this.height = 300;

        this.x = 0;
        this.y = 0;
        this.isDragging = false;

        const ctx = this.dsCanvas.getContext('2d');
        ctx.fillStyle = 'green';
        ctx.fillRect(this.x, this.y, this.width, this.height);

        this.dsCanvas.addEventListener('mousedown', this.onMouseDown.bind(this));
        this.dsCanvas.addEventListener('mouseup', this.onMouseUp.bind(this));
        this.dsCanvas.addEventListener('mousemove', this.onMouseMove.bind(this));

        // Add touch event listeners
        this.dsCanvas.addEventListener('touchstart', this.onTouchStart.bind(this));
        this.dsCanvas.addEventListener('touchend', this.onTouchEnd.bind(this));
        this.dsCanvas.addEventListener('touchmove', this.onTouchMove.bind(this));
    }

    onMouseDown(event) {
        const rect = this.dsCanvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        if (mouseX >= this.x && mouseX <= this.x + this.width && mouseY >= this.y && mouseY <= this.y + this.height) {
            this.offsetX = mouseX - this.x;
            this.offsetY = mouseY - this.y;
            this.isDragging = true;
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
                this.isDragging = true;
                this.activeTouchId = touch.identifier;
            }
        }
    }
    onMouseUp() {
        this.isDragging = false;
    }
    onMouseMove(event) {
        this.onMove(event.clientX, event.clientY);
    }
    onMove(x, y) {
        if (this.isDragging) {
            const rect = this.dsCanvas.getBoundingClientRect();
            const ctx = this.dsCanvas.getContext('2d');
            ctx.clearRect(0, 0, this.dsCanvas.width, this.dsCanvas.height);
            this.x = x - rect.left - this.offsetX;
            this.y = y - rect.top - this.offsetY;
            if (this.x < 0) this.x = 0;
            if (this.y < 0) this.y = 0;
            if (this.x + this.width > this.dsCanvas.width) {
                this.x = this.dsCanvas.width - this.width
            };
            if (this.y + this.height > this.dsCanvas.height) {
                this.y = this.dsCanvas.height - this.height
            };
            for (let i = 0; i < this.dsItems.length; i++) {
                this.dsItems[i].draw();
            }
            this.draw();
        }
    }
    onTouchEnd(event) {
        for (let i = 0; i < event.changedTouches.length; i++) {
            const touch = event.changedTouches[i];
            if (touch.identifier === this.activeTouchId) {
                this.isDragging = false;
                this.activeTouchId = null;
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
        ctx.fillStyle = 'green';
        ctx.fillRect(this.x, this.y, this.width, this.height);
    }
}

async function setLed(status) {
    fetch('./' + (status ? 'on' : 'off'));
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