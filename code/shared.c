function u32 U64_To_U32(u64 V) {
	Assert(V <= 0xFFFFFFFF);
	return (u32)V;
}

function u32 Ceil_U32(f32 V) {
	Assert(V >= 0.0f);
	u32 Result = (u32)ceilf(V);
	return Result;
}

function f32 Sin(f32 V) {
	return sinf(V);
}

function f32 Cos(f32 V) {
	return cosf(V);
}

function f32 Tan(f32 V) {
	return tanf(V);
}

function f32 Sqrt(f32 V) {
	return sqrtf(V);
}

function f32 SNorm(s16 Value) {
    return Clamp(-1.0f, (f32)Value / (f32)((1 << 15) - 1), 1.0f);
}

function s16 SNorm_S16(f32 Value) {
    s16 Result = (s16)(Clamp(-1.0f, Value, 1.0f) * (f32)(1 << 15));
    return Result;
}

function f32 UNorm_To_SNorm_F32(u8 UNorm) {
	return ((f32)UNorm / 128.0f) - 1.0f;
}

function s32 F32_To_S32_Bits(f32 V) {
	s32 Result = *(s32 *)&V;
	return Result;
}

function f32 S32_To_F32_Bits(s32 V) {
	f32 Result = *(f32 *)&V;
	return Result;
}

function b32 Is_Nan(f32 V) {
	return isnan(V);
}

function b32 Is_Finite(f32 V) {
	return isfinite(V);
}

function vec2i V2i(s32 x, s32 y) {
	vec2i Result = { .x = x, .y = y };
	return Result;
}

function vec2i V2i_Zero() {
	vec2i Result = V2i(0, 0);
	return Result;
}

function vec2 V2(f32 x, f32 y) {
	vec2 Result = { .x = x, .y = y };
	return Result;
}

function vec2 V2_Add_V2(vec2 A, vec2 B) {
	vec2 Result = V2(A.x + B.x, A.y + B.y);
	return Result;
}

function s64 Pack_S64(vec2i Data) {
	s64 PackedX = ((s64)Data.x) & 0xFFFFFFFFLL;
    s64 PackedY = ((s64)Data.y) & 0xFFFFFFFFLL;
	s64 Result = PackedX | (PackedY << 32);
	return Result;
}

function vec2i Unpack_S64(s64 PackedData) {
	s32 X = (s32)PackedData;
	s32 Y = (s32)(PackedData >> 32);
	return V2i(X, Y);
}

function s64 Pack_F32_To_S64(vec2 Data) {
	s32 X = F32_To_S32_Bits(Data.x);
	s32 Y = F32_To_S32_Bits(Data.y);
	return Pack_S64(V2i(X, Y));
}

function vec2 Unpack_S64_To_F32(s64 PackedData) {
	vec2i Data = Unpack_S64(PackedData);
	f32 X = S32_To_F32_Bits(Data.x);
	f32 Y = S32_To_F32_Bits(Data.y);
	return V2(X, Y);
}

function u32 Ceil_Pow2_U32(u32 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V++;
    return V;
}

function u64 Ceil_Pow2_U64(u64 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    V++;
    return V;
}

#ifdef BITNESS_32
function usize Ceil_Pow2_Size(usize Size) {
	return Ceil_Pow2_U32(Size);
}

function u32 USize_To_U32(usize Size) {
	return (u32)Size;
}

#else
function usize Ceil_Pow2_Size(usize Size) {
	return Ceil_Pow2_U64(Size);
}

function u32 USize_To_U32(usize Size) {
	return U64_To_U32((u32)Size);
}
#endif

function void Memory_Copy(void* Dst, const void* Src, usize Size) {
	memcpy(Dst, Src, Size);
}

function void Memory_Clear(void* Memory, usize Size) {
	memset(Memory, 0, Size);
}

