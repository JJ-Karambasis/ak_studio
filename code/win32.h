#ifndef WIN32_H
#define WIN32_H

#include <windows.h>
#include "app.h"
#include "gl_gdi.h"

typedef struct {
	string   DLLPath;
	string   TempDLLPath;
	HMODULE  Library;
	FILETIME FileTime;
} win32_app_code;

typedef struct {
	platform Platform;
	arena*   Arena;
} win32;

typedef struct {
	gl_gdi GDI;
	HDC    DeviceContext;
} win32_gdi;

#endif