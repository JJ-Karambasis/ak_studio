function void App_Buffer_Add_Line(app_buffer* Buffer, line* Line) {
	line_array* LineArray = &Buffer->Lines;
	memory_reserve* Reserve = &LineArray->Reserve;
	Assert(Reserve_Is_Valid(Reserve));

	usize MemoryUsed = (LineArray->Count + 1) * sizeof(line);
	if (MemoryUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, MemoryUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the line array!");
			return;
		}
	}

	LineArray->Ptr[LineArray->Count++] = *Line;
}

function void App_Buffer_Add_Codepoint(app_buffer* Buffer, u32 Codepoint) {
	codepoint_array* CodepointArray = &Buffer->Codepoints;
	memory_reserve* Reserve = &CodepointArray->Reserve;
	Assert(Reserve_Is_Valid(Reserve));

	usize MemoryUsed = (CodepointArray->Count + 1) * sizeof(codepoint);
	if (MemoryUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, MemoryUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the line array!");
			return;
		}
	}

	CodepointArray->Ptr[CodepointArray->Count++].Value = Codepoint;
}

function app_buffer App_Buffer_Create() {
	app_buffer Result = { 0 };
	Result.Codepoints.Reserve = Make_Memory_Reserve(GB(1));
	Result.Codepoints.Ptr = (codepoint*)Result.Codepoints.Reserve.BaseAddress;

	Result.Lines.Reserve = Make_Memory_Reserve(GB(1));
	Result.Lines.Ptr = (line*)Result.Lines.Reserve.BaseAddress;

	line Line = { 0 };
	App_Buffer_Add_Line(&Result, &Line);

	return Result;
}


function line* App_Buffer_Get_Line_Safe(app_buffer* Buffer, usize LineIndex) {
	Assert(LineIndex < Buffer->Lines.Count);
	return Buffer->Lines.Ptr + LineIndex;
}

function line* App_Buffer_Get_Line(app_buffer* Buffer, usize LineIndex) {
	if (LineIndex < Buffer->Lines.Count) {
		return Buffer->Lines.Ptr + LineIndex;
	}
	return NULL;
}

function line* App_Buffer_Get_Last_Line(app_buffer* Buffer) {
	return App_Buffer_Get_Line_Safe(Buffer, Buffer->Lines.Count - 1);
}

function void App_Buffer_Append(app* App, app_buffer* Buffer, char_stream* Stream) {
	const char* CStream = Char_Stream_CStr(Stream);
	line* Line = App_Buffer_Get_Last_Line(Buffer);

	buffer_coord* Cursor = &App->Cursor;
	
	utf8_reader Reader = UTF8_Reader_Begin(Char_Stream_CStr(Stream), Stream->Used);
	while (UTF8_Reader_Is_Valid(&Reader)) {
		u32 Codepoint = UTF8_Reader_Next(&Reader);
		switch (Codepoint) {
			case '\n': {
				line NewLine = {
					.CodepointAt = Line->CodepointAt+Line->CodepointCount
				};

				App_Buffer_Add_Line(Buffer, &NewLine);
				Line = App_Buffer_Get_Last_Line(Buffer);

				Cursor->Line++;
				Cursor->Column = 0;
			} break;

			default: {
				Line->CodepointCount++;
				App_Buffer_Add_Codepoint(Buffer, Codepoint);
				Cursor->Column++;
			} break;
		}
	}
}

function void App_Buffer_Insert_Line(app_buffer* Buffer, usize LineInsertIndex, line* Line) {
	line_array* LineArray = &Buffer->Lines;
	Assert(LineInsertIndex <= LineArray->Count);
	memory_reserve* Reserve = &LineArray->Reserve;
	Assert(Reserve_Is_Valid(Reserve));

	LineArray->Count++;
	usize MemoryUsed = LineArray->Count * sizeof(line);
	if (MemoryUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, MemoryUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the line array!");
			return;
		}
	}

	for (usize i = LineArray->Count - 1; i != LineInsertIndex; i--) {
		LineArray->Ptr[i] = LineArray->Ptr[i - 1];
	}

	LineArray->Ptr[LineInsertIndex] = *Line;
}

