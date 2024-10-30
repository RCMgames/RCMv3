var configInfo = null;
var code = null;

document.addEventListener('DOMContentLoaded', async function () {
    await getConfigInfo();
    await getCode();
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

    code["boot_app0"] = await getRequest(codeDirectoryURL + "boot_app0.bin", true);
    code["bootloader"] = await getRequest(codeDirectoryURL + "bootloader.bin", true);
    code["partitions"] = await getRequest(codeDirectoryURL + "partitions.bin", true);
    code["littlefs"] = await getRequest(codeDirectoryURL + "littlefs.bin", true);
    code["firmware"] = await getRequest(codeDirectoryURL + "firmware.bin", true);

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
};
