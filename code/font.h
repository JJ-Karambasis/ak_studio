#ifndef FONT_H
#define FONT_H

function void* STBTT_Malloc(arena* Arena, usize Size);
#define STBTT_malloc(size, user_data) STBTT_Malloc((arena*)user_data, size)
#define STBTT_free(p, user_data)

#include "stb_truetype.h"

typedef struct {
	vec2 Dim;
	vec2 Offset;
	f32  Advance;
} glyph;

typedef struct {
	stbtt_fontinfo FontInfo;
	f32 	 	   Ascent;
	f32 	 	   Descent;
	f32 	 	   LineHeight;
	f32 		   Scale;
	glyph    	   Glyphs[256];
	texture* 	   Textures[256];
} font;

#endif