function void App_Buffer_Insert_Codepoint(app_buffer* Buffer, usize CodepointInsertIndex, u32 Codepoint) {
	codepoint_array* CodepointArray = &Buffer->Codepoints;
	Assert(CodepointInsertIndex <= CodepointArray->Count);
	memory_reserve* Reserve = &CodepointArray->Reserve;
	Assert(Reserve_Is_Valid(Reserve));

	CodepointArray->Count++;
	usize MemoryUsed = CodepointArray->Count * sizeof(line);
	if (MemoryUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, MemoryUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the line array!");
			return;
		}
	}

	for (usize i = CodepointArray->Count - 1; i != CodepointInsertIndex; i--) {
		CodepointArray->Ptr[i] = CodepointArray->Ptr[i - 1];
	}

	CodepointArray->Ptr[CodepointInsertIndex].Value = Codepoint;
}

function void App_Buffer_Delete_Line(app_buffer* Buffer, usize LineIndex) {
	line_array* Lines = &Buffer->Lines;
	for (usize i = LineIndex + 1; i < Lines->Count; i++) {
		usize PrevLine = i - 1;
		Lines->Ptr[PrevLine] = Lines->Ptr[i];
	}
	Lines->Count--;
}

function void App_Buffer_Delete_Codepoint(app_buffer* Buffer, usize CodepointIndex) {
	codepoint_array* Codepoints = &Buffer->Codepoints;
	for (usize i = CodepointIndex + 1; i < Codepoints->Count; i++) {
		usize PrevCodepoint = i - 1;
		Codepoints->Ptr[PrevCodepoint] = Codepoints->Ptr[i];
	}
	Codepoints->Count--;
}

function void App_Buffer_Delete(app* App, app_buffer* Buffer) {
	buffer_coord* Cursor = &App->Cursor;
	line* Line = App_Buffer_Get_Line_Safe(Buffer, Cursor->Line);

	if (Cursor->Column == Line->CodepointCount) {
		line* NextLine = App_Buffer_Get_Line(Buffer, Cursor->Line + 1);
		if (NextLine) {
			Line->CodepointCount += NextLine->CodepointCount;
			App_Buffer_Delete_Line(Buffer, Cursor->Line + 1);
		}
	} else {
		App_Buffer_Delete_Codepoint(Buffer, Line->CodepointAt + Cursor->Column);
		Line->CodepointCount--;
		for (usize i = Cursor->Line + 1; i < Buffer->Lines.Count; i++) {
			line* NextLine = App_Buffer_Get_Line(Buffer, i);
			NextLine->CodepointAt--;
		}
	}
}

function void App_Buffer_Backspace(app* App, app_buffer* Buffer) {
	buffer_coord* Cursor = &App->Cursor;
	line* Line = App_Buffer_Get_Line_Safe(Buffer, Cursor->Line);

	if (Cursor->Column == 0) {
		if (Cursor->Line != 0) {
			line* PrevLine = App_Buffer_Get_Line_Safe(Buffer, Cursor->Line-1);
			Cursor->Column = PrevLine->CodepointCount;
			PrevLine->CodepointCount += Line->CodepointCount;
			App_Buffer_Delete_Line(Buffer, Cursor->Line);
			Cursor->Line--;
		}
	} else {
		Cursor->Column--;
		App_Buffer_Delete(App, Buffer);
	}
}

function void App_Buffer_Insert(app* App, app_buffer* Buffer, char_stream* Stream) {
	const char* CStream = Char_Stream_CStr(Stream);
	buffer_coord* Cursor = &App->Cursor;

	line* Line = App_Buffer_Get_Line_Safe(Buffer, Cursor->Line);

	utf8_reader Reader = UTF8_Reader_Begin(Char_Stream_CStr(Stream), Stream->Used);
	while (UTF8_Reader_Is_Valid(&Reader)) {
		u32 Codepoint = UTF8_Reader_Next(&Reader);
		switch (Codepoint) {
			case '\n': {
				usize OldLineCount = Cursor->Column;
				usize NewLineCount = Line->CodepointCount - OldLineCount;
				
				Line->CodepointCount = OldLineCount;
				line NewLine = {
					.CodepointAt = Line->CodepointAt+OldLineCount,
					.CodepointCount = NewLineCount
				};

				Cursor->Line++;
				App_Buffer_Insert_Line(Buffer, Cursor->Line, &NewLine);
				Cursor->Column = 0;

				Line = App_Buffer_Get_Line_Safe(Buffer, Cursor->Line);
			} break;

			default: {
				App_Buffer_Insert_Codepoint(Buffer, Line->CodepointAt+Cursor->Column, Codepoint);
				Line->CodepointCount++;
				Cursor->Column++;

				for (usize i = Cursor->Line+1; i < Buffer->Lines.Count; i++) {
					line* NextLine = App_Buffer_Get_Line_Safe(Buffer, i);
					NextLine->CodepointAt++;
				}
			} break;
		}
	}
}