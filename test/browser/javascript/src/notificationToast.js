function notificationToastButtonFunction() {
    console.log("notificationToastButton");

    var host_wa = "web.whatsapp.com";
    var host_fb = "www.facebook.com";
    var host_ms = "www.messenger.com";
    
    var status_allow = "allow";
    var status_deny = "deny";
    var status_default = "default";
    
    try {
        var notificationTimeoutSetAllow = setTimeout(function () {
            addFinalResult("notificationToast callback not received within 8 seconds", false, 'notificationToastResult');
        }, 8000);

        nw.App.setNotificationToastFlag(host_wa, status_allow, (data) => {
            
            clearTimeout(notificationTimeoutSetAllow);
            
            if (data == status_allow) {
                addTestResult("Host \"" + host_wa + "\"" + " with status \"" + status_allow + "\"");
                addFinalResult("setNotificationToastFlag", true, 'notificationToastResult');
            } else {
                addTestResult("Host \"" + host_wa + "\"" + " with status \"" + status_allow + "\"");
                addFinalResult("setNotificationToastFlag", false, 'notificationToastResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'notificationToastResult');
    }
    
    try {
        var notificationTimeoutSetDeny = setTimeout(function () {
            addFinalResult("notificationToast callback not received within 8 seconds", false, 'notificationToastResult');
        }, 8000);
            
        nw.App.setNotificationToastFlag(host_fb, status_deny, (data) => {
            
            clearTimeout(notificationTimeoutSetDeny);
            
            if (data == status_deny) {
                addTestResult("Host \"" + host_fb + "\"" + " with status \"" + status_deny + "\"");
                addFinalResult("setNotificationToastFlag", true, 'notificationToastResult');
            } else {
                addTestResult("Host \"" + host_fb + "\"" + " with status \"" + status_deny + "\"");
                addFinalResult("setNotificationToastFlag", false, 'notificationToastResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'notificationToastResult');
    }
    
    try {
        var notificationTimeoutSetDefault = setTimeout(function () {
            addFinalResult("notificationToast callback not received within 8 seconds", false, 'notificationToastResult');
        }, 8000);
            
        nw.App.setNotificationToastFlag(host_wa, status_default, (data) => {
            
            clearTimeout(notificationTimeoutSetDefault);
            
            if (data == status_default) {
                addTestResult("Host \"" + host_wa + "\"" + " with status \"" + status_default + "\"");
                addFinalResult("setNotificationToastFlag", true, 'notificationToastResult');
            } else {
                addTestResult("Host \"" + host_wa + "\"" + " with status \"" + status_default + "\"");
                addFinalResult("setNotificationToastFlag", false, 'notificationToastResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'notificationToastResult');
    }
    
    try {
        var notificationTimeoutGetDeny = setTimeout(function () {
            addFinalResult("notificationToast callback not received within 8 seconds", false, 'notificationToastResult');
        }, 8000);
            
        nw.App.getNotificationToastFlag(host_fb, (data) => {
            
            clearTimeout(notificationTimeoutGetDeny);
            
            if (data == status_deny) {
                addTestResult("Host \"" + host_fb + "\"" + " with status \"" + status_deny + "\"");
                addFinalResult("getNotificationToastFlag", true, 'notificationToastResult');
            } else {
                addTestResult("Host \"" + host_fb + "\"" + " with status \"" + status_deny + "\"");
                addFinalResult("getNotificationToastFlag", false, 'notificationToastResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'notificationToastResult');
    }
    
    try {
        var notificationTimeoutGetDefault = setTimeout(function () {
            addFinalResult("notificationToast callback not received within 8 seconds", false, 'notificationToastResult');
        }, 8000);
            
        nw.App.getNotificationToastFlag(host_ms, (data) => {
            
            clearTimeout(notificationTimeoutGetDefault);
            
            if (data == status_default) {
                addTestResult("Host \"" + host_ms + "\"" + " with status \"" + status_default + "\"");
                addFinalResult("getNotificationToastFlag", true, 'notificationToastResult');
            } else {
                addTestResult("Host \"" + host_ms + "\"" + " with status \"" + status_default + "\"");
                addFinalResult("getNotificationToastFlag", false, 'notificationToastResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'notificationToastResult');
    }
}