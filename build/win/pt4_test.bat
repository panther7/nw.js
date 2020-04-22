REM Set selenium
cd C:\nwjs\test\browser\javascript
if %errorlevel% neq 0 exit /b %errorlevel%
call path=C:/venv_selenium/Scripts;%path%
if %errorlevel% neq 0 exit /b %errorlevel%

REM Run selenium tests
call python ../../test.py -t 80 browser/default_browser
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/download_id
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/flags_settings
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/get_browser_registry_id
if %errorlevel% neq 0 exit /b %errorlevel%
REM call python ../../test.py -t 80 browser/history
REM if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/notification_permission_request
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/process_for_webview
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/sqlite_namespace
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/ssl_change
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/target_url_update
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/title_favicon_event
if %errorlevel% neq 0 exit /b %errorlevel%
call python ../../test.py -t 80 browser/notification_toast
if %errorlevel% neq 0 exit /b %errorlevel%
