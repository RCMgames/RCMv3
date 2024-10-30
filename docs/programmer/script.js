var configInfo = null;
var code = null;

document.addEventListener('DOMContentLoaded', async function () {

    if (!("serial" in navigator)) {
        var x = document.getElementsByClassName("serial-alert");
        for (var i = 0; i < x.length; i++) {
            x[i].innerHTML = "Web Serial is not available, so this site won't be able to communicate with your car. Please use Google Chrome, Opera, or Edge, and make sure Web Serial is enabled.<br>";
        }
    } else {

        await getConfigInfo();
    }
    // watch the upload-progress span to get information about the program upload progress
    const observer = new MutationObserver(mutationRecords => {
        if (mutationRecords[0].addedNodes[0].data === "Done!") {
            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                x[i].innerHTML = 'Upload complete! Press the reset button on the ESP32 to run the program.';
            }
        } else if (mutationRecords[0].addedNodes[0].data === "Error!") {
            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                x[i].innerHTML = 'Error Uploading! Check the USB cable, board, and port selections, then press the upload button to the left to try again';
            }

        } else if (mutationRecords[0].addedNodes[0].data === "0%") {

            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                x[i].innerHTML = "Upload starting..."
            }
        } else if (mutationRecords[0].addedNodes[0].data === "Ready") {
        } else {
            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                // draw progress bar
                x[i].innerHTML = 'Uploading ' + mutationRecords[0].addedNodes[0].data + '\xa0complete. <br>'
                    + '</div> <div style="width:100%; background-color:#ddd;"> <div style="width:'
                    + mutationRecords[0].addedNodes[0].data
                    + '; background-color: #04AA6D; height: 30px;"></div></div>';
            }
        }

    });
    observer.observe(document.getElementById("upload-progress"), {
        childList: true
    });

});

async function getCode() {
    code = {};
    var program_selector = document.getElementById("board-selector");
    var board = program_selector.options[program_selector.selectedIndex].value;
    var codeDirectoryURL = "";
    if (configInfo[0] == "release") {
        codeDirectoryURL = "https://raw.githubusercontent.com/RCMgames/RCMv3/refs/tags/" + configInfo[1] + "/docs/programmer/firmware/" + board;
    } else {
        codeDirectoryURL = "https://raw.githubusercontent.com/RCMgames/RCMv3/refs/heads/main/docs/programmer/firmware/" + board;
    }

    try {
        document.getElementById("upload-button").disabled = true;
        document.getElementById("upload-button").innerHTML = "Loading...";
        code["boot_app0"] = await getRequest(codeDirectoryURL + "boot_app0.bin", true);
        code["bootloader"] = await getRequest(codeDirectoryURL + "bootloader.bin", true);
        code["partitions"] = await getRequest(codeDirectoryURL + "partitions.bin", true);
        code["littlefs"] = await getRequest(codeDirectoryURL + "littlefs.bin", true);
        code["firmware"] = await getRequest(codeDirectoryURL + "firmware.bin", true);
        document.getElementById("upload-button").disabled = false;
        document.getElementById("upload-button").innerHTML = "Upload";
    } catch (e) {
        document.getElementById("upload-button").innerHTML = "Error: Couldn't get code";
        document.getElementById("upload-button").disabled = true
    }
}

