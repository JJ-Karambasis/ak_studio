#include <windows.h>
#include <gl/GL.h>

#include "win32.h"
#include "shared.c"
#include "font.c"
#include "gl_gdi.c"

global app_update_and_render* App_Update_And_Render;

function void Win32_LogV(const char* Format, va_list Args) {
	char Buffer[1024];
	stbsp_vsnprintf(Buffer, Array_Count(Buffer), Format, Args);
	OutputDebugStringA(Buffer);
	OutputDebugStringA("\n");
}

function void Win32_Log(const char* Format, ...) {
	va_list List;
	va_start(List, Format);
	Win32_LogV(Format, List);
	va_end(List);
}

function void* Win32_Reserve_Memory(usize ReserveSize) {
	void* Memory = VirtualAlloc(NULL, ReserveSize, MEM_RESERVE, PAGE_READWRITE);
	return Memory;
}

function void* Win32_Commit_Memory(void* BaseCommit, usize DeltaSize) {
	void* Result = VirtualAlloc(BaseCommit, DeltaSize, MEM_COMMIT, PAGE_READWRITE);
	return Result;
}

function void Win32_Decommit_Memory(void* BaseCommit, usize DeltaSize) {
	VirtualFree(BaseCommit, DeltaSize, MEM_DECOMMIT);
}

function buffer Win32_Read_Entire_File(arena* Arena, string Path) {
	buffer Buffer = { 0 };
	
	arena* Scratch = Scratch_Get();
	wstring PathW = WString_From_String(Scratch, Path);
	HANDLE Handle = CreateFileW(PathW.Ptr, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	Scratch_Release();

	if (Handle == INVALID_HANDLE_VALUE) {
		return Buffer;
	}

	LARGE_INTEGER Size;
	GetFileSizeEx(Handle, &Size);

	Buffer.Size = U64_To_U32(Size.QuadPart);
	Buffer.Ptr = Arena_Push(Arena, Buffer.Size);

	DWORD BytesRead;
	ReadFile(Handle, Buffer.Ptr, Buffer.Size, &BytesRead, NULL);

	CloseHandle(Handle);
	return Buffer;
}

function string Win32_Get_Executable_Path(arena* Arena) {
	DWORD MemorySize = 1024;
	for (int Iterations = 0; Iterations < 32; Iterations++) {
		arena* Scratch = Scratch_Get();
		wchar_t* Buffer = Arena_Push(Scratch, sizeof(wchar_t)*MemorySize);
		DWORD Size = GetModuleFileNameW(NULL, Buffer, MemorySize);
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			string String = String_From_WString(Arena, WString(Buffer, Size));
			Scratch_Release();
			return String;
		}

		Scratch_Release();
	}

	return String_Empty();
}


function LRESULT WINAPI Win32_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	LRESULT Result = 0;
	switch (Message) {
		case WM_CLOSE: {
			PostQuitMessage(0);
		} break;

		default: {
			Result = DefWindowProcW(Window, Message, WParam, LParam);
		} break;
	}

	return Result;
}

function b32 Win32_Init_OpenGL(win32_gdi* GDI, HWND Window) {
	GDI->DeviceContext = GetDC(Window);

	PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {
		.nSize = sizeof(PIXELFORMATDESCRIPTOR),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 24,
		.cDepthBits = 16,
		.iLayerType = PFD_MAIN_PLANE
	};

	int PixelFormatIndex = ChoosePixelFormat(GDI->DeviceContext, &PixelFormatDescriptor);
	if (!PixelFormatIndex) {
		Log("Failed to choose a proper pixel format!");
		return false;
	}

	PIXELFORMATDESCRIPTOR PixelFormat;
	DescribePixelFormat(GDI->DeviceContext, PixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &PixelFormat);
	
	if (!SetPixelFormat(GDI->DeviceContext, PixelFormatIndex, &PixelFormat)) {
		Log("Failed to set the pixel format!");
		return false;
	}

	HGLRC RenderContext = wglCreateContext(GDI->DeviceContext);
	if (!RenderContext) {
		Log("Failed to create the win32 render context");
		return false;
	}

	if (!wglMakeCurrent(GDI->DeviceContext, RenderContext)) {
		Log("Failed to set the win32 opengl context");
		return false;
	}

	GL_Create((gdi*)GDI);

	return true;
}

