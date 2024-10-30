var configurations_info = null; // the configuration file pulled from github
var options = null; // configurations_info, but just the lines with program info
document.addEventListener('DOMContentLoaded', async function () {
    // runs on startup
    // check if web serial is enabled
    if (!("serial" in navigator)) {
        var x = document.getElementsByClassName("serial-alert");
        for (var i = 0; i < x.length; i++) {
            x[i].innerHTML = "Web Serial is not available, so this site won't be able to communicate with your car. Please use Google Chrome, Opera, or Edge, and make sure Web Serial is enabled.";
        }
    }

    updateUpload(); // get the compiled code from github

    // watch the upload-progress span to get information about the program upload progress
    const observer = new MutationObserver(mutationRecords => {
        if (mutationRecords[0].addedNodes[0].data === "Done!") {
            document.getElementById("upload-program").style.backgroundColor = "lightgrey";

            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                x[i].innerHTML = "Upload complete!"
            }
            document.getElementById("upload-button").style.outline = "0px";

        } else if (mutationRecords[0].addedNodes[0].data === "Error!") {
            var x = document.getElementsByClassName("upload-info");
            for (var i = 0; i < x.length; i++) {
                x[i].innerHTML = 'Error Uploading! Check the USB cable, board, and port selections, then press the upload button to the left to try again. If you have an Arduino nano try selecting the other type of nano.';
            }

            document.getElementById("upload-button").style.outline = "3px solid red";
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

async function updateUpload() {
    var checkBox = document.getElementById("upload-main-checkbox");
    configurations_info = await getConfigInfo(checkBox.checked);

    if (configurations_info == null) {
        document.getElementById("no-config-alert").innerHTML = "The list of programs available to upload can not be found. Please check your internet connection, try again in 10 minutes, then please contact us if the problem continues.";

        document.getElementById("source-name-display").innerHTML = "Error: configurations info is null!";

    } else { //there are options for programs to upload

        configurations_info = configurations_info.split("\n");
        document.getElementById("no-config-alert").innerHTML = "";

        options = configurations_info.slice(2, -1); //get just the rows with data for an option
        //split rows and make 2d array
        for (var i = 0; i < options.length; i++) {
            options[i] = options[i].split(", ");
        }

        console.log(options);

        updateProgramOptionsSelector();

    }
}

async function getCode() {
    var upload_warning_span = document.getElementById("upload-warning-span");
    var upload_button = document.getElementById("upload-button");
    upload_button.hidden = true;
    upload_warning_span.innerHTML = "loading...";

    var program_selector = document.getElementById("board-selector");
    var program = program_selector.options[program_selector.selectedIndex].value;
    console.log(options);
    var name = options.filter((v) => { return v[1] === program })[0];
    var code = null;
    try {
        code = {};
        console.log(configurations_info);
        
        code["boot_app"] = await getRequest("https://raw.githubusercontent.com/gobabygocarswithjoysticks/car-code/" + configurations_info[1] + "/hex/" + name + "/boot_app0.bin", true);
        code["ino_bin"] = await getRequest("https://raw.githubusercontent.com/gobabygocarswithjoysticks/car-code/" + configurations_info[1] + "/hex/" + name + "/" + program + ".ino." + "bin", true);
        code["bootloader"] = await getRequest("https://raw.githubusercontent.com/gobabygocarswithjoysticks/car-code/" + configurations_info[1] + "/hex/" + name + "/" + program + ".ino." + "bootloader.bin", true);
        code["partitions"] = await getRequest("https://raw.githubusercontent.com/gobabygocarswithjoysticks/car-code/" + configurations_info[1] + "/hex/" + name + "/" + program + ".ino." + "partitions.bin", true);

        const reader1 = new FileReader();
        reader1.onload = (ev) => {
            code["boot_app"] = ev.target.result;
        }
        reader1.readAsBinaryString(code["boot_app"]); // I know it's deprecated but I can't find anything else that works

        const reader2 = new FileReader();
        reader2.onload = (ev) => {
            code["ino_bin"] = ev.target.result;
        }
        reader2.readAsBinaryString(code["ino_bin"]);

        const reader3 = new FileReader();
        reader3.onload = (ev) => {
            code["bootloader"] = ev.target.result;
        }
        reader3.readAsBinaryString(code["bootloader"]);

        const reader4 = new FileReader();
        reader4.onload = (ev) => {
            code["partitions"] = ev.target.result;
        }
        reader4.readAsBinaryString(code["partitions"]);

        if (code["boot_app"] == null || code["ino_bin"] == null || code["bootloader"] == null || code["partitions"] == null) {
            code = null; // there was a problem getting all 4 components of the code
        }
        var fileEnding = null;

    } catch (e) {
        console.log("error downloading code " + e);
    }

    if (code == null) {
        upload_warning_span.innerHTML = "<mark>The code for the car can not be found. Please check your internet connection, try again in 10 minutes, then please contact us if the problem continues.</mark>";
        upload_button.hidden = true;
    } else { // code received! 
        var upload_progress_span = document.getElementById("upload-progress");
        upload_progress_span.innerHTML = "Ready";
        if (esp32UploadListenerFunction != null) {
            upload_button.removeEventListener("click", esp32UploadListenerFunction);
        }
        esp32UploadListenerFunction = async function () {
            if (upload_button.disabled) {
                return;
            }
            document.getElementById("upload-button").disabled = true;

            upload_button.hidden = false;
            upload_warning_span.innerHTML = "";
            //using this example: https://github.com/espressif/esptool-js/blob/main/examples/typescript/src/index.ts
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

                // ; esptool.py --chip esp32 --port "COM30" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 esp32dev\bootloader.bin 0x8000 esp32dev\partitions.bin 0xe000 boot_app0.bin 0x10000 esp32dev\firmware.bin
                // ; esptool.py --chip esp32 --port "COM30" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 2162688 esp32dev\littlefs.bin
                let fileArray = [];
                console.log(code);

                // await fileArray.push({ data: await code["bootloader"], address: 0x1000 });
                // await fileArray.push({ data: await code["partitions"], address: 0x8000 });
                // await fileArray.push({ data: await code["boot_app"], address: 0xe000 });
                // await fileArray.push({ data: await code["ino_bin"], address: 0x10000 });

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
                        espLoaderTerminal.writeLine("PROGRESS:" + fileIndex + "," + written + "," + total);
                        document.getElementById("upload-progress").innerHTML = Math.floor(1 + (fileIndex * 10) + ((fileIndex < 3) ? 9 * (written / total) : 68 * (written / total))) + "%";
                    }
                    , calculateMD5Hash: (image) => CryptoJS.MD5(CryptoJS.enc.Latin1.parse(image))
                };

                await esploader.writeFlash(flashOptions);

                await esploader.hardReset();

            } catch (e) {
                console.log(e);
                document.getElementById("upload-progress").innerHTML = "Error!"
            } finally {
                //
                try {
                    await device.close();
                } catch (e) {
                    console.log("caught error closing device (it probably never opened");
                    console.log(e);
                }
                document.getElementById("upload-button").disabled = false;
            }
        }
        upload_button.addEventListener("click", esp32UploadListenerFunction);

        upload_warning_span.innerHTML = "";
        var upload_progress_span = document.getElementById("upload-progress");
        upload_progress_span.innerHTML = "Ready";
        upload_button.hidden = false;
    }

}
function updateProgramOptionsSelector() {
    //uses https://stackoverflow.com/a/42123984 to find unique values

    var select = document.getElementById("board-selector");

    if (options == null) {
        select.replaceChildren();
    } else {
        select.replaceChildren();
        // https://www.geeksforgeeks.org/how-to-create-a-dropdown-list-with-array-values-using-javascript/
        for (var i = 0; i < options.length; i++) {
            var optn = options[i];
            var el = document.createElement("option");
            el.textContent = optn;
            el.value = optn;
            select.appendChild(el);
        }
    }
    getCode();
}

function extractColumn(arr, column) {
    //https://gist.github.com/eddieajau/5f3e289967de60cf7bf9?permalink_comment_id=2727196#gistcomment-2727196
    return arr.map(x => x[column])
}

/**
 * Gets configurations-info.txt from github, and returns it as a string.
 * returns null if there is an error
 * if fromMain is true, it pulls the data from the main branch, if false it pulls the data from the most recent release
 */
async function getConfigInfo(fromMain) {
    if (fromMain === true) {
        try {
            result = await getRequest("https://raw.githubusercontent.com/RCMgames/RCMv3/refs/heads/main/docs/programmer/firmware/configurations_info.txt");
            return "unreleased\n" + "main" + "\n" + result;
        } catch (e) {
            return null;
        }

    } else {
        try {
            var json = JSON.parse(await getRequest("https://api.github.com/repos/rcmgames/rcmv3/releases"));
            var most_recent_release_tag = json[0].tag_name;

            var configUrl = "https://raw.githubusercontent.com/RCMgames/RCMv3/refs/tags/" + most_recent_release_tag + "/docs/programmer/firmware/configurations_info.txt";
            return "release\n" + most_recent_release_tag + "\n" + await getRequest(configUrl);
        } catch (e) {
            return null;
        }
    }
};
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
