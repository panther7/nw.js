# BookmarkEntry structure

Used as the target for importing bookmarks from other browser's profiles. Represents one bookmark.

## Member variables

### in_toolbar
True if the bookmark is visible in the bookmark panel.

### is_folder
True if the bookmark is an empty folder.

### url
The URL associated with the bookmark. See [GURL](Chrome_PasswordForm_structure.md#gurl).

### path
An array of strings where each string represents one folder in the bookmark path.

### title
The title of the page associated with the bookmark.

### creation_time
When the bookmark was saved. See [Time](Chrome_PasswordForm_structure.md#time).
