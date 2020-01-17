# Import

### Importable items
Returns list of browsers and items available for import. Each browser contains 3 integer values: **bookmarks**, **history** and **passwords**. These values represent number of importable items from selected profile or from default profile if no profile was selected.

    void importableItems(ObjectsCallback callback);


### Get profiles
Returns list of profiles for specified *browser*. Each profile contains two string values: **id** and **name**.

    void getProfiles(string browser, ObjectsCallback callback);

**Parameters**

 - browser - Name of the browser to import from. See [BrowserType](#browsertype).

**Examples**

    nw.Import.getProfiles("Firefox", console.log); //Returns firefox profiles

### Select profile
Selects an active profile for specified *browser*. The selected profile is used in later calls to `importableItems()` and `importItem()` functions.

    bool selectProfile(string browser, optional string profileID);

**Parameters**

 - browser - Name of the browser to import from. See [BrowserType](#browsertype).
 - profileID - A profile ID returned from function `getProfiles()`. Default if omitted.

**Return value**

Returns true if the profile was successfully selected, false if the *browser* or *profile* was not found.

**Examples**

    nw.Import.selectProfile("Firefox", "Profile1"); //Selects "Profile1" as an active firefox profile

### Import item
Returns list of *items* for specified *browser*. Items are imported from profile selected by function `selectProfile()` or from default profile if no profile was selected.

    void importItem(string browser, string item, optional int limit, ObjectsCallback callback);

**Parameters**

 - browser - Name of the browser to import from. See [BrowserType](#browsertype).
 - item - Name of the item to import. See [ItemType](#itemtype).
 - limit - Maximum number of items to be imported. No limit if omitted.

**Examples**

    nw.Import.importItem("Google Chrome", "passwords", console.log); //Returns Chrome passwords


## Enums

### BrowserType
|Name                   |
|-----------------------|
|Chromium               |
|Google Chrome          |
|Google Chrome Canary   |
|Google Chrome Beta     |
|Firefox                |
|Opera Internet Browser |
|Internet Explorer      |

### ItemType
|Name      |
|----------|
|history   |
|bookmarks |
|cookies   |
|passwords |
