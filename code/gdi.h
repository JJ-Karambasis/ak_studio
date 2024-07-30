#ifndef GDI_H
#define GDI_H

typedef enum {
	ALPHA_BLEND_NONE,
	ALPHA_BLEND_NORMAL,
	ALPHA_BLEND_PREMULTIPLIED
} alpha_blend;

typedef struct {
	arena* Arena;
	vec2   Resolution;
} gdi;


typedef struct {
	vec2i   	Dim;
	const void* Texels;
} texture_create_info;

typedef struct {
	vec2i Dim;
} texture;

function texture* Create_Texture(const texture_create_info* CreateInfo);

function void GDI_Begin();
function void GDI_End();
function void Draw_Rect(vec2 Min, vec2 Max, vec4 Color);
function void Draw_Texture_Rect(vec2 Min, vec2 Max, texture* Texture);
function void Set_Alpha_Blend(alpha_blend AlphaBlend);

#endif