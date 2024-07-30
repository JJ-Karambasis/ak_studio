@echo off

for %%a in (%*) do set "%%a=1"

set base_path=%~dp0..\ak_studio
set code_path=%base_path%\code
set data_path=%base_path%\data
set shader_path=%code_path%\shaders
set shader_out_path=%data_path%\shaders

set warnings=-Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-char-subscripts -Wno-switch
set common=-g -std=c11 -DSTB_SPRINTF_IMPLEMENTATION -DAK_ATOMIC_IMPLEMENTATION -DSTB_TRUETYPE_IMPLEMENTATION -D_CRT_SECURE_NO_WARNINGS -DOS_WINDOWS

if "%Win32%"=="1" set common=%common% -m32
if "%Debug%"=="1" set common=%common% -O0 -DDEBUG_BUILD
if "%Release%"=="1" set common=%common% -O3

set obj_postfix=
if "%Win32%"=="1"   set obj_postfix=%obj_postfix%_x86
if "%x64%"=="1"     set obj_postfix=%obj_postfix%_x64
if "%Debug%"=="1"   set obj_postfix=%obj_postfix%_debug
if "%Release%"=="1" set obj_postfix=%obj_postfix%_release

pushd %base_path%\bin
	clang %warnings% %common% -shared -o app.dll %code_path%\app.c -lopengl32.lib
	clang %warnings% %common% -o ak_studio.exe %code_path%\win32.c -luser32.lib -lgdi32.lib -lopengl32.lib
popd

exit /b 0