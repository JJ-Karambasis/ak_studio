#ifndef WIN32_OPENGL_GDI_H
#define WIN32_OPENGL_GDI_H

typedef struct {
	HDC   DeviceContext;
	HGLRC RenderContext;
} win32_opengl_context;

struct swapchain {
	HWND 				 Window;
	win32_opengl_context Context;
	gdi_renderer 		 Renderer;
	swapchain* 			 Next;
};

typedef struct {
	opengl_gdi			 Base;
	arena*               Arena;
	HWND 				 Window;
	win32_opengl_context Context;
	swapchain* CurrentSwapchain;

	swapchain* FreeSwapchains;
} win32_opengl_gdi;

#endif