# CanonicalCookie structure

Implements one network cookie as described by [RFC 2965](http://www.rfc-editor.org/rfc/rfc2965.txt) specification, plus the ["HttpOnly" extension](https://docs.microsoft.com/en-us/previous-versions//ms533046(v=vs.85)?redirectedfrom=MSDN).

## Member variables

### name
Name of the cookie. Name is the only mandatory field, without which the cookie is not considered valid.

### value
Value of the cookie as specified in the cookie string. Note that a cookie is still valid if its value is empty.

### domain
Domain this cookie is associated with. This corresponds to the "domain" field of the cookie string. Note that the domain may start with a dot, which is not a valid hostname. However, it means this cookie matches all hostnames ending with that domain name.

### path
Path associated with this cookie. This corresponds to the "path" field of the cookie string.

### creation_date
When the cookie was created. See [Time](Chrome_PasswordForm_structure.md#time).

### expiry_date
The expiration date for this cookie. See [Time](Chrome_PasswordForm_structure.md#time).

### last_access_date
When the cookie was last accessed. See [Time](Chrome_PasswordForm_structure.md#time).

### is_secure
True if the "secure" option was specified in the cookie string, false otherwise. Secure cookies may contain private information and should not be resent over unencrypted connections.

### is_http_only
True if the "HttpOnly" flag is enabled for this cookie. A cookie that is "HttpOnly" is only set and retrieved by the network requests and replies; i.e., the HTTP protocol. It is not accessible from scripts running on browsers.

### same_site
Information about same site cookie restrictions. See [CookieSameSite](#cookiesamesite).

### priority
This field aims to reduce the impact of cookie "eviction" (i.e., deletion upon exceeding per-domain cookie capacity limits) on user experience. See [CookiePriority](#cookiepriority).

## Enums

### CookieSameSite
See https://tools.ietf.org/html/draft-ietf-httpbis-cookie-same-site-00 and https://tools.ietf.org/html/draft-ietf-httpbis-rfc6265bis for information about same site cookie restrictions.

|Name            |Value |
|----------------|------|
|UNSPECIFIED     |-1    |
|NO_RESTRICTION  |0     |
|LAX_MODE        |1     |
|STRICT_MODE     |2     |
|EXTENDED_MODE   |3     |

### CookiePriority
|Name                    |Value |
|------------------------|------|
|COOKIE_PRIORITY_LOW     |0     |
|COOKIE_PRIORITY_MEDIUM  |1     |
|COOKIE_PRIORITY_HIGH    |2     |
|COOKIE_PRIORITY_DEFAULT |1     |
