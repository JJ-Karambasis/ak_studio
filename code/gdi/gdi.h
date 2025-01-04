#ifndef GDI_H
#define GDI_H

typedef struct gdi gdi;
typedef struct gdi_renderer gdi_renderer;
typedef struct swapchain swapchain;

typedef enum {
	GDI_RENDER_CMD_TYPE_DRAW_RECT
} gdi_render_cmd_type;

typedef struct {
	v2 Min;
	v2 Max;
	v4 Color;
} gdi_render_cmd_draw_rect;

typedef struct {
	gdi_render_cmd_type Type;
	union {
		gdi_render_cmd_draw_rect DrawRect;
	};
} gdi_render_cmd;

#define GDI_MAX_RENDER_CMD_COUNT 1024
struct gdi_renderer {
	size_t CmdCount;
	gdi_render_cmd Cmds[GDI_MAX_RENDER_CMD_COUNT];
};

#define GDI_CREATE_SWAPCHAIN_DEFINE(name) swapchain* name(gdi* GDI, HWND Window)
#define GDI_DELETE_SWAPCHAIN_DEFINE(name) void name(gdi* GDI, swapchain* Swapchain)
#define GDI_BEGIN_RENDERER_DEFINE(name) gdi_renderer* name(gdi* GDI, swapchain* Swapchain)
#define GDI_END_RENDERER_DEFINE(name) void name(gdi* GDI)

typedef GDI_CREATE_SWAPCHAIN_DEFINE(gdi_create_swapchain_func);
typedef GDI_DELETE_SWAPCHAIN_DEFINE(gdi_delete_swapchain_func);
typedef GDI_BEGIN_RENDERER_DEFINE(gdi_begin_renderer_func);
typedef GDI_END_RENDERER_DEFINE(gdi_end_renderer_func);

typedef struct {
	gdi_create_swapchain_func* CreateSwapchainFunc;
	gdi_delete_swapchain_func* DeleteSwapchainFunc;
	gdi_begin_renderer_func*   BeginRendererFunc;
	gdi_end_renderer_func*     EndRendererFunc;
} gdi_vtable;

struct gdi {
	gdi_vtable* VTable;
};

#define GDI_Create_Swapchain(...) G_Platform->GDI->VTable->CreateSwapchainFunc(G_Platform->GDI, __VA_ARGS__)
#define GDI_Delete_Swapchain(...) G_Platform->GDI->VTable->DeleteSwapchainFunc(G_Platform->GDI, __VA_ARGS__)
#define GDI_Begin_Renderer(...) G_Platform->GDI->VTable->BeginRendererFunc(G_Platform->GDI, __VA_ARGS__)
#define GDI_End_Renderer(...) G_Platform->GDI->VTable->EndRendererFunc(G_Platform->GDI)

#define GDI_CREATE_DEFINE(name) gdi* name(platform* Platform)
typedef GDI_CREATE_DEFINE(gdi_create_func);

#endif