# Import

### Importable items
Returns list of browsers and items available for import. Each browser contains these 4 integer values: **bookmarks**, **cookies**, **history** and **passwords**. These values represent number of importable items from selected profile or from default profile if no profile was selected.

    void importableItems(ObjectsCallback callback);


### Get profiles
Returns list of profiles for specified *browser*. Each profile contains two string values: **id** and **name**.

    void getProfiles(int browser, ObjectsCallback callback);

**Parameters**

 - browser - An enum value representing the browser to import from. See [BrowserType](#browsertype).

**Examples**

    nw.Import.getProfiles(2, console.log); //Returns firefox profiles

### Select profile
Selects an active profile for specified *browser*. The selected profile is used in later calls to `importableItems()` and `importItem()` functions.

    bool selectProfile(int browser, string profileID);

**Parameters**

 - browser - An enum value representing the browser to import from. See [BrowserType](#browsertype).
 - profileID - A profile ID returned from function `getProfiles()`.

**Return value**

Returns true if the profile was successfully selected, false if the *browser* or *profile* was not found.

**Examples**

    nw.Import.selectProfile(2, "Profile1"); //Selects "Profile1" as an active firefox profile

### Import item
Returns list of *items* for specified *browser*. Items are imported from profile selected by function `selectProfile()` or from default profile if no profile was selected.

    void importItem(int browser, int item, ObjectsCallback callback);

**Parameters**

 - browser - An enum value representing the browser to import from. See [BrowserType](#browsertype).
 - item - An enum value representing the item to import. See [ItemType](#itemtype).

**Examples**

    nw.Import.importItem(2, 8, console.log); //Returns firefox passwords


## Enums

### BrowserType
|Name               |Value |
|-------------------|------|
|TYPE_CHROME        |1     |
|TYPE_FIREFOX       |2     |
|TYPE_CHROME_CANARY |7     |
|TYPE_CHROMIUM      |8     |
|TYPE_CHROME_BETA   |9     |
|TYPE_OPERA         |10    |

### ItemType
|Name      |Value |
|----------|------|
|HISTORY   |1     |
|FAVORITES |2     |
|COOKIES   |4     |
|PASSWORDS |8     |