function usize Align_Size(usize Value, usize Alignment) {
    usize Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

function bool Equal_Zero_Approx(f32 Value, f32 Epsilon) {
    return Abs(Value) <= Epsilon;
}

function bool Equal_Zero_Eps(f32 Value) {
    return Equal_Zero_Approx(Value, FLT_EPSILON);
}

function bool Equal_Zero_Eps_Sq(f32 SqValue) {
    return Equal_Zero_Approx(SqValue, Sq(FLT_EPSILON));
}

function void* Allocate_Memory(usize Size) {
//	AK_Mutex_Lock(&G_Platform->HeapLock);
//	void* Memory = Heap_Alloc(G_Platform->Heap, Size);
//	AK_Mutex_Unlock(&G_Platform->HeapLock);
	void* Result = malloc(Size);
	if (Result) {
		Memory_Clear(Result, Size);
	}
	return Result;
}

function void Free_Memory(void* Memory) {
//	if (Memory) {
//		AK_Mutex_Lock(&G_Platform->HeapLock);
//		Heap_Free(G_Platform->Heap, Size);
//		AK_Mutex_Unlock(&G_Platform->HeapLock);
//	}
	free(Memory);
}

function memory_reserve Make_Memory_Reserve(usize Size) {
	memory_reserve Result = { 0 };

	Size = Align_Size(Size, G_Platform->PageSize);
	Result.BaseAddress = (u8*)Reserve_Memory(Size);
	if (!Result.BaseAddress) {
		return Result;
	}

	Result.ReserveSize = Size;
	Result.CommitSize  = 0;
	return Result;
}

function b32 Reserve_Is_Valid(memory_reserve* Reserve) {
	return (Reserve->BaseAddress != NULL && Reserve->ReserveSize != 0);
}

function void* Commit_New_Size(memory_reserve* Reserve, usize NewSize) {
	//Get the proper offset to the commit address and find the new commit size.
	//Committed size must align on a page boundary
	u8* BaseCommit = Reserve->BaseAddress + Reserve->CommitSize;
	usize NewCommitSize = Align_Size(NewSize, G_Platform->PageSize);
	usize DeltaSize = NewCommitSize - Reserve->CommitSize;

	if (NewCommitSize > Reserve->ReserveSize) {
		Log("Allocated more memory what was reserved!");
		return NULL;
	}

	Assert(NewCommitSize <= Reserve->ReserveSize);
	Assert((NewCommitSize % G_Platform->PageSize) == 0);
	Assert((DeltaSize % G_Platform->PageSize) == 0);

	//Commit the new memory pages
	void* CommitMemory = Commit_Memory(BaseCommit, DeltaSize); 
	if (!CommitMemory) {
		Log("Failed to commit more memory!");
		return NULL;
	}

	Reserve->CommitSize = NewCommitSize;
	return CommitMemory;
}

function void Decommit_New_Size(memory_reserve* Reserve, usize NewSize) {
	if (!Reserve->CommitSize) {
		//If we have not allocated any memory yet, make sure that the marker is valid and return
		Assert(NewSize == 0);
		return;
	}

	usize NewCommitSize = Align_Size(NewSize, G_Platform->PageSize);
	Assert(Reserve->CommitSize >= NewCommitSize);
	usize DeltaSize = Reserve->CommitSize - NewCommitSize;

	Assert((NewCommitSize % G_Platform->PageSize) == 0);
	Assert((DeltaSize % G_Platform->PageSize) == 0);

	if (DeltaSize) {
		//If we have more memory than needed, decommit the pages
		u8* BaseCommit = Reserve->BaseAddress + NewCommitSize;
		Decommit_Memory(BaseCommit, DeltaSize);
		Reserve->CommitSize = NewCommitSize;
	}
}

function arena* Arena_Create_With_Size(usize ReserveSize) {
	arena* Arena = (arena*)Allocate_Memory(sizeof(arena));
	if (Arena) {
		Arena->Reserve = Make_Memory_Reserve(ReserveSize);

		if (!Arena->Reserve.BaseAddress) {
			Log("Failed to reserve the arena memory!");
			Free_Memory(Arena);
			Arena = NULL;
		}
	} else {
		Log("Failed to allocate memory for the arena!");
	}

	return Arena;
}

function arena* Arena_Create() {
	return Arena_Create_With_Size(GB(1));
}

function b32 Arena_Is_Valid(arena* Arena) {
	return Arena && Reserve_Is_Valid(&Arena->Reserve);
}

function void* Arena_Push_Aligned_No_Clear(arena* Arena, usize Size, usize Alignment) {
	Assert(Arena_Is_Valid(Arena));
	
	if (!Size) {
		return NULL;
	}

	memory_reserve* Reserve = &Arena->Reserve;
	
	//Handle alignment
	Assert(Is_Pow2(Alignment));
	usize NewUsed = Align_Pow2(Arena->Used, Alignment);

	//Get the aligned memory address and increment the used value
	void* Result = Reserve->BaseAddress + NewUsed;
	NewUsed += Size;

	//Check if we need to commit more memory
	if (NewUsed > Reserve->CommitSize) {
		void* CommitMemory = Commit_New_Size(Reserve, NewUsed);
		if (!CommitMemory) {
			Log("Failed to commit more memory for the arena!");
			return NULL;
		}
	}

	//Set the new used value before returning the address
	Arena->Used = NewUsed;
	return Result;
}

function void* Arena_Push_No_Clear(arena* Arena, usize Size) {
	return Arena_Push_Aligned_No_Clear(Arena, Size, DEFAULT_ALIGNMENT);
}

function void* Arena_Push(arena* Arena, usize Size) {
	void* Memory = Arena_Push_No_Clear(Arena, Size);
	if (Memory) {
		Memory_Clear(Memory, Size);
	}
	return Memory;
}

#define Arena_Push_Struct(arena, type) (type *)Arena_Push(arena, sizeof(type))
#define Arena_Push_Array(arena, count, type) (type *)Arena_Push(arena, (count) * sizeof(type))

function arena_marker Arena_Get_Marker(arena* Arena) {
	arena_marker Result = {
		.Arena = Arena,
		.Used  = Arena->Used
	};
	return Result;
}

function void Arena_Set_Marker(arena* Arena, arena_marker Marker) {
	Assert(Arena == Marker.Arena);
	memory_reserve* Reserve = &Arena->Reserve;
	Decommit_New_Size(Reserve, Marker.Used);
	//Finally set the arena's used value to the marker
	Arena->Used = Marker.Used;
}

function void Arena_Clear(arena* Arena) {
	arena_marker Marker = { .Arena = Arena, .Used = 0 };
	Arena_Set_Marker(Arena, Marker);
}

function thread_context* Thread_Context_Get() {
	thread_context* Result = (thread_context*)AK_TLS_Get(&G_Platform->ThreadContextTLS);
	if (!Result) {
		Result = (thread_context*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(thread_context));
		AK_TLS_Set(&G_Platform->ThreadContextTLS, Result);
	}
	return Result;
}

function b32 Thread_Context_Validate() {
	thread_context* ThreadContext = Thread_Context_Get();
	b32 Result = true;
	if (ThreadContext->ScratchIndex != 0) {
		Log("Thread context has invalid scratch memory usage. %d scratch arenas have not been released!", 
			ThreadContext->ScratchIndex);
		Result = false;
	}

	for (u32 i = 0; i < Array_Count(ThreadContext->ScratchArenas); i++) {
		if (ThreadContext->ScratchArenas[i]) {
			if (ThreadContext->ScratchArenas[i]->Used != 0) {
				Log("Thread context %llu has scratch arena %d with used memory %llu. Memory leak detected!", 
					AK_Thread_Get_Current_ID(), i, ThreadContext->ScratchArenas[i]->Used);
				Result = false;
			}
		}
	}

	return Result;
}

function b32 Buffer_Is_Empty(buffer Buffer) {
	return !Buffer.Size || !Buffer.Ptr;
}

function arena* Scratch_Get() {
	thread_context* ThreadContext = Thread_Context_Get();
	u32 ScratchIndex = ThreadContext->ScratchIndex++;
	Assert(ThreadContext->ScratchIndex <= MAX_SCRATCH_COUNT);
	if (!ThreadContext->ScratchArenas[ScratchIndex]) {
		ThreadContext->ScratchArenas[ScratchIndex] = Arena_Create();
	}

	arena* Result = ThreadContext->ScratchArenas[ScratchIndex];
	ThreadContext->ScratchMarkers[ScratchIndex] = Arena_Get_Marker(Result);
	return Result;
}

function void Scratch_Release() {
	thread_context* ThreadContext = Thread_Context_Get();
	Assert(ThreadContext->ScratchIndex);
	ThreadContext->ScratchIndex--;
	u32 ScratchIndex = ThreadContext->ScratchIndex;
	Arena_Set_Marker(ThreadContext->ScratchArenas[ScratchIndex], ThreadContext->ScratchMarkers[ScratchIndex]);
}

global const u8 G_ClassUTF8[32] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

function u32 UTF8_Read(const char* Str, u32* OutLength) {
    u32 Result = 0xFFFFFFFF;
    
    u8 Byte = (u8)Str[0];
    u8 ByteClass = G_ClassUTF8[Byte >> 3];
    
    local const u32 BITMASK_3 = 0x00000007;
    local const u32 BITMASK_4 = 0x0000000f;
    local const u32 BITMASK_5 = 0x0000001f;
    local const u32 BITMASK_6 = 0x0000003f;
    
    switch(ByteClass)
    {
        case 1:
        {
            Result = Byte;
        } break;
        
        case 2:
        {
            u8 NextByte = (u8)Str[1];
            if(G_ClassUTF8[NextByte >> 3] == 0)
            {
                Result = (Byte & BITMASK_5) << 6;
                Result |= (NextByte & BITMASK_6);
            }
        } break;
        
        case 3:
        {
            u8 NextBytes[2] = {(u8)Str[1], (u8)Str[2]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[1] >> 3] == 0)
            {
                Result = (Byte & BITMASK_4) << 12;
                Result |= ((NextBytes[0] & BITMASK_6) << 6);
                Result |= (NextBytes[1] & BITMASK_6);
            }
        } break;
        
        case 4:
        {
            u8 NextBytes[3] = {(u8)Str[1], (u8)Str[2], (u8)Str[3]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[1] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[2] >> 3] == 0)
            {
                Result = (Byte & BITMASK_3) << 18;
                Result |= ((NextBytes[0] & BITMASK_6) << 12);
                Result |= ((NextBytes[1] & BITMASK_6) << 6);
                Result |= (NextBytes[2] & BITMASK_6);
            }
        } break;
    }
    
    if(OutLength) *OutLength = ByteClass;
    return Result;
}

