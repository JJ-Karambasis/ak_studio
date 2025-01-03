#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdbool.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef float f32;
typedef double f64;

#define DEFAULT_ALIGNMENT 16

#define function static
#define global static

#include <assert.h>
#define Assert(c) assert(c)

#define Offset_Pointer(ptr, offset) (void*)((u8*)(ptr) + (ptrdiff_t)(offset))
#define Array_Count(arr) sizeof(arr) / sizeof((arr)[0])
#define Sq(x) ((x)*(x))

#define KB(x) (x*1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

#define Is_Pow2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define Align_Pow2(x, a) (((x) + (a)-1) & ~((a)-1))

#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Clamp(min, v, max) Min(Max(min, v), max)
#define Saturate(v) Clamp(0, v, 1)

#define SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define SLL_Push_Front(First, Node) SLL_Push_Front_N(First, Node, Next)
#define SLL_Pop_Front(First) SLL_Pop_Front_N(First, Next)


#define SLL_Pop_Front_N(First, Next) ((First) = (First)->Next)
#define SLL_Push_Front_N(First, Node, Next) (Node->Next = First, First = Node)

#define DLL_Remove_Front_NP(First, Last, Next, Prev) \
do { \
if(First == Last) \
First = Last = NULL; \
else { \
First = First->Next; \
First->Prev = NULL; \
} \
} while(0)

#define DLL_Remove_Back_NP(First, Last, Next, Prev) \
do { \
if(First == Last) \
First = Last = NULL; \
else { \
Last = Last->Prev; \
Last->Next = NULL; \
} \
} while(0)

#define DLL_Push_Front_NP(First, Last, Node, Next, Prev) (!(First) ? ((First) = (Last) = (Node)) : ((Node)->Next = (First), (First)->Prev = (Node), (First) = (Node)))
#define DLL_Push_Back_NP(First, Last, Node, Next, Prev) (!(First) ? ((First) = (Last) = (Node)) : ((Node)->Prev = (Last), (Last)->Next = (Node), (Last) = (Node)))
#define DLL_Remove_NP(First, Last, Node, Next, Prev) \
do { \
if(First == Node) { \
First = First->Next; \
if(First) First->Prev = NULL; \
} \
if(Last == Node) { \
Last = Last->Prev; \
if(Last) Last->Next = NULL; \
} \
if(Node->Prev) Node->Prev->Next = Node->Next; \
if(Node->Next) Node->Next->Prev = Node->Prev; \
Node->Prev = NULL; \
Node->Next = NULL; \
} while(0)

#define DLL_Insert_After_NP(First, Last, Target, Node, Next, Prev) \
do { \
if(Target->Next) { \
Target->Next->Prev = Node; \
Node->Next = Target->Next; \
} \
else { \
Assert(Target == Last, "Invalid"); \
Last = Node; \
} \
Node->Prev = Target; \
Target->Next = Node; \
} while(0)

#define DLL_Insert_Prev_NP(First, Last, Target, Node, Next, Prev) \
do { \
if(Target->Prev) { \
Target->Prev->Next = Node; \
Node->Prev = Target->Prev; \
} \
else { \
Assert(Target == First, "Invalid"); \
First = Node; \
} \
Node->Next = Target; \
Target->Prev = Node; \
} while(0)

#define DLL_Remove_Back(First, Last) DLL_Remove_Back_NP(First, Last, Next, Prev)
#define DLL_Remove_Front(First, Last) DLL_Remove_Front_NP(First, Last, Next, Prev)
#define DLL_Push_Front(First, Last, Node) DLL_Push_Front_NP(First, Last, Node, Next, Prev)
#define DLL_Push_Back(First, Last, Node) DLL_Push_Back_NP(First, Last, Node, Next, Prev)
#define DLL_Remove(First, Last, Node) DLL_Remove_NP(First, Last, Node, Next, Prev)
#define DLL_Push_Front_Only(First, Node) DLL_Push_Front_Only_NP(First, Node, Next, Prev)

typedef struct allocator allocator;

#define ALLOCATOR_ALLOCATE_MEMORY_DEFINE(name) void* name(allocator* Allocator, size_t Size)
#define ALLOCATOR_FREE_MEMORY_DEFINE(name) void name(allocator* Allocator, void* Memory)

typedef ALLOCATOR_ALLOCATE_MEMORY_DEFINE(allocator_allocate_memory_func);
typedef ALLOCATOR_FREE_MEMORY_DEFINE(allocator_free_memory_func);

typedef struct {
	allocator_allocate_memory_func* AllocateMemoryFunc;
	allocator_free_memory_func*     FreeMemoryFunc;
} allocator_vtable;

struct allocator {
	allocator_vtable* VTable;
};

#define Allocator_Allocate_Memory(allocator, size) (allocator)->VTable->AllocateMemoryFunc(allocator, size)
#define Allocator_Free_Memory(allocator, memory) (allocator)->VTable->FreeMemoryFunc(allocator, memory)

typedef struct {
	allocator Base;
	u8*       Memory;
	size_t    ReserveSize;
	size_t    CommitSize;
	size_t 	  Used;
} arena;

typedef struct {
	arena* Arena;
	size_t Offset;
} arena_marker;

#define THREAD_CONTEXT_MAX_ARENA_COUNT 32
typedef struct {
	arena* 		 ScratchArenas[THREAD_CONTEXT_MAX_ARENA_COUNT];
	arena_marker ScratchMarkers[THREAD_CONTEXT_MAX_ARENA_COUNT];
	size_t 		 ScratchIndex;
} thread_context;

typedef struct {
	s32 x;
	s32 y;
} v2i;

#include <platform/platform.h>

#endif