# URLRow structure

Used as the target for importing history URLs from other browser's profiles. Represents one item in the history.

## Member variables

### url
The URL associated with the history item. See [GURL](Chrome_PasswordForm_structure.md#gurl).

### title
The title of the page associated with the history item.

### visit_count
Total number of times this URL has been visited.

### typed_count
Number of times this URL has been manually entered in the URL bar.

### last_visit
The date of the last visit of this URL, which saves us from having to loop up in the visit table for things like autocomplete and expiration. See [Time](Chrome_PasswordForm_structure.md#time).

### hidden
Indicates this entry should not be shown in typical UI or queries, this is usually for subframes.
