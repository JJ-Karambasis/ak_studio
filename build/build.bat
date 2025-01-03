@echo off

set base_path=%~dp0..
set code_path=%base_path%\code

if not exist %base_path%\bin mkdir %base_path%\bin


pushd %base_path%\bin
clang -g -gcodeview -O0 -std=c11 -I%code_path% %code_path%\platform\win32\win32_app.c -lAdvapi32.lib -luser32.lib -lgdi32.lib -lopengl32.lib -o AK_Studio.exe
popd