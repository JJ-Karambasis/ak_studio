#include <base.h>
#include <gl/gl.h>
#include <gdi/gdi.h>
#include <gdi/opengl/opengl_gdi.h>
#include "win32_opengl_gdi.h"
#include <base.c>

function v2i Win32_Get_Window_Dim(HWND Window) {
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);

	v2i Result = V2i(ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
	return Result;
}

function b32 Win32_Init_OpenGL(win32_opengl_context* Context, win32_opengl_context* ShareContext, HWND Window) {
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

	if (ShareContext) {
		BOOL Status = wglShareLists(ShareContext->RenderContext, Context->RenderContext);
		Assert(Status);
	}

	return true;
}

function GDI_CREATE_SWAPCHAIN_DEFINE(Win32_OpenGL_GDI_Create_Swapchain) {
	win32_opengl_gdi* Win32OpenGL = (win32_opengl_gdi*)GDI;
	swapchain* Swapchain = Win32OpenGL->FreeSwapchains;
	if (Swapchain) SLL_Pop_Front(Win32OpenGL->FreeSwapchains);
	else Swapchain = Arena_Push_Struct_No_Clear(Win32OpenGL->Arena, swapchain);
	Memory_Clear(Swapchain, sizeof(swapchain));

	Swapchain->Window = Window;
	Win32_Init_OpenGL(&Swapchain->Context, &Win32OpenGL->Context, Swapchain->Window);
	return Swapchain;
}

function GDI_DELETE_SWAPCHAIN_DEFINE(Win32_OpenGL_GDI_Delete_Swapchain) {
	win32_opengl_gdi* Win32OpenGL = (win32_opengl_gdi*)GDI;
	if (Swapchain) {
		ReleaseDC(Swapchain->Window, Swapchain->Context.DeviceContext);
		wglDeleteContext(Swapchain->Context.RenderContext);
		Memory_Clear(Swapchain, sizeof(swapchain));
		SLL_Push_Front(Win32OpenGL->FreeSwapchains, Swapchain);
	}
}

function GDI_BEGIN_RENDERER_DEFINE(Win32_OpenGL_GDI_Begin_Renderer) {
	win32_opengl_gdi* Win32OpenGL = (win32_opengl_gdi*)GDI;
	Win32OpenGL->CurrentSwapchain = Swapchain;
	gdi_renderer* Renderer = &Swapchain->Renderer;
	Renderer->CmdCount = 0;
	return Renderer;
}

function GDI_END_RENDERER_DEFINE(Win32_OpenGL_GDI_End_Renderer) {
	win32_opengl_gdi* Win32OpenGL = (win32_opengl_gdi*)GDI;
	swapchain* Swapchain = Win32OpenGL->CurrentSwapchain;
	gdi_renderer* Renderer = &Swapchain->Renderer;

	wglMakeCurrent(Swapchain->Context.DeviceContext, Swapchain->Context.RenderContext);

	v2i WindowDim = Win32_Get_Window_Dim(Swapchain->Window);
	glViewport(0, 0, WindowDim.x, WindowDim.y);
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, WindowDim.x, WindowDim.y, 0.0f, 0.0f, 1.0f);

	for (size_t CmdIndex = 0; CmdIndex < Renderer->CmdCount; CmdIndex++) {
		gdi_render_cmd* Cmd = Renderer->Cmds + CmdIndex;
		switch (Cmd->Type) {
			case GDI_RENDER_CMD_TYPE_DRAW_RECT: {
				gdi_render_cmd_draw_rect* DrawRect = &Cmd->DrawRect;
				
				glBegin(GL_TRIANGLES);

				glColor4f(DrawRect->Color.x, DrawRect->Color.y, DrawRect->Color.z, DrawRect->Color.w);
				
				glVertex2f(DrawRect->Min.x, DrawRect->Min.y);
				glVertex2f(DrawRect->Max.x, DrawRect->Min.y);
				glVertex2f(DrawRect->Max.x, DrawRect->Max.y);
				
				glVertex2f(DrawRect->Max.x, DrawRect->Max.y);
				glVertex2f(DrawRect->Min.x, DrawRect->Max.y);
				glVertex2f(DrawRect->Min.x, DrawRect->Min.y);

				glEnd();
			} break;
		}
	}

	SwapBuffers(Swapchain->Context.DeviceContext);

	wglMakeCurrent(Win32OpenGL->Context.DeviceContext, Win32OpenGL->Context.RenderContext);
	Win32OpenGL->CurrentSwapchain = NULL;
}

global gdi_vtable Win32_OpenGL_VTable = {
	.CreateSwapchainFunc = Win32_OpenGL_GDI_Create_Swapchain,
	.DeleteSwapchainFunc = Win32_OpenGL_GDI_Delete_Swapchain,
	.BeginRendererFunc   = Win32_OpenGL_GDI_Begin_Renderer,
	.EndRendererFunc     = Win32_OpenGL_GDI_End_Renderer
};

GDI_CREATE_DEFINE(GDI_Create) {
	G_Platform = Platform;

	arena* Arena = Arena_Create();
	win32_opengl_gdi* Result = Arena_Push_Struct(Arena, win32_opengl_gdi);
	Result->Base.Base.VTable = &Win32_OpenGL_VTable;
	Result->Arena = Arena;

	{
		WNDCLASSA OpenGLWindowClass = {
			.style = CS_OWNDC,
			.lpfnWndProc = DefWindowProcA,
			.hInstance = GetModuleHandleA(0),
			.lpszClassName = "AK_Studio OpenGL Window Class"
		};
		RegisterClassA(&OpenGLWindowClass);
		Result->Window = CreateWindowA(OpenGLWindowClass.lpszClassName, "", 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandleA(0), NULL);
		Win32_Init_OpenGL(&Result->Context, NULL, Result->Window);
		wglMakeCurrent(Result->Context.DeviceContext, Result->Context.RenderContext);
	}

	return (gdi*)Result;
}