global platform* G_Platform;

#define Get_Page_Size() G_Platform->PageSize
#define Reserve_Memory(size) G_Platform->VTable->ReserveMemoryFunc(size)
#define Commit_Memory(address, size) G_Platform->VTable->CommitMemoryFunc(address, size)
#define Decommit_Memory(address, size) G_Platform->VTable->DecommitMemoryFunc(address, size)
#define Release_Memory(address, size) G_Platform->VTable->ReleaseMemoryFunc(address, size)

#define Allocate_Memory(size) G_Platform->VTable->AllocateMemoryFunc(size)
#define Free_Memory(memory) G_Platform->VTable->FreeMemoryFunc(memory)

#define Allocate_Struct(type) (type *)Allocate_Memory(sizeof(type))

#define TLS_Create() G_Platform->VTable->TLSCreateFunc()
#define TLS_Get(tls) G_Platform->VTable->TLSGetFunc(tls)
#define TLS_Set(tls, data) G_Platform->VTable->TLSSetFunc(tls, data)
#define TLS_Delete(tls) G_Platform->VTable->TLSDeleteFunc(tls)

#define Is_Pow2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

function void Memory_Clear(void* Memory, size_t Size) {
	memset(Memory, 0, Size);
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

function size_t Ceil_Pow2(size_t Size) {
#ifdef ENV32
	return (size_t)Ceil_Pow2_U32((u32)Size);
#else
	return (size_t)Ceil_Pow2_U64((u64)Size);
#endif
}

function u64 Align_U64(u64 Value, u64 Alignment) {
    u64 Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

function u32 Align_U32(u32 Value, u32 Alignment) {
    u32 Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

function size_t Align(size_t Value, size_t Alignment) {
    size_t Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

function platform* Platform_Get() {
	return G_Platform;
}

function arena* Arena_Create_With_Reserve_Size(size_t ReserveSize) {
	size_t PageSize = Get_Page_Size();
	Assert(Is_Pow2(PageSize));
	ReserveSize = Align_Pow2(ReserveSize, PageSize);
	void* Memory = Reserve_Memory(ReserveSize);
	if (!Memory) {
		Assert(!"Failed to reserve arena memory!");
		return NULL;
	}

	arena* Arena = (arena*)Commit_Memory(Memory, PageSize);
	Arena->Memory = Memory;
	Arena->ReserveSize = ReserveSize;
	Arena->CommitSize = PageSize;
	Arena->Used = sizeof(arena);
	return Arena;
}

function arena* Arena_Create() {
	return Arena_Create_With_Reserve_Size(GB(1));
}

function void Arena_Delete(arena* Arena) {
	if (Arena) {
		void* Memory = Arena->Memory;
		size_t ReserveSize = Arena->ReserveSize;
		Memory_Clear(Arena, sizeof(arena));
		Release_Memory(Memory, ReserveSize);
	}
}

function void* Arena_Push_Aligned_No_Clear(arena* Arena, size_t Size, size_t Alignment) {
	Assert(Is_Pow2(Alignment));
	Arena->Used = Align_Pow2(Arena->Used, Alignment);

	Assert(Arena->Used + Size <= Arena->ReserveSize);

	if (Arena->Used + Size > Arena->CommitSize) {
		size_t PageSize = Get_Page_Size();

		size_t NewCommitSize = Max(Arena->Used + Size, Arena->CommitSize + PageSize);
		NewCommitSize = Align_Pow2(NewCommitSize, PageSize);
		size_t CommitDiff = NewCommitSize - Arena->CommitSize;
		Assert((CommitDiff % PageSize) == 0);

		if (!Commit_Memory(Arena->Memory + Arena->CommitSize, CommitDiff)) {
			Assert(!"Failed to commit arena memory!");
			return NULL;
		}

		Arena->CommitSize = NewCommitSize;
	}

	Assert(Arena->CommitSize <= Arena->ReserveSize);
	void* Memory = Arena->Memory + Arena->Used;
	Arena->Used += Size;
	return Memory;
}

function void* Arena_Push_No_Clear(arena* Arena, size_t Size) {
	return Arena_Push_Aligned_No_Clear(Arena, Size, DEFAULT_ALIGNMENT);
}

#define Arena_Push_Struct_No_Clear(arena, type) (type*)Arena_Push_No_Clear(arena, sizeof(type))

function arena_marker Arena_Get_Marker(arena* Arena) {
	arena_marker Result;
	Result.Arena = Arena;
	Result.Offset = Arena->Used;
	return Result;
}

function void Arena_Set_Marker(arena* Arena, arena_marker Marker) {
	Assert(Arena == Marker.Arena);
	Arena->Used = Marker.Offset;
}

function void Arena_Clear(arena* Arena) {
	Arena->Used = sizeof(arena);
}

function thread_context* Thread_Context_Get() {
	platform* Platform = Platform_Get();
	thread_context* ThreadContext = TLS_Get(Platform->ThreadContextTLS);
	if (!ThreadContext) {
		ThreadContext = Allocate_Struct(thread_context);
		TLS_Set(Platform->ThreadContextTLS, ThreadContext);
	}
	return ThreadContext;
}

function arena* Scratch_Get() {
	thread_context* ThreadContext = Thread_Context_Get();
	
	Assert(ThreadContext->ScratchIndex < THREAD_CONTEXT_MAX_ARENA_COUNT);
	u32 ScratchIndex = ThreadContext->ScratchIndex++;
	arena* Arena = ThreadContext->ScratchArenas[ScratchIndex];
	if (!Arena) {
		ThreadContext->ScratchArenas[ScratchIndex] = Arena_Create();
		Arena = ThreadContext->ScratchArenas[ScratchIndex];
	}
	ThreadContext->ScratchMarkers[ScratchIndex] = Arena_Get_Marker(Arena);
	return Arena;
}

function v2i V2i(s32 x, s32 y) {
	v2i Result = { x, y };
	return Result;
}