function u32 UTF8_Write(char* Str, u32 Codepoint) {
    local const u32 BIT_8 = 0x00000080;
    local const u32 BITMASK_2 = 0x00000003;
    local const u32 BITMASK_3 = 0x00000007;
    local const u32 BITMASK_4 = 0x0000000f;
    local const u32 BITMASK_5 = 0x0000001f;
    local const u32 BITMASK_6 = 0x0000003f;
    
    u32 Result = 0;
    if (Codepoint <= 0x7F)
    {
        Str[0] = (char)Codepoint;
        Result = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        Str[0] = (char)((BITMASK_2 << 6) | ((Codepoint >> 6) & BITMASK_5));
        Str[1] = (char)(BIT_8 | (Codepoint & BITMASK_6));
        Result = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        Str[0] = (char)((BITMASK_3 << 5) | ((Codepoint >> 12) & BITMASK_4));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 6) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ( Codepoint       & BITMASK_6));
        Result = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        Str[0] = (char)((BITMASK_4 << 3) | ((Codepoint >> 18) & BITMASK_3));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 12) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ((Codepoint >>  6) & BITMASK_6));
        Str[3] = (char)(BIT_8 | ( Codepoint        & BITMASK_6));
        Result = 4;
    }
    else
    {
        Str[0] = '?';
        Result = 1;
    }
    
    return Result;
}

