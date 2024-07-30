function void* STBTT_Malloc(arena* Arena, usize Size) {
	return Arena_Push(Arena, Size);
}

function font* Font_Create(arena* Arena, string Path, u32 FontSize) {
	buffer Buffer = Read_Entire_File(Arena, Path);
	if (Buffer_Is_Empty(Buffer)) {
		Log("Failed to load font file %.*s", Path.Size, Path.Ptr);
		return NULL;
	}

	arena* Scratch = Scratch_Get();

	font* Result = Arena_Push_Struct(Arena, font);
	stbtt_InitFont(&Result->FontInfo, Buffer.Ptr, 0);

	int Ascent, Descent, LineGap;
	stbtt_GetFontVMetrics(&Result->FontInfo, &Ascent, &Descent, &LineGap);

	Result->Scale = stbtt_ScaleForPixelHeight(&Result->FontInfo, (f32)FontSize);
	Result->Ascent = (f32)Abs(Ascent)*Result->Scale;
	Result->Descent = (f32)Abs(Descent)*Result->Scale;
	Result->LineHeight = (Ascent - Descent + LineGap)*Result->Scale;

	Result->FontInfo.userdata = Scratch;

	for (u32 i = 0; i < Array_Count(Result->Glyphs); i++) {
		glyph* Glyph = Result->Glyphs + i;

		int Width, Height, XOffset, YOffset;
		const u8* SrcTexels = stbtt_GetCodepointBitmap(&Result->FontInfo, Result->Scale, Result->Scale, i, &Width, &Height, &XOffset, &YOffset);

		//Pad texels to RGBA instead of single 8bit channel
		u8* DstTexels = Arena_Push_Array(Scratch, Width * Height * 4, u8);

		u8* Dst = DstTexels;
		for (u32 y = 0; y < Height; y++) {
			for (u32 x = 0; x < Width; x++) {
				u8 Value = *SrcTexels++;
				*Dst++ = Value;
				*Dst++ = Value;
				*Dst++ = Value;
				*Dst++ = Value;
			}
		}

		int Advance;
		stbtt_GetCodepointHMetrics(&Result->FontInfo, i, &Advance, NULL);

		Glyph->Offset = V2((f32)XOffset, (f32)YOffset);
		Glyph->Dim = V2((f32)Width, (f32)Height);
		Glyph->Advance = (f32)Advance*Result->Scale;

		texture_create_info TextureCreateInfo = {
			.Dim = V2i(Width, Height),
			.Texels = DstTexels
		};

		Result->Textures[i] = Create_Texture(&TextureCreateInfo);
	}

	Scratch_Release();
	return Result;

}