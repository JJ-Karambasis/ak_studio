#include <windows.h>
#include <base.h>

#include "win32_app.h"

#include <base.c>
#include <platform/platform_shared.c>

#include <gl/gl.h>

function HMONITOR Win32_Get_Primary_Monitor() {
	const POINT Pt = { 0, 0 };
	return MonitorFromPoint(Pt, MONITOR_DEFAULTTOPRIMARY);
}

function PLATFORM_RESERVE_MEMORY_DEFINE(Win32_Reserve_Memory) {
	void* Result = VirtualAlloc(NULL, Size, MEM_RESERVE, PAGE_READWRITE);
	return Result;
}

function PLATFORM_COMMIT_MEMORY_DEFINE(Win32_Commit_Memory) {
	void* Result = VirtualAlloc(Address, Size, MEM_COMMIT, PAGE_READWRITE);
	return Result;
}

function PLATFORM_DECOMMIT_MEMORY_DEFINE(Win32_Decommit_Memory) {
	if (Address) {
		VirtualFree(Address, Size, MEM_DECOMMIT);
	}
}

function PLATFORM_RELEASE_MEMORY_DEFINE(Win32_Release_Memory) {
	if (Address) {
		VirtualFree(Address, 0, MEM_RELEASE);
	}
}

global platform_vtable Win32_VTable = {
	.ReserveMemoryFunc = Win32_Reserve_Memory,
	.CommitMemoryFunc = Win32_Commit_Memory,
	.DecommitMemoryFunc = Win32_Decommit_Memory,
	.ReleaseMemoryFunc = Win32_Release_Memory,

	.AllocateMemoryFunc = Platform_Allocate_Memory,
	.FreeMemoryFunc = Platform_Free_Memory
};

#define Win32_Get() (win32*)Platform_Get()

function win32_window* Win32_Window_From_HWND(HWND Hwnd) {
	win32_window* Window = (win32_window*)GetWindowLongPtrW(Hwnd, GWLP_USERDATA);
	return Window;
}

function LRESULT Win32_Window_Proc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam) {
	win32* Win32 = Win32_Get();

	LRESULT Result = 0;
	switch (Message) {
		case WM_CREATE: {
			CREATESTRUCTW* CreateStruct = (CREATESTRUCTW*)LParam;
			SetWindowLongPtrW(Hwnd, GWLP_USERDATA, (LONG_PTR)CreateStruct->lpCreateParams);
		} break;

		case WM_CLOSE: {
			DestroyWindow(Hwnd);
		} break;

		case WM_DESTROY: {
			win32_window* Window = Win32_Window_From_HWND(Hwnd);
			if (Window) {
				Assert(Window->Handle == Hwnd);
				DLL_Remove(Win32->FirstWindow, Win32->LastWindow, Window);
				SLL_Push_Front(Win32->FreeWindows, Window);
				Win32->WindowCount--;
			}
		} break;

		default: {
			Result = DefWindowProcW(Hwnd, Message, WParam, LParam);
		} break;
	}
	return Result;
}

function b32 Win32_Init_OpenGL(win32_opengl_context* Context, HWND Window) {
	HDC DeviceContext = GetDC(Window);

	PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {
		.nSize = sizeof(PIXELFORMATDESCRIPTOR),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 32,
	};

	int PixelFormatIndex = ChoosePixelFormat(DeviceContext, &PixelFormatDescriptor);
	if (PixelFormatIndex == 0) {
		Assert(!"Failed to get a valid pixel format");
		return false;
	}

	PIXELFORMATDESCRIPTOR ActualPixelFormatDescriptor;
	DescribePixelFormat(DeviceContext, PixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &ActualPixelFormatDescriptor);

	if (!SetPixelFormat(DeviceContext, PixelFormatIndex, &ActualPixelFormatDescriptor)) {
		Assert(!"Failed to set the pixel format");
		return false;
	}

	HGLRC RenderContext = wglCreateContext(DeviceContext);
	if (!RenderContext) {
		Assert(!"Failed to create the render context");
		return false;
	}

	Context->DeviceContext = DeviceContext;
	Context->RenderContext = RenderContext;

	return true;
}

function win32_window* Win32_Create_Window(s32 Width, s32 Height, const wchar_t* WindowName, int WindowFlags, const wchar_t* ClassName) {
	win32* Win32 = Win32_Get();

	win32_window* Window = Win32->FreeWindows;
	if (Window) SLL_Pop_Front(Win32->FreeWindows);
	else Window = Arena_Push_Struct_No_Clear(Win32->Arena, win32_window);
	Memory_Clear(Window, sizeof(win32_window));

	Window->Handle = CreateWindowExW(WS_EX_APPWINDOW, ClassName, WindowName, WS_OVERLAPPEDWINDOW, 
									 CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 
									 NULL, NULL, GetModuleHandleA(0), Window);
	DLL_Push_Back(Win32->FirstWindow, Win32->LastWindow, Window);
	Win32->WindowCount++;

	ShowWindow(Window->Handle, WindowFlags);

	if (!Win32_Init_OpenGL(&Window->OpenGLContext, Window->Handle)) {
		DestroyWindow(Window->Handle);
		return NULL;
	}

	return Window;
}

function v2i Win32_Get_Window_Dim(HWND Window) {
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);

	v2i Result = V2i(ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
	return Result;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {
	win32 Win32 = { 0 };

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	
	G_Platform = &Win32.Base;
	G_Platform->VTable = &Win32_VTable;
	G_Platform->PageSize = SystemInfo.dwAllocationGranularity;

	Platform_Initialize_Memory();

	Win32.Arena = Arena_Create();

	WNDCLASSEXW WindowClass = {
		.cbSize = sizeof(WNDCLASSEXW),
		.style = CS_VREDRAW|CS_HREDRAW|CS_OWNDC,
		.lpfnWndProc = Win32_Window_Proc,
		.hInstance = GetModuleHandleA(0),
		.lpszClassName = L"AK_Studio Window Class"
	};

	RegisterClassExW(&WindowClass);

	HMONITOR MainMonitor = Win32_Get_Primary_Monitor();

	MONITORINFO MonitorInfo = { .cbSize = sizeof(MONITORINFO) };
	GetMonitorInfoA(MainMonitor, &MonitorInfo);

	s32 Width = MonitorInfo.rcWork.right - MonitorInfo.rcWork.left;
	s32 Height = MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top;

	Win32_Create_Window(Width, Height, L"AK_Studio", SW_MAXIMIZE, WindowClass.lpszClassName);
	for (;;) {

		if (!Win32.WindowCount) {
			PostQuitMessage(0);
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

		for (win32_window* Window = Win32.FirstWindow; Window; Window = Window->Next) {
			//Some update here
			
			v2i Resolution = Win32_Get_Window_Dim(Window->Handle);

			win32_opengl_context* OpenGLContext = &Window->OpenGLContext;
			wglMakeCurrent(OpenGLContext->DeviceContext, OpenGLContext->RenderContext);

			glViewport(0, 0, Resolution.x, Resolution.y);
			glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glBegin(GL_TRIANGLES);
			glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glVertex2f(0.0f, 0.5f);
			glEnd();

			SwapBuffers(OpenGLContext->DeviceContext);
		}
	}

	return 0;
}