function u32 UTF16_Read(const wchar_t* Str, u32* OutLength) {
    u32 Offset = 1;
    u32 Result = *Str;
    if (0xD800 <= Str[0] && Str[0] < 0xDC00 && 0xDC00 <= Str[1] && Str[1] < 0xE000)
    {
        Result = (u32)(((Str[0] - 0xD800) << 10) | (Str[1] - 0xDC00));
        Offset = 2;
    }
    if(OutLength) *OutLength = Offset;
    return Result;
}

function u32 UTF16_Write(wchar_t* Str, u32 Codepoint) {
    local const u32 BITMASK_10 = 0x000003ff;
    
    u32 Result = 0;
    if(Codepoint == 0xFFFFFFFF)
    {
        Str[0] = (wchar_t)'?';
        Result = 1;
    }
    else if(Codepoint < 0x10000)
    {
        Str[0] = (wchar_t)Codepoint;
        Result = 1;
    }
    else
    {
        Codepoint -= 0x10000;
        Str[0] = (wchar_t)(0xD800 + (Codepoint >> 10));
        Str[1] = (wchar_t)(0xDC00 + (Codepoint & BITMASK_10));
        Result = 2;
    }
    
    return Result;
}

#define STRING_INVALID_INDEX ((usize)-1)

function string String_Empty() {
	string Result = { 0 };
	return Result;
}

function string String(const char* Ptr, usize Size) {
	string Result = {
		.Ptr = Ptr,
		.Size = Size
	};
	return Result;
}

