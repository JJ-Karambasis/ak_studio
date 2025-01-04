#include <base.h>
#include <gdi/gdi.h>
#include "app.h"

#include <base.c>
#include <gdi/gdi_renderer.c>

APP_UPDATE_VIEWS_AND_RENDER_DEFINE(App_Update_Views_And_Render) {
	GDI_Draw_Rect(Renderer, V2(0.0f, 0.0f), V2(50.0f, 50.0f), V4(1.0f, 1.0f, 0.0f, 1.0f));
	GDI_Draw_Rect(Renderer, V2(300.0f, 300.0f), V2(400.0f, 400.0f), V4(0.0f, 1.0f, 1.0f, 1.0f));
}