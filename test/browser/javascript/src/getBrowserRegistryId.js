function getBrowserRegistryIdButtonFunction() {
    console.log("getBrowserRegistryIdButton");
    try {
        var registryIdTimeout = setTimeout(function () {
            addFinalResult("getBrowserRegistryId callback not received within 8 seconds", false, 'defaultBrowserResult');
        }, 8000);

        var nwjsid = nw.App.getBrowserRegistryId();
        addTestResult("Browser Registry ID: " + nwjsid)

        clearTimeout(registryIdTimeout);

        if (/^nwjs\.\S+$/.test(nwjsid)) {
            addFinalResult("Browser Registry ID match the regexp", true, 'getBrowserRegistryIdResult');
        } else {
            addFinalResult("Browser Registry ID did not match the regexp", false, 'getBrowserRegistryIdResult');
        }
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'getBrowserRegistryIdResult');
    }
}