function FILETIME Win32_Get_File_Time(string File) {
	arena* Scratch = Scratch_Get();

	wstring FileW = WString_From_String(Scratch, File);

	WIN32_FILE_ATTRIBUTE_DATA Data;
	GetFileAttributesExW(FileW.Ptr, GetFileExInfoStandard, &Data);

	Scratch_Release();

	return Data.ftLastWriteTime;
}


function b32 Win32_Load_App(win32_app_code* AppCode) {
	arena* Scratch = Scratch_Get();

	wstring AppDLLW = WString_From_String(Scratch, AppCode->DLLPath);
	wstring TempDLLW = WString_From_String(Scratch, AppCode->TempDLLPath);

	if (!CopyFileW(AppDLLW.Ptr, TempDLLW.Ptr, FALSE)) {
		Scratch_Release();
		return false;
	}

	AppCode->Library = LoadLibraryW(TempDLLW.Ptr);
	if (!AppCode->Library) {
		Scratch_Release();
		return false;
	}

	AppCode->FileTime = Win32_Get_File_Time(AppCode->DLLPath);

	Scratch_Release();

	App_Update_And_Render = (app_update_and_render *)GetProcAddress(AppCode->Library, "App_Update_And_Render");

	return true;
}

function void Win32_Unload_App(win32_app_code* AppCode) {
	if (AppCode->Library) {
		FreeLibrary(AppCode->Library);
		AppCode->Library = NULL;
	}
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	
	win32 Win32 = { 0 };
	G_Platform = &Win32.Platform;

	G_Platform->PlatformLog = Win32_Log;
	G_Platform->ReserveMemory = Win32_Reserve_Memory;
	G_Platform->CommitMemory = Win32_Commit_Memory;
	G_Platform->DecommitMemory = Win32_Decommit_Memory;
	G_Platform->ReadEntireFile = Win32_Read_Entire_File;
	AK_TLS_Create(&G_Platform->ThreadContextTLS);

	Win32.Arena = Arena_Create();

	WNDCLASSEXW WindowClass = {
		.cbSize = sizeof(WNDCLASSEXW),
		.style = CS_OWNDC,
		.lpfnWndProc = Win32_Window_Proc,
		.hInstance = Instance,
		.lpszClassName = L"AK_Studio Window Class"
	};

	RegisterClassExW(&WindowClass);

	HWND Window = CreateWindowExW(0, WindowClass.lpszClassName, L"AK_Studio", WS_OVERLAPPEDWINDOW,
								  0, 0, 0, 0, NULL, NULL, WindowClass.hInstance, NULL);
	ShowWindow(Window, SW_MAXIMIZE);

	win32_gdi Win32GDI;
	Memory_Clear(&Win32GDI, sizeof(win32_gdi));

	if (!Win32_Init_OpenGL(&Win32GDI, Window)) {
		return 1;
	}
	G_Platform->GDI = (gdi *)&Win32GDI;
	
	arena* Scratch = Scratch_Get();
	string ExecutableFilePath = Win32_Get_Executable_Path(Scratch);
	string ExecutablePath = String_Copy(Win32.Arena, String_Substr(ExecutableFilePath, 0, 
																	String_Find_Last(ExecutableFilePath, '\\')+1));
	win32_app_code AppCode = {
		.DLLPath = String_Concat(Win32.Arena, ExecutablePath, String_Lit("app.dll")),
		.TempDLLPath = String_Concat(Win32.Arena, ExecutablePath, String_Lit("temp_app.dll"))
	};
	Scratch_Release();

	if (!Win32_Load_App(&AppCode)) {
		Log("Failed to load app.dll");
		return 1;
	}

	app App = {};

	for (;;) {
		FILETIME FileTime = Win32_Get_File_Time(AppCode.DLLPath);
		if (CompareFileTime(&FileTime, &AppCode.FileTime) > 0) {
			Win32_Unload_App(&AppCode);
			for (u32 i = 0; i < 30; i++) {
				if (Win32_Load_App(&AppCode)) {
					break;
				}
				AK_Sleep(1);
			}
		}


		MSG Message;
		while (PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE)) {
			switch (Message.message) {
				case WM_QUIT: {
					return 0;
				} break;

				default: {
					TranslateMessage(&Message);
					DispatchMessageW(&Message);
				} break;
			}
		}

		gdi* GDI = G_Platform->GDI;

		RECT Rect;
		GetClientRect(Window, &Rect);
		GDI->Resolution = V2(Rect.right - Rect.left, Rect.bottom - Rect.top);
		App_Update_And_Render(&App, G_Platform);

		SwapBuffers(Win32GDI.DeviceContext);

		Assert(Thread_Context_Validate());
	}
}