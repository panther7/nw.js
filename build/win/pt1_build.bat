REM First portion of python scripts in virtual environment (Python 3)
@echo off

REM Copy widevine
rmdir C:\nwjs\build\WidevineCdm /s /q
mkdir C:\nwjs\build\WidevineCdm
xcopy WidevineCdm C:\nwjs\build\WidevineCdm /s /e

cd C:\nwjs
call env\Scripts\activate

python build\nw_build.py build\config.yaml pt1_build
if %errorlevel% NEQ 0 exit /b %errorlevel%

call deactivate
