#ifndef WIN32_APP_H
#define WIN32_APP_H

typedef struct win32_window win32_window;

struct win32_window {
	HWND 		  Handle;
	swapchain*    Swapchain;
	app_panel* 	  RootPanel;
	win32_window* Next;
	win32_window* Prev;
};

typedef struct {
	platform Base;
	arena* Arena;
	gdi* GDI;

	win32_window* 		 FirstWindow;
	win32_window* 		 LastWindow;
	win32_window* 		 FreeWindows;
	u32 		  		 WindowCount;
} win32;

#endif