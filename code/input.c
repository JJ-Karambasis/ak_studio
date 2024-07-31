function b32 Button_Is_Down(const button* Button) {
	return Button->IsDown;
}

function b32 Button_Is_Pressed(const button* Button) {
	return Button->IsDown && !Button->WasDown;
}

function b32 Button_Is_Released(const button* Button) {
	return !Button->IsDown && Button->WasDown;
}

function b32 Button_Is_Down_Rate_Repeated(button* Button, f64 dt, f64 WaitTime, f64 RepeatTime) {
	if (Button_Is_Pressed(Button)) {
		return true;
	}
	if (Button->IsDown) {
		Button->RepeatingTime += dt;
		if (Button->IsRepeating) {
			if (Button->RepeatingTime >= RepeatTime) {
				Button->RepeatingTime -= RepeatTime;
				return true;
			}
		} else {
			if (Button->RepeatingTime >= WaitTime) {
				Button->IsRepeating = true;
				Button->RepeatingTime = 0.0f;
				return true;
			}
		}
	}
	return false;
}

function void Button_New_Frame(button* Button) {
	Button->WasDown = Button->IsDown;
	if (!Button->IsDown) {
		Button->RepeatingTime = 0.0f;
		Button->IsRepeating = false;
	}
}

function char_stream Char_Stream_Create(usize Size) {
	char_stream Result = { 0 };
	Result.Reserve = Make_Memory_Reserve(Size);
	return Result;
}

function void Char_Stream_Push(char_stream* Stream, const char* UTF8, usize Length) {
	memory_reserve* Reserve = &Stream->Reserve;
	Assert(Reserve_Is_Valid(Reserve) && UTF8 && Length);
	if (Stream->Used + Length > Reserve->ReserveSize) {
		Log("Allocated more memory in character stream than what is reserved!");
		return;
	}


	void* Result = Reserve->BaseAddress + Stream->Used;
	usize NewUsed = Stream->Used + Length;

	if (NewUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, NewUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the char stream!");
			return;
		}
	}

	Memory_Copy(Result, UTF8, Length);
	Stream->Used = NewUsed;
}

function void Char_Stream_Reset(char_stream* Stream) {
	memory_reserve* Reserve = &Stream->Reserve;
	if (!Reserve->CommitSize) {
		//If we have not allocated any memory yet, make sure that the marker is valid and return
		Assert(Stream->Used == 0);
		return;
	}

	Decommit_New_Size(&Stream->Reserve, 0);
	Stream->Used = 0;
}

function const char* Char_Stream_CStr(char_stream* Stream) {
	return (const char*)Stream->Reserve.BaseAddress;
}


function void Char_Stream_Append(char_stream* DstStream, char_stream* SrcStream) {
	Char_Stream_Push(DstStream, Char_Stream_CStr(SrcStream), SrcStream->Used);
}