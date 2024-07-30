#define GL_SRGB_ALPHA                     0x8C42
#define GL_FRAMEBUFFER_SRGB               0x8DB9


function texture* Create_Texture(const texture_create_info* CreateInfo) {
	gdi* GDI = G_Platform->GDI;
	gl_texture* glTexture = Arena_Push_Struct(GDI->Arena, gl_texture);
	texture* Texture = &glTexture->Texture;

	glGenTextures(1, &glTexture->Handle);
	glBindTexture(GL_TEXTURE_2D, glTexture->Handle);

	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE , GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, CreateInfo->Dim.w, CreateInfo->Dim.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, CreateInfo->Texels);

	glBindTexture(GL_TEXTURE_2D, 0);

	Texture->Dim = CreateInfo->Dim;

	return Texture;
}

function void GDI_Begin() {
	gdi* GDI = G_Platform->GDI;

	glViewport(0, 0, GDI->Resolution.w, GDI->Resolution.h);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GDI->Resolution.w, GDI->Resolution.h, 0.0f, -1.0f, 1.0f);

	Set_Alpha_Blend(ALPHA_BLEND_NONE);
}

function void GDI_End() {
}

function void Draw_Rect(vec2 Min, vec2 Max, vec4 Color) {
	glBegin(GL_TRIANGLES);

	glColor4f(Color.x, Color.y, Color.z, Color.w);

	glVertex2f(Min.x, Min.y);
	glVertex2f(Min.x, Max.y);
	glVertex2f(Max.x, Max.y);

	glVertex2f(Max.x, Max.y);
	glVertex2f(Max.x, Min.y);
	glVertex2f(Min.x, Min.y);

	glEnd();
}

function void Draw_Texture_Rect(vec2 Min, vec2 Max, texture* Texture) {
	gl_texture* glTexture = (gl_texture*)Texture;
	glBindTexture(GL_TEXTURE_2D, glTexture->Handle);

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Min.x, Min.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(Min.x, Max.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Max.x, Max.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Max.x, Max.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(Max.x, Min.y);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Min.x, Min.y);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
}

function void Set_Alpha_Blend(alpha_blend AlphaBlend) {
	if (AlphaBlend != ALPHA_BLEND_NONE) {
		glEnable(GL_BLEND);

		if (AlphaBlend == ALPHA_BLEND_NORMAL) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else if(AlphaBlend == ALPHA_BLEND_PREMULTIPLIED) {
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
	} else {
		glDisable(GL_BLEND);
	}
}

function void GL_Create(gdi* GDI) {
	GDI->Arena = Arena_Create();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_FRAMEBUFFER_SRGB);
}