function string String_FormatV(arena* Arena, const char* Format, va_list Args) {
	char TmpBuffer[1];
	int TotalLength = stbsp_vsnprintf(TmpBuffer, 1, Format, Args);
	char* Buffer = Arena_Push_Array(Arena, TotalLength + 1, char);
	stbsp_vsnprintf(Buffer, TotalLength + 1, Format, Args);
	return String(Buffer, TotalLength);
}

function string String_Format(arena* Arena, const char* Format, ...) {
	va_list List;
	va_start(List, Format);
	string Result = String_FormatV(Arena, Format, List);
	va_end(List);
	return Result;
}

function string String_Copy(arena* Arena, string Str) {
	char* Ptr = Arena_Push_Array(Arena, Str.Size+1, char);
	Memory_Copy(Ptr, Str.Ptr, Str.Size * sizeof(char));
	Ptr[Str.Size] = 0;
	return String(Ptr, Str.Size);
}

function b32 String_Is_Empty(string String) {
	return !String.Ptr || !String.Size;
}

function b32 String_Equals(string StringA, string StringB) {
	if (StringA.Size != StringB.Size) return false;

	for (usize i = 0; i < StringA.Size; i++) {
		if (StringA.Ptr[i] != StringB.Ptr[i]) {
			return false;
		}
	}

	return true;
}

function string String_Concat(arena* Arena, string StringA, string StringB) {
	usize TotalSize = StringA.Size + StringB.Size;
	char* Ptr = Arena_Push_Array(Arena, TotalSize + 1, char);
	Memory_Copy(Ptr, StringA.Ptr, StringA.Size * sizeof(char));
	Memory_Copy(Ptr + StringA.Size, StringB.Ptr, StringB.Size * sizeof(char));
	Ptr[TotalSize] = 0;
	return String(Ptr, TotalSize);
}

function usize String_Find_Last(string String, char Character) {
	for (usize Index = String.Size; Index != 0; Index--) {
		usize ArrayIndex = Index - 1;
		if (String.Ptr[ArrayIndex] == Character) {
			return ArrayIndex;
		}
	}

	return STRING_INVALID_INDEX;
}

function string String_Substr(string Str, usize FirstIndex, usize LastIndex) {
	Assert(LastIndex > FirstIndex);
	Assert(LastIndex != STRING_INVALID_INDEX && LastIndex != STRING_INVALID_INDEX);
	const char* At = Str.Ptr + FirstIndex;
	return String(At, LastIndex - FirstIndex);
}

function string String_From_WString(arena* Arena, wstring WString) {
	arena* Scratch = Scratch_Get();

    const wchar_t* WStrAt = WString.Ptr;
    const wchar_t* WStrEnd = WStrAt+WString.Size;

    char* StrStart = (char*)Arena_Push(Scratch, (WString.Size*4)+1);
    char* StrEnd = StrStart + WString.Size*4;
    char* StrAt = StrStart;

    for(;;) {
        Assert(StrAt <= StrEnd);
        if(WStrAt >= WStrEnd) {
            Assert(WStrAt == WStrEnd);
            break;
        }

        u32 Length;
        u32 Codepoint = UTF16_Read(WStrAt, &Length);
        WStrAt += Length;
        StrAt += UTF8_Write(StrAt, Codepoint);
    }

    *StrAt = 0;
	string Result = String_Copy(Arena, String(StrStart, (usize)(StrAt - StrStart)));
	Scratch_Release();
	return Result;
}

function usize WString_Length(const wchar_t* Ptr) {
	return (usize)wcslen(Ptr);
}

function wstring WString(const wchar_t* Ptr, usize Size) {
	wstring Result = {
		.Ptr = Ptr,
		.Size = Size
	};
	return Result;
}

function wstring WString_Copy(arena* Arena, wstring WStr) {
	wchar_t* Ptr = Arena_Push_Array(Arena, WStr.Size+1, wchar_t);
	Memory_Copy(Ptr, WStr.Ptr, WStr.Size * sizeof(wchar_t));
	Ptr[WStr.Size] = 0;
	return WString(Ptr, WStr.Size);
}

