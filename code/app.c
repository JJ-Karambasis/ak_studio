#include "app.h"
#include "shared.c"
#include "font.c"

__declspec(dllexport) APP_UPDATE_AND_RENDER(App_Update_And_Render) {
	G_Platform = Platform;

	if (!App->Initialized) {
		App->Arena = Arena_Create();
		App->Font = Font_Create(App->Arena, String_Lit("fonts/LiberationMono.ttf"), 16);
		App->Initialized = true;
	}

	GDI_Begin();
	Draw_Rect(V2(50.0f, 50.0f), V2(150.0f, 150.0f), Green4());

	glyph* Glyph = &App->Font->Glyphs['W'];
	texture* Texture = App->Font->Textures['W'];
	vec2 Min = V2(200.0f, 200.0f);
	vec2 Max = V2_Add_V2(Min, Glyph->Dim);

	Set_Alpha_Blend(ALPHA_BLEND_PREMULTIPLIED);
	Draw_Texture_Rect(Min, Max, Texture);

	Draw_Rect(V2(500.0f, 500.0f), V2(550.0f, 550.0f), Green4_With_Alpha(0.75f));

	GDI_End();
}
#include "gl_gdi.h"
#include "gl_gdi.c"