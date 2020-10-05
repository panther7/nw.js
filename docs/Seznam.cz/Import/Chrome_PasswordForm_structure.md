
# Chrome PasswordForm structure

The PasswordForm struct encapsulates information about a login form, which can be an HTML form or a dialog with username/password text fields.

The Web Data database stores saved username/passwords and associated form metdata using a PasswordForm struct, typically one that was created from a parsed HTMLFormElement or LoginDialog, but the saved entries could have also been created by imported data from another browser.

The PasswordManager implements a fuzzy-matching algorithm to compare saved PasswordForm entries against PasswordForms that were created from a parsed HTML or dialog form. As one might expect, the more data contained in one of the saved PasswordForms, the better the job the PasswordManager can do
in matching it against the actual form it was saved on, and autofill accurately. But it is not always possible, especially when importing from other browsers with different data models, to copy over all the information
about a particular "saved password entry" to our PasswordForm representation.

The field descriptions in the struct specification below are intended to describe which fields are not strictly required when adding a saved password entry to the database and how they can affect the matching process.

## Member variables

### signon_realm
The "Realm" for the sign-on. This is scheme, host, port for SCHEME_HTML. Dialog based forms also contain the HTTP realm. Android based forms will contain a string of the form "android://\<hash of cert>@\<package name>"

The signon_realm is effectively the primary key used for retrieving data from the database, so it must not be empty.