function wstring WString_From_String(arena* Arena, string String) {
    arena* Scratch = Scratch_Get();

    const char* StrAt = String.Ptr;
    const char* StrEnd = StrAt+String.Size;

    wchar_t* WStrStart = (wchar_t*)Arena_Push(Scratch, (String.Size+1)*sizeof(wchar_t));
    wchar_t* WStrEnd = WStrStart + String.Size;
    wchar_t* WStrAt = WStrStart;

    for(;;) {
        Assert(WStrAt <= WStrEnd);
        if(StrAt >= StrEnd) {
            Assert(StrAt == StrEnd);
            break;
        }

        u32 Length;
        u32 Codepoint = UTF8_Read(StrAt, &Length);
        StrAt += Length;
        WStrAt += UTF16_Write(WStrAt, Codepoint);
    }

    *WStrAt = 0; 
    wstring Result = WString_Copy(Arena, WString(WStrStart, (usize)(WStrAt-WStrStart)));
	Scratch_Release();
	return Result;
}

function wstring WString_Substr(wstring Str, usize FirstIndex, usize LastIndex) {
	Assert(LastIndex > FirstIndex);
	Assert(LastIndex != STRING_INVALID_INDEX && LastIndex != STRING_INVALID_INDEX);
	const wchar_t* At = Str.Ptr + FirstIndex;
	return WString(At, LastIndex - FirstIndex);
}

function usize WString_Find_First_Char(wstring String, wchar_t Character) {
	for (usize Index = 0; Index < String.Size; Index++) {
		if (String.Ptr[Index] == Character) {
			return Index;
		}
	}
	return STRING_INVALID_INDEX;
}

function b32 WString_Begins_With(wstring String, wstring Substr) {
	if(String.Size < Substr.Size) return false;
    for(usize i = 0; i < Substr.Size; i++) {
        if(String.Ptr[i] != Substr.Ptr[i]) {
            return false;
        }
    }

    return true;
}

function usize WString_Find_First(wstring String, wstring Pattern) {
    usize StopOffset = Max(String.Size+1, Pattern.Size)-Pattern.Size;
    usize PStart = 0;
    usize PEnd = StopOffset;

    if(Pattern.Size > 0) {
        wchar_t FirstChar = Pattern.Ptr[0];
        wstring SubstringTail = WString_Substr(Pattern, 1, Pattern.Size);
        for(; PStart < PEnd; PStart++) {
            wchar_t HaystackChar = String.Ptr[PStart];
            if(HaystackChar == FirstChar) {
                if(WString_Begins_With(WString_Substr(String, PStart+1, String.Size), 
                                       SubstringTail)) {
                    break;
                }
            }
        }
    }

    usize Result = STRING_INVALID_INDEX;
    if(PStart < PEnd) {
        Result = PStart;
    }
    return Result;
}

#define String_Lit(str) String(str, sizeof(str)-1)
#define WString_Lit(str) WString(L##str, (sizeof(L##str) / 2)-1)
#define WString_Null_Term(str) WString(str, WString_Length(str))

function utf8_reader UTF8_Reader_Begin(const char* Str, usize Size) {
	utf8_reader Result = {
		.Start = Str,
		.At	 = Str,
		.End = Str+Size
	};
	return Result;
}

function b32 UTF8_Reader_Is_Valid(utf8_reader* Reader) {
	return Reader->At < Reader->End;
}

function u32 UTF8_Reader_Next(utf8_reader* Reader) {
	Assert(UTF8_Reader_Is_Valid(Reader));
	u32 Length;
	u32 Result = UTF8_Read(Reader->At, &Length);
	Reader->At += Length;
	return Result;
}

function vec4 V4(f32 x, f32 y, f32 z, f32 w) {
	vec4 Result = { .x = x, .y = y, .z = z, .w = w };
	return Result;
}

function vec4 Green4_With_Alpha(f32 Alpha) {
	vec4 Result = V4(0.0f, 1.0f, 0.0f, Alpha);
	return Result;
}

function vec4 Green4() {
	vec4 Result = Green4_With_Alpha(1.0f);
	return Result;
}

function vec4 White4_With_Alpha(f32 Alpha) {
	vec4 Result = V4(1.0f, 1.0f, 1.0f, Alpha);
	return Result;
}

function vec4 White4() {
	vec4 Result = White4_With_Alpha(1.0f);
	return Result;
}

function vec4 Black4_With_Alpha(f32 Alpha) {
	vec4 Result = V4(0.0f, 0.0f, 0.0f, Alpha);
	return Result;
}

function vec4 Black4() {
	vec4 Result = Black4_With_Alpha(1.0f);
	return Result;
}