function sqliteNamespaceButtonFunction() {
    console.log("sqliteNamespaceButton");
    
    try {
        var sqliteTimeoutInit = setTimeout(function () {
            addFinalResult("sqliteNamespace callback not received within 8 seconds", false, 'sqliteNamespaceResult');
        }, 8000);
        nw.SQLite.init("", (rc, data) => {
            
            clearTimeout(sqliteTimeoutInit);
            
            addTestResult("Result code: " + rc);
            if (rc == 0) {
                addTestResult("Database filename: \"" + data + "\"");
                addFinalResult("SQLite.init", true, 'sqliteNamespaceResult');
            } else {
                addTestResult("Error message: " + data);
                addFinalResult("SQLite.init", false, 'sqliteNamespaceResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'sqliteNamespaceResult');
    }
    
    try {
        var sqliteTimeoutDeinitOk = setTimeout(function () {
            addFinalResult("sqliteNamespace callback not received within 8 seconds", false, 'sqliteNamespaceResult');
        }, 8000);
        nw.SQLite.deinit("", (rc, data) => {
            
            clearTimeout(sqliteTimeoutDeinitOk);
            
            addTestResult("Result code: " + rc);
            if (rc == 0) {
                addTestResult("Result data: " + data);
                addFinalResult("SQLite.deinit positive test", true, 'sqliteNamespaceResult');
            } else {
                addTestResult("Error message: " + data);
                addFinalResult("SQLite.deinit positive test", false, 'sqliteNamespaceResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'sqliteNamespaceResult');
    }
    
    try {
        var sqliteTimeoutDeinitNok = setTimeout(function () {
            addFinalResult("sqliteNamespace callback not received within 8 seconds", false, 'sqliteNamespaceResult');
        }, 8000);
        nw.SQLite.deinit("", (rc, data) => {
            
            clearTimeout(sqliteTimeoutDeinitNok);
            
            addTestResult("Result code: " + rc);
            if (rc == 1) {
                addTestResult("Error message: " + data);
                addFinalResult("SQLite.deinit negative test", true, 'sqliteNamespaceResult');
            } else {
                addTestResult("Result data: " + data);
                addFinalResult("SQLite.deinit negative test", false, 'sqliteNamespaceResult');
            }
        } );
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'sqliteNamespaceResult');
    }
}