async function upload() {
    if (document.getElementById("upload-button").disabled == true) {
        console.log("upload button is disabled");
        return;
    }
    console.log(code);
    if (code == null || code["boot_app0"] == null || code["bootloader"] == null || code["partitions"] == null || code["littlefs"] == null || code["firmware"] == null) {
        console.log("code is not loaded");
        return;
    }
    document.getElementById("upload-button").disabled = true;

    try {
        var device = await navigator.serial.requestPort({});
        var transport = new Transport(device, true);

        let espLoaderTerminal = {
            clean() {
            },
            writeLine(data) {
                console.log(data);
                if (data === "Leaving...") {
                    document.getElementById("upload-progress").innerHTML = "Done!"
                }
            },
            write(data) {
            },
        };

        let fileArray = [];

        console.log(code);

        const readFileAsBinaryString = (file) => {
            return new Promise((resolve, reject) => {
                const reader = new FileReader();
                reader.onload = (ev) => resolve(ev.target.result);
                reader.onerror = (err) => reject(err);
                reader.readAsBinaryString(file);
            });
        };

        try {
            const results = await Promise.all([
                readFileAsBinaryString(code["boot_app0"]),
                readFileAsBinaryString(code["bootloader"]),
                readFileAsBinaryString(code["partitions"]),
                readFileAsBinaryString(code["littlefs"]),
                readFileAsBinaryString(code["firmware"])
            ]);

            code["boot_app0"] = results[0];
            code["bootloader"] = results[1];
            code["partitions"] = results[2];
            code["littlefs"] = results[3];
            code["firmware"] = results[4];

            console.log(code);

            await fileArray.push({ data: code["boot_app0"], address: 0xe000 });
            await fileArray.push({ data: code["bootloader"], address: 0x1000 });
            await fileArray.push({ data: code["partitions"], address: 0x8000 });
            await fileArray.push({ data: code["littlefs"], address: 0x210000 });
            await fileArray.push({ data: code["firmware"], address: 0x10000 });

            console.log(fileArray);

            const flashOptionsMain = {
                transport,
                baudrate: 921600,
                enableTracing: false,
                debugLogging: false,
                terminal: espLoaderTerminal
            }

            let esploader = new ESPLoader(flashOptionsMain);

            alert("Hold the IO0 button on the ESP32 until the green progress bar appears. Press OK on this message when you have started to hold the button.")

            document.getElementById("upload-progress").innerHTML = "0%"

            setTimeout(() => {
                if (document.getElementById("upload-progress").innerHTML === "0%") {
                    alert("It's taking too long to connect to the ESP32. This can happen if you weren't holding the IO0 button. Try refreshing the website and trying again.");
                }
            }, 5000);

            await esploader.main();

            const flashOptions = {
                fileArray: fileArray,
                flashSize: "keep",
                eraseAll: false,
                compress: true,
                baudrate: 921600,
                reportProgress: (fileIndex, written, total) => {
                    console.log("PROGRESS:" + fileIndex + "," + written + "," + total);
                    document.getElementById("upload-progress").innerHTML = Math.floor(1 + (fileIndex * 10) + ((fileIndex < 4) ? 9 * (written / total) : 58 * (written / total))) + "%";
                }
                , calculateMD5Hash: (image) => CryptoJS.MD5(CryptoJS.enc.Latin1.parse(image))
            };

            await esploader.writeFlash(flashOptions);

            await esploader.hardReset();

        } catch (error) {
            console.error("Error reading files:", error);
        }

    } catch (e) {
        console.log(e);
        document.getElementById("upload-progress").innerHTML = "Error!";
    }
    finally {
        try {
            await device.close();
        } catch (e) {
            console.log("caught error closing device (it probably never opened");
            console.log(e);
        }
        document.getElementById("upload-button").disabled = false;
    }
}

/**
 * code for getting data from a url, returns response.text if the request succeeds, and throws an error if there is an error status.
 */
async function getRequest(url, blob = false) {
    await fetch(url)
        .then((response) => {
            if (response.status == 200) {
                if (blob) {
                    return response.blob();
                } else {
                    return response.text();
                }
            } else {
                throw new Error(`HTTP error in getRequest() Status: ${response.status} `);
            }
        })
        .then((responseText) => {
            result = responseText;
        });
    return result;
}
/**
 * Gets configurations-info.txt from github, and returns it as a string.
 * returns null if there is an error
 * if fromMain is true, it pulls the data from the main branch, if false it pulls the data from the most recent release
 */
async function getConfigInfo() {
    fromMain = document.getElementById("pull-from-main").checked;
    let retVal;
    if (fromMain === true) {
        try {
            result = await getRequest("https://raw.githubusercontent.com/RCMgames/RCMv3/refs/heads/main/docs/programmer/firmware/configurations_info.txt");
            retVal = "unreleased\n" + "main" + "\n" + result;
        } catch (e) {
            retVal = null;
        }
    } else {
        try {
            var json = JSON.parse(await getRequest("https://api.github.com/repos/rcmgames/rcmv3/releases"));
            var most_recent_release_tag = json[0].tag_name;

            var configUrl = "https://raw.githubusercontent.com/RCMgames/RCMv3/refs/tags/" + most_recent_release_tag + "/docs/programmer/firmware/configurations_info.txt";
            retVal = "release\n" + most_recent_release_tag + "\n" + await getRequest(configUrl);
        } catch (e) {
            retVal = null;
        }
    }
    let options;
    if (retVal == null) {
        options = null;
    } else {
        configInfo = retVal.split("\n");

        document.getElementById("release").innerHTML = configInfo[1];

        options = configInfo.slice(2, -1); //get just the rows with data for an option
        //split rows and make 2d array
        for (var i = 0; i < options.length; i++) {
            options[i] = options[i].split(", ");
        }
    }

    if (options == null) {
        boardSelector = document.getElementById("board-selector");
        boardSelector.replaceChildren();
        configInfo = null;
        //TODO: ERROR, couldn't get config info
    } else {
        boardSelector = document.getElementById("board-selector");
        boardSelector.replaceChildren();
        // https://www.geeksforgeeks.org/how-to-create-a-dropdown-list-with-array-values-using-javascript/
        for (var i = 0; i < options.length; i++) {
            var optn = options[i][0];
            var el = document.createElement("option");
            el.textContent = optn.slice(0, -1);
            el.value = optn;
            boardSelector.appendChild(el);
        }
    }
    await getCode();
};
