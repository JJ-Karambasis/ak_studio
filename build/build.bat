@echo off

set base_path=%~dp0..
set code_path=%base_path%\code

if not exist %base_path%\bin mkdir %base_path%\bin

set cflags=-DOS_WIN32 -g -gcodeview -O0 -std=c11 -I%code_path% -Wall -Werror -Wno-unused-function -Wno-unused-variable

pushd %base_path%\bin
clang %cflags% -shared %code_path%\gdi\opengl\win32\win32_opengl_gdi.c -o gdi.dll -lgdi32.lib -lopengl32.lib -luser32.lib -Xlinker -EXPORT:GDI_Create
clang %cflags% -shared %code_path%\app.c -o app.dll -Xlinker -EXPORT:App_Update_Views_And_Render
clang %cflags% %code_path%\platform\win32\win32_app.c -lAdvapi32.lib -luser32.lib -o AK_Studio.exe
popd