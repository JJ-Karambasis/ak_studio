#ifndef GL_GDI_H
#define GL_GDI_H

#include <gl/GL.h>

typedef struct {
	gdi GDI;
} gl_gdi;

typedef struct {
	texture Texture;
	GLuint  Handle;
} gl_texture;

#endif