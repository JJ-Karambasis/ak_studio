#include "app.h"
#include "shared.c"
#include "font.c"
#include "input.c"
#include "app_buffer.c"

__declspec(dllexport) APP_UPDATE_AND_RENDER(App_Update_And_Render) {
	G_Platform = Platform;

	if (!App->Initialized) {
		App->Arena = Arena_Create();
		App->Font = Font_Create(App->Arena, String_Lit("fonts/LiberationMono.ttf"), 50);
		App->Buffer = App_Buffer_Create();
		App->Initialized = true;

		App->LastTimer = AK_Query_Performance_Counter();
	}
	u64 Frequency = AK_Query_Performance_Frequency();

	u64 CurrentTimer = AK_Query_Performance_Counter();
	f64 dt = (f64)(CurrentTimer - App->LastTimer) / (f64)Frequency;
	App->LastTimer = CurrentTimer;

	input* Input = &App->Input;

	buffer_coord* Cursor = &App->Cursor;
	app_buffer* Buffer = &App->Buffer;
	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_LEFT], dt, 0.5, 0.02)) {
		if (Cursor->Column == 0) {
			if (Cursor->Line != 0) {
				Cursor->Line--;
				line* Line = Buffer->Lines.Ptr + Cursor->Line;
				Cursor->Column = Line->CodepointCount;
			}
		} else {
			Cursor->Column--;
		}
	}

	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_RIGHT], dt, 0.5, 0.02)) {
		line* Line = Buffer->Lines.Ptr + Cursor->Line;
		if (Cursor->Column == Line->CodepointCount) {
			if (Cursor->Line != (Buffer->Lines.Count-1)) {
				Cursor->Line++;
				Cursor->Column = 0;
			}
		} else {
			Cursor->Column++;
		}
	}

	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_UP], dt, 0.5, 0.02)) {
		if (Cursor->Line != 0) {
			Cursor->Line--;
			line* Line = App_Buffer_Get_Line(Buffer, Cursor->Line);
			Cursor->Column = Min(Line->CodepointCount, Cursor->Column);
		}
	}

	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_DOWN], dt, 0.5, 0.02)) {
		if (Cursor->Line != (Buffer->Lines.Count-1)) {
			Cursor->Line++;
			line* Line = App_Buffer_Get_Line(Buffer, Cursor->Line);
			Cursor->Column = Min(Line->CodepointCount, Cursor->Column);
		}
	}

	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_DELETE], dt, 0.5, 0.02)) {
		App_Buffer_Delete(App, &App->Buffer);
	}

	if (Button_Is_Down_Rate_Repeated(&Input->Keyboard[KEYBOARD_KEY_BACKSPACE], dt, 0.5, 0.02)) {
		App_Buffer_Backspace(App, &App->Buffer);
	}

	char_stream* CharStream = &Input->CharStream;
	if (CharStream->Used != 0) {
		App_Buffer_Insert(App, &App->Buffer, CharStream);
	}

	GDI_Begin();
	Set_Alpha_Blend(ALPHA_BLEND_NORMAL);


	vec2 P = V2(0.0f, 0.0f);
	font* Font = App->Font;

	for (usize LineIndex = 0; LineIndex < Buffer->Lines.Count; LineIndex++) {
		line* Line = Buffer->Lines.Ptr + LineIndex;
		u32 PrevCodepoint = (u32)-1;

		for (usize ColumnIndex = 0; ColumnIndex < Line->CodepointCount; ColumnIndex++) {
			codepoint* Codepoint = &Buffer->Codepoints.Ptr[Line->CodepointAt + ColumnIndex];
			Assert(Codepoint->Value < Array_Count(Font->Glyphs));
			glyph* Glyph = Font->Glyphs + Codepoint->Value;
			texture* Texture = Font->Textures[Codepoint->Value];

			if (PrevCodepoint != (u32)-1) {
				P.x += Font_Get_Kerning(Font, PrevCodepoint, Codepoint->Value);
			}
			
			b32 CursorDrawn = false;
			if (Cursor->Line == LineIndex && Cursor->Column == ColumnIndex) {
				vec2 Min = V2(P.x, P.y);
				vec2 Max = V2_Add_V2(Min, V2(Glyph->Advance, Font->LineHeight));
				Draw_Rect(Min, Max, White4());
				CursorDrawn = true;
			}

			if (Texture) {
				vec2 PixelP = V2(P.x, P.y + Font->Ascent);

				vec4 Color = CursorDrawn ? Black4() : White4();

				vec2 Min = V2_Add_V2(PixelP, Glyph->Offset);
				vec2 Max = V2_Add_V2(Min, Glyph->Dim);
				Draw_Texture_Rect(Min, Max, Color, Texture);
			}

			P.x += Glyph->Advance;
			PrevCodepoint = Codepoint->Value;
		}
		
		if (Cursor->Line == LineIndex && Cursor->Column == Line->CodepointCount) {
			glyph* SpaceGlyph = &Font->Glyphs[' '];

			vec2 Min = V2(P.x, P.y);
			vec2 Max = V2_Add_V2(Min, V2(SpaceGlyph->Advance, Font->LineHeight));
			Draw_Rect(Min, Max, White4());
		}

		P.y += Font->LineHeight;
		P.x = 0.0f;
	}

	GDI_End();
}
#include "gl_gdi.h"
#include "gl_gdi.c"