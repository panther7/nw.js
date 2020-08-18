@echo off

cd C:\nwjs
if %errorlevel% neq 0 exit /b %errorlevel%
call env\Scripts\activate
if %errorlevel% neq 0 exit /b %errorlevel%

python build\nw_build.py build\config.yaml pt1_build
if %errorlevel% neq 0 exit /b %errorlevel%

call deactivate
if %errorlevel% neq 0 exit /b %errorlevel%
