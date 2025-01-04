#ifndef APP_H
#define APP_H

typedef struct app app;
typedef struct app_panel app_panel;


typedef enum {
	APP_PANEL_SPLIT_NONE,
	APP_PANEL_SPLIT_HORIZONTAL,
	APP_PANEL_SPLIT_VERTICAL
} app_panel_split;

struct app_panel {
	f32 			SplitPercent;
	app_panel_split Split;
	app_panel* 		LeftChild;
	app_panel* 		RightChild;
};

#define APP_UPDATE_VIEWS_AND_RENDER_DEFINE(name) void name(app* App, app_panel* RootPanel, platform* Platform, gdi_renderer* Renderer)
typedef APP_UPDATE_VIEWS_AND_RENDER_DEFINE(app_update_views_and_render_func);

struct app {
	arena* Arena;
};

#endif