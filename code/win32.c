#include <windows.h>
#include <gl/GL.h>

#include "win32.h"
#include "shared.c"
#include "font.c"
#include "gl_gdi.c"
#include "input.c"

global app_update_and_render* App_Update_And_Render;

function int Win32_Get_VKCode(keyboard_key Key);
function int Win32_Get_Mouse_VKCode(mouse_key Key);

function void Win32_LogV(const char* Format, va_list Args) {
	arena* Scratch = Scratch_Get();
	char TmpBuffer[1];
	int TotalLength = stbsp_vsnprintf(TmpBuffer, 1, Format, Args);
	char* Buffer = Arena_Push_Array(Scratch, TotalLength + 1, char);
	stbsp_vsnprintf(Buffer, TotalLength + 1, Format, Args);
	wstring WStr = WString_From_String(Scratch, String(Buffer, TotalLength));
	OutputDebugStringW(WStr.Ptr);
	Scratch_Release();
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

function void Win32_Add_To_Char_Stream(char_stream* Stream, wchar_t* Char) {
	char UTF8[4];
	u32 Codepoint = UTF16_Read(Char, NULL);
	u32 Length = UTF8_Write(UTF8, Codepoint);
	Char_Stream_Push(Stream, UTF8, Length);
}

function LRESULT WINAPI Win32_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	LRESULT Result = 0;
	
	win32* Win32 = (win32*)G_Platform;
	input* Input = Win32->Input;

	switch (Message) {
		case WM_CHAR: {
			//Skip backspace as it will be handled by the keyboard input
			b32 IsValidChar = WParam != '\b';

			if (IsValidChar) {
				char_stream* CharStream = &Input->CharStream;

				wchar_t* Param = (wchar_t*)&WParam;
				if (WParam == '\r') {
					Param = L"\n"; //Make carriage return always output a newline
				}

				Win32_Add_To_Char_Stream(CharStream, Param);
			}
		} break;
		
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
	
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	win32 Win32 = { 0 };
	G_Platform = &Win32.Platform;

	G_Platform->PageSize = SystemInfo.dwAllocationGranularity;
	G_Platform->PlatformLog = Win32_Log;
	G_Platform->ReserveMemory = Win32_Reserve_Memory;
	G_Platform->CommitMemory = Win32_Commit_Memory;
	G_Platform->DecommitMemory = Win32_Decommit_Memory;
	G_Platform->ReadEntireFile = Win32_Read_Entire_File;
	AK_TLS_Create(&G_Platform->ThreadContextTLS);

	Win32.Arena = Arena_Create();

	app App = {};
	input* Input = &App.Input;
	Win32.Input = Input;

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

	Input->CharStream = Char_Stream_Create(GB(1));
	char_stream* CharStream = &Input->CharStream;

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

		for (u32 i = 0; i < Array_Count(Input->Keyboard); i++) {
			Button_New_Frame(&Input->Keyboard[i]);
			Input->Keyboard[i].IsDown = false;
		}

		for (u32 i = 0; i < Array_Count(Input->Mouse); i++) {
			Button_New_Frame(&Input->Mouse[i]);
			Input->Mouse[i].IsDown = false;
		}

		Char_Stream_Reset(CharStream);

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

		for (u32 i = 0; i < Array_Count(Input->Keyboard); i++) {
			int VKCode = Win32_Get_VKCode((keyboard_key)i);
			if (GetAsyncKeyState(VKCode) & 0x8000) {
				Input->Keyboard[i].IsDown = true;
			}
		}

		for (u32 i = 0; i < Array_Count(Input->Mouse); i++) {
			int VKCode = Win32_Get_Mouse_VKCode((mouse_key)i);
			if (GetAsyncKeyState(VKCode) & 0x8000) {
				Input->Mouse[i].IsDown = true;
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

function int Win32_Get_VKCode(keyboard_key Key) {
	if (Key < 256) {
		return (int)Key;
	}
	
	int Index = Key - 256;
	Assert(Index < (KEYBOARD_KEY_COUNT - 256));
	local const int VKCodes[] = {
		VK_MENU,
		VK_SHIFT,
		VK_CONTROL,
		VK_F1,
		VK_F2,
		VK_F3,
		VK_F4,
		VK_F5,
		VK_F6,
		VK_F7,
		VK_F8,
		VK_F9,
		VK_F10,
		VK_F11,
		VK_F12,
		VK_LEFT,
		VK_RIGHT,
		VK_DOWN,
		VK_UP,
		VK_DELETE,
		VK_BACK
	};

	Static_Assert(Array_Count(VKCodes) == KEYBOARD_KEY_COUNT-256);
	return VKCodes[Index];
}

function int Win32_Get_Mouse_VKCode(mouse_key Key) {
	local const int VKCodes[] = {
		VK_LBUTTON,
		VK_MBUTTON,
		VK_RBUTTON
	};
	Assert(Key < MOUSE_KEY_COUNT);
	Static_Assert(Array_Count(VKCodes) == MOUSE_KEY_COUNT);
	return VKCodes[Key];
}