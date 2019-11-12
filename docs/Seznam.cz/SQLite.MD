## SQLite

### SQLite version

    string getVersion();
    string version;

**Return value**

Returns SQLite version in the string format "X.Y.Z" where X is the major version number, Y is the minor version number and Z is the release number. For example: "3.30.1". Equivalent to variable `nw.SQLite.version`.


### Init SQLite
Opens an SQLite database file identified by parameter *db_file_path*.

    void init(string db_file_path, optional int flags, optional ObjectsCallback callback);

**Parameters**

 - db_file_path - Database filename. Special values are ":memory:" for an anonymous in-memory database and an empty string for an anonymous disk-based database. Anonymous databases are not persisted and when closing the database handle, their content is lost.
 - flags - Optional flags used to determine how to open the database. By default, init() uses SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE. List of all available flags: [https://www.sqlite.org/c3ref/c_open_autoproxy.html](https://www.sqlite.org/c3ref/c_open_autoproxy.html)
 - callback - Function with signature: void func(object[] data); The first item in the data parameter is a result code. The second item in the data parameter is database filename or error message. List of all result codes: [https://www.sqlite.org/c3ref/c_abort.html](https://www.sqlite.org/c3ref/c_abort.html)

**Examples**

    nw.SQLite.init("C:\\playground\\sbrowser.db");
    nw.SQLite.init("C:\\playground\\sbrowser.db", SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE);
    nw.SQLite.init("C:\\playground\\sbrowser.db", SQLITE_OPEN_READONLY, console.log);
    nw.SQLite.init("C:\\playground\\sbrowser.db", console.log);


### Deinit SQLite
Closes the database identified by parameter *db_id* and deallocates all associated resources.

    void deinit(string db_id, optional ObjectsCallback callback);

**Parameters**

 - db_id - Database identifier returned from `nw.SQLite.init()`
 - callback - Function with signature: void func(object[] data); The first item in the data parameter is a result code SQLITE_OK or SQLITE_ERROR. The second item in the data parameter is a status message.

**Examples**

    nw.SQLite.deinit("C:\\playground\\sbrowser.db");
    nw.SQLite.deinit("C:\\playground\\sbrowser.db", console.log);


### Execute SQLite
Runs a sequence of SQL statements, separated by semicolons, in the context of the database connection.

    void execute(string db_id, string sql_statement, optional ObjectsCallback callback);

**Parameters**

 - db_id - Database identifier returned from `nw.SQLite.init()`
 - sql_statement - SQL statement to be evaluated
 - callback - Function with signature: void func(object[] data); The first item in the data parameter is a result code. The second item in the data parameter is a list of results.

**Examples**

    nw.SQLite.execute("C:\\playground\\sbrowser.db", "CREATE TABLE IF NOT EXISTS Tasks (task_id INT PRIMARY KEY, description TEXT);");
    nw.SQLite.execute("C:\\playground\\sbrowser.db", "INSERT INTO Tasks (task_id, description) VALUES (1, 'Michal');");
    nw.SQLite.execute("C:\\playground\\sbrowser.db", "INSERT INTO Tasks (task_id, description) VALUES (2, 'Martin');");
    nw.SQLite.execute("C:\\playground\\sbrowser.db", "SELECT * FROM Tasks;", console.log);


###  Set busy timeout
Sets a busy handler that sleeps for a specified amount of time when a table is locked. The handler will sleep multiple times until at least *timeout_ms* milliseconds of sleeping have accumulated. After at least *timeout_ms* milliseconds of sleeping, the handler returns 0 which causes nw.SQLite.execute() to return SQLITE_BUSY.

    void setBusyTimeout(string db_id, optional int timeout_ms, optional ObjectsCallback callback);

**Parameters**

 - db_id - Database identifier returned from `nw.SQLite.init()`
 - timeout_ms - Maximum time in milliseconds of sleeping. Default value is 1000.
 - callback  - Function with signature: void func(object[] data); The first item in the data parameter is a result code. The second item in the data parameter is a status message.

**Examples**

    nw.SQLite.setBusyTimeout("C:\\playground\\sbrowser.db", 1500, console.log);
    nw.SQLite.setBusyTimeout("C:\\playground\\sbrowser.db");

### Last ID

    int lastID(string db_id);

**Parameters**

 - db_id - Database identifier returned from `nw.SQLite.init()`

**Return value**
Returns the rowid of the most recent successful INSERT. Inserts into WITHOUT ROWID tables are not recorded.

**Examples**

    nw.SQLite.lastID("C:\\playground\\sbrowser.db");
