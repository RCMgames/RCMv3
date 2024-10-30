



document.addEventListener('DOMContentLoaded', async function () {

    getConfigInfo();
});

async function updateUpload() {

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
    console.log(retVal);
    return retVal;
};