### scheme
Type of the PasswordForm. See [PasswordForm_Scheme](#passwordform_scheme).

### origin
An origin URL consists of the scheme, host, port and path; the rest is stripped. This is the primary data used by the PasswordManager to decide (in longest matching prefix fashion) whether or not a given PasswordForm result from the database is a good fit for a particular form on a page. This should not be empty except for Android based credentials. See [GURL](#gurl).

### action
The action target of the form; like [origin](#origin) URL consists of the scheme, host, port and path; the rest is stripped. This is the primary data used by the PasswordManager for form autofill; that is, the action of the saved credentials must match the action of the form on the page to be autofilled. If this is empty / not available, it will result in a "restricted" IE-like autofill policy, where we wait for the user to type in their username before autofilling the password. In these cases, after successful login the action URL will automatically be assigned by the PasswordManager. See [GURL](#gurl).

When parsing an HTML form, this must always be set.

### affiliated_web_realm
The web realm affiliated with the Android application, if the form is an Android credential. Otherwise, the string is empty. If there are several realms affiliated with the application, an arbitrary realm is chosen. The field is filled out when the PasswordStore injects affiliation and branding information, i.e. in InjectAffiliationAndBrandingInformation. If there was no prior call to this method, the string is empty.

### app_display_name
The display name (e.g. Play Store name) of the Android application if the form is an Android credential. Otherwise, the string is empty. The field is filled out when the PasswordStore injects affiliation and branding information, i.e. in InjectAffiliationAndBrandingInformation. If there was no prior call to this method, the string is empty.

### app_icon_url
The icon URL (e.g. Play Store icon URL) of the Android application if the form is an Android credential. Otherwise, the URL is empty. The field is filled out when the PasswordStore injects affiliation and branding information, i.e. in InjectAffiliationAndBrandingInformation. If there was no prior call to this method, the URL is empty. See [GURL](#gurl).

### submit_element
The name of the submit button used. Optional; only used in scoring of PasswordForm results from the database to make matches as tight as possible.

When parsing an HTML form, this must always be set.

### username_element
The name of the username input element. Optional (improves scoring).

When parsing an HTML form, this must always be set.

### username_element_renderer_id
The renderer id of the username input element. It is set during the new form parsing and not persisted.

### username_may_use_prefilled_placeholder
True if the server-side classification believes that the field may be pre-filled with a placeholder in the value attribute. It is set during form parsing and not persisted.

### username_value
The username. Optional.

When parsing an HTML form, this is typically empty unless the site has implemented some form of autofill.

### all_possible_usernames
This member is populated in cases where there are multiple input elements that could possibly be the username. Used when our heuristics for determining the username are incorrect. Optional. See [ValueElementPair](#valueelementpair).

### all_possible_passwords
This member is populated in cases where there are multiple possible password values. Used in pending password state, to populate a dropdown for possible passwords. Contains all possible passwords. Optional. See [ValueElementPair](#valueelementpair).

### form_has_autofilled_value
True if [all_possible_passwords](#all_possible_passwords) have autofilled value or its part.

### password_element
The name of the input element corresponding to the current password. Optional (improves scoring).

When parsing an HTML form, this will always be set, unless it is a sign-up form or a change password form that does not ask for the current password. In these two cases the [new_password_element](#new_password_element) will always be set.

### password_element_renderer_id
The renderer id of the password input element. It is set during the new form parsing and not persisted.

### password_value
The current password. Must be non-empty for PasswordForm instances that are meant to be persisted to the password store.

When parsing an HTML form, this is typically empty.

### new_password_element
If the form was a sign-up or a change password form, the name of the input element corresponding to the new password. Optional, and not persisted.

### new_password_element_renderer_id
The renderer id of the new password input element. It is set during the new form parsing and not persisted.

### confirmation_password_element
The confirmation password element. Optional, only set on form parsing, and not persisted.

### confirmation_password_element_renderer_id
The renderer id of the confirmation password input element. It is set during the new form parsing and not persisted.

### new_password_value
The new password. Optional, and not persisted.

### preferred
True if this PasswordForm represents the last username/password login the user selected to log in to the site. If there is only one saved entry for the site, this will always be true, but when there are multiple entries the PasswordManager ensures that only one of them has a preferred bit set to true. Default to false.

When parsing an HTML form, this is not used.

### date_created
When the login was saved (by chrome). See [Time](#time).

When parsing an HTML form, this is not used.

### date_synced
When the login was downloaded from the sync server. For local passwords is not used. See [Time](#time).

When parsing an HTML form, this is not used.

### blocked_by_user
Tracks if the user opted to never remember passwords for this form. Default to false.

When parsing an HTML form, this is not used.

### type
The form type. See [PasswordForm_Type](#passwordform_type).

### times_used
The number of times that this username/password has been used to authenticate the user.

When parsing an HTML form, this is not used.

### form_data
Autofill representation of this form. Used to communicate with the Autofill servers if necessary. Currently this is only used to help determine forms where we can trigger password generation. See [FormData](#formdata).

When parsing an HTML form, this is normally set.

### generation_upload_status
What information has been sent to the Autofill server about this form. See [PasswordForm_GenerationUploadStatus](#passwordform_generationuploadstatus).

### display_name
User friendly name to show in the UI.

### icon_url
The URL of this credential's icon, such as the user's avatar, to display in the UI. See [GURL](#gurl).

### federation_origin
The origin of identity provider used for federated login. See [TupleOrigin](#tupleorigin).

### skip_zero_click
If true, Chrome will not return this credential to a site in response to 'navigator.credentials.request()' without user interaction. Once user selects this credential the flag is reseted.

### was_parsed_using_autofill_predictions
If true, this form was parsed using Autofill predictions.

### is_public_suffix_match
If true, this match was found using public suffix matching.

### is_affiliation_based_match
If true, this is a credential saved through an Android application, and found using affiliation-based match.

### submission_event
The type of the event that was taken as an indication that this form is being or has already been submitted. This field is not persisted and filled out only for submitted forms. See [SubmissionIndicatorEvent](#submissionindicatorevent).

### only_for_fallback
True iff heuristics declined this form for normal saving or filling (e.g. only credit card fields were found). But this form can be saved or filled only with the fallback.

### is_new_password_reliable
True if the new password field was found with server hints or autocomplete attributes. Only set on form parsing for filling, and not persisted. Used as signal for password generation eligibility.

## Clases

### GURL
Represents a URL. GURL is Google's URL parsing library.

A parsed canonicalized URL is guaranteed to be UTF-8. Any non-ASCII input characters are UTF-8 encoded and % escaped to ASCII.

The string representation of a URL is called the spec(). Getting the spec will assert if the URL is invalid to help protect against malicious URLs. If you want the "best effort" canonicalization of an invalid URL, you can use possibly_invalid_spec(). Test validity with is_valid(). Data and javascript URLs use GetContent() to extract the data.

This class has existence checkers and getters for the various components of a URL. Existence is different than being nonempty. "http://www.google.com/?" has a query that just happens to be empty, and has_query() will return true while the query getters will return the empty string.

Prefer not to modify a URL using string operations (though sometimes this is unavoidable). Instead, use ReplaceComponents which can replace or delete multiple parts of a URL in one step, doesn't re-canonicalize unchanged sections, and avoids some screw-ups. An example is creating a URL with a path that contains a literal '#'. Using string concatenation will generate a URL with a truncated path and a reference fragment, while ReplaceComponents will know to escape this and produce the desired result.

|Name  |Description                                     |
|------|------------------------------------------------|
|url   |Full text of the URL in canonical UTF-8.        |
|port  |Port number of the URL, or default port number. |

### Time
Represents a wall clock time in local time. Values are not guaranteed to be monotonically non-decreasing and are subject to large amounts of skew.

|Name         |Description                              |
|-------------|-----------------------------------------|
|year         |Four digit year "2007"                   |
|month        |1-based month (1 = January, etc.)        |
|day_of_week  |0-based day of week (0 = Sunday, etc.)   |
|day_of_month |1-based day of month (1-31)              |
|hour         |Hour within the current day (0-23)       |
|minute       |Minute within the current hour (0-59)    |
|second       |Second within the current minute (0-59 plus leap seconds which may take it up to 60) |
|millisecond  |Milliseconds within the current second (0-999) |

### ValueElementPair
Pair of a value and the name of the element that contained this value.

### FormData
Holds information about a form to be filled and/or submitted.

|Name                 |Description                                       |
|---------------------|--------------------------------------------------|
|id_attribute         |The id attribute of the form.                     |
|name_attribute       |The name attribute of the form.                   |
|name                 |The name by which autofill knows this form. This is generally either the name attribute or the id_attribute value, which-ever is non-empty with priority given to the name_attribute. This value is used when computing form signatures.       |
|button_titles        |Titles of form's buttons.                         |
|url                  |The URL (minus query parameters) containing the form. See [GURL](#gurl).|
|action               |The action target of the form. See [GURL](#gurl). |
|main_frame_origin    |The URL of main frame containing this form. See [TupleOrigin](#tupleorigin). |
|is_form_tag          |True if this form is a form tag.                  |
|is_formless_checkout |True if the form is made of unowned fields (i.e., not within a \<form> tag) in what appears to be a checkout flow. This attribute is only calculated and used if features::kAutofillRestrictUnownedFieldsToFormlessCheckout is enabled, to prevent heuristics from running on formless non-checkout. |
|unique_renderer_id   |Unique renderer id which is returned by function WebFormElement::UniqueRendererFormId(). It is not persistant between page loads, so it is not saved and not used in comparison in SameFormAs(). |
|submission_event     |The type of the event that was taken as an indication that this form is being or has already been submitted. This field is filled only in Password Manager for submitted password forms. |
|fields               |A vector of all the input fields in the form. See [FormFieldData](#formfielddata). |
|username_predictions |Contains unique renderer IDs of text elements which are predicted to be usernames. The order matters: elements are sorted in descending likelihood of being a username (the first one is the most likely username). Can contain IDs of elements which are not in fields. This is only used during parsing into PasswordForm, and hence not serialised for storage. |
|is_gaia_with_skip_save_password_form |True if this is a Gaia form which should be skipped on saving. |

### TupleOrigin
This class ought to be used when code needs to determine if two resources are "same-origin", and when a canonical serialization of an origin is required. Note that the canonical serialization of an origin *must not* be used to determine if two resources are same-origin.

Origin is composed of a tuple of (scheme, host, port), but contains a number of additional concepts which make it appropriate for use as a security boundary and access control mechanism between contexts. Two tuple origins are same-origin if the tuples are equal. A tuple origin may also be re-created from its serialization.

|Name   |Description                           |
|-------|--------------------------------------|
|scheme |Scheme of the url (e.g. "https").     |
|host   |Host of the url (e.g. "example.com"). |
|port   |Port of the url (e.g. 443).           |

### FormFieldData
Stores information about a field in a form.

|Name                   |Description                               |
|-----------------------|------------------------------------------|
|name                   |The name by which autofill knows this field. This is generally either the name_attribute or the id_attribute value, which-ever is non-empty with priority given to the name_attribute. This value is used when computing form signatures. |
|id_attribute           |The id attribute of the field.            |
|name_attribute         |The name attribute of the field.          |
|label                  |Label of the field.                       |
|value                  |Value of the field.                       |
|form_control_type      |Type of the form control.                 |
|autocomplete_attribute |The autocomplete attribute of the field.  |
|placeholder            |Placeholder for the value attribute.      |
|css_classes            |Css classes of the field.                 |
|aria_label             |A string that labels the current element. Used in cases where a text label is not visible. |
|aria_description       |Indicates the IDs of the elements that describe the object. Used to establish a relationship between widgets or groups and text that described them. |
|unique_renderer_id     |Unique renderer id which is returned by function UniqueRendererFormControlId(). It is not persistant between page loads, so it is not saved and not used in comparison in SameFieldAs(). |
|form_control_ax_id     |The ax node id of the form control in the accessibility tree. |
|section                |The unique identifier of the section (e.g. billing vs. shipping address) of this field. |
|max_length             |Maximum length of the field value.        |
|is_autofilled          |True if the field is autofilled.          |
|check_status           |Check status of the field control. See [FormFieldData_CheckStatus](#formfielddata_checkstatus). |
|is_focusable           |True if the field is focusable.           |
|should_autocomplete    |True if the field should autocomplete.    |
|role                   |Role of the field control. See [FormFieldData_RoleAttribute](#formfielddata_roleattribute). |
|text_direction         |Direction of the text in the field.       |
|properties_mask        |Contains combinations of FieldPropertiesFlags values. |
|is_enabled             |True if the field is enabled.             |
|is_readonly            |True if the field is readonly.            |
|typed_value            |Typed value of the field.                 |
|option_values          |Html option values of the field.          |
|option_contents        |Html option contents of the field.        |
|label_source           |Label source type of the field. See [FormFieldData_LabelSource](#formfielddata_labelsource). |


## Enums

### PasswordForm_Scheme
|Name          |Value |
|--------------|------|
|kHtml         |0     |
|kBasic        |1     |
|kDigest       |2     |
|kOther        |3     |
|kUsernameOnly |4     |

### PasswordForm_Type
|Name          |Value |
|--------------|------|
|kManual       |0     |
|kGenerated    |1     |
|kApi          |2     |

### PasswordForm_GenerationUploadStatus
|Name                |Value |
|--------------------|------|
|kNoSignalSent       |0     |
|kPositiveSignalSent |1     |
|kNegativeSignalSent |2     |
|kUnknownStatus      |10    |

### FormFieldData_CheckStatus
|Name                   |Value |
|-----------------------|------|
|kNotCheckable          |0     |
|kCheckableButUnchecked |1     |
|kChecked               |2     |

### FormFieldData_RoleAttribute
|Name          |Value |
|--------------|------|
|kPresentation |0     |
|kOther        |1     |

### FormFieldData_LabelSource
|Name          |Value |
|--------------|------|
|kUnknown      |0     |
|kLabelTag     |1     |
|kPTag         |2     |
|kDivTable     |3     |
|kTdTag        |4     |
|kDdTag        |5     |
|kLiTag        |6     |
|kPlaceHolder  |7     |
|kAriaLabel    |8     |
|kCombined     |9     |
|kValue        |10    |

### SubmissionIndicatorEvent
|Name                     |Value |
|-------------------------|------|
|NONE                     |0     |
|HTML_FORM_SUBMISSION     |1     |
|SAME_DOCUMENT_NAVIGATION |2     |
|XHR_SUCCEEDED            |3     |
|FRAME_DETACHED           |4     |
|DOM_MUTATION_AFTER_XHR   |6     |
|PROVISIONALLY_SAVED_FORM_ON_START_PROVISIONAL_LOAD |7     |
|PROBABLE_FORM_SUBMISSION |10     |
