REM Build nwjs files
cd C:\nwjs\src
call ninja -C out/nw nwjs
if %errorlevel% NEQ 0 exit /b %errorlevel%

REM Generate node ninja files
cd C:\nwjs\src
call python ./build/gyp_chromium -D target_arch=ia32 -D building_nw=1 -D icu_use_data_file_flag=1 -D clang=1 -D nwjs_sdk=1 -D disable_nacl=0 -I third_party/node-nw/common.gypi third_party/node-nw/node.gyp
if %errorlevel% NEQ 0 exit /b %errorlevel%

REM Build node files
cd C:\nwjs
call ninja -C src/out/Release node
if %errorlevel% NEQ 0 exit /b %errorlevel%

REM Copy node files
cd C:\nwjs\src
call ninja -C out/nw copy_node
if %errorlevel% NEQ 0 exit /b %errorlevel%
