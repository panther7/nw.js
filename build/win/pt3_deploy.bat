REM Third portion of python scripts in virtual environment (Python 3)
@echo off

cd C:\nwjs
call env\Scripts\activate

python build\nw_build.py build\config.yaml pt3_deploy
if %errorlevel% NEQ 0 exit /b %errorlevel%

call deactivate
