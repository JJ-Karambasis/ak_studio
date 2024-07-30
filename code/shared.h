#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <math.h>
#include <float.h>

#if _WIN64
#define BITNESS_64
#else
#define BITNESS_32
#endif

#include <stdint.h>
#include <math.h>
#include <float.h>

#include "ak_atomic.h"
#include "stb_sprintf.h"

#define function static
#define local static
#define global static

#if _WIN64
#define BITNESS_64
#else
#define BITNESS_32
#endif

#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Clamp(min, v, max) Min(Max(min, v), max)

#ifdef DEBUG_BUILD
#include <assert.h>
#define Assert(c) assert(c)
#else
#define Assert(c)
#endif

#define Not_Implemented() Assert(!"Not Implemented")

#define Static_Assert(c) static_assert((c), "(" #c ") failed")

#define false 0
#define true  1

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef float  f32;
typedef double f64;

#ifdef BITNESS_32
typedef u32 usize;
#define PTR_SIZE 4
#else
typedef u64 usize;
#define PTR_SIZE 8
#endif

#define DEFAULT_ALIGNMENT (PTR_SIZE*2)

#define Array_Count(arr) sizeof(arr) / sizeof((arr)[0])
#define Sq(x) ((x)*(x))

#define KB(x) (x*1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

#define Is_Pow2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define Align_Pow2(x, a) (((x) + (a)-1) & ~((a)-1))
#define Abs(v) (((v) < 0) ? -(v) : (v))

//Generic link list macros
#define SLL_Pop_Front_N(First, Next) (First = First->Next)
#define SLL_Push_Front_N(First, Node, Next) (Node->Next = First, First = Node)

#define Read_Bit(S,N) ((S) & (0x1ul << (N)))

#define SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define SLL_Push_Front(First, Node) SLL_Push_Front_N(First, Node, Next)
#define SLL_Pop_Front(First) SLL_Pop_Front_N(First, Next)

#define SLL_Push_Front_Async(First, Node) \
do { \
auto OldTop = AK_Atomic_Load_Ptr_Relaxed(First); \
Node->Next = (decltype(Node))OldTop; \
if(AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(First, OldTop, Node, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) { \
break; \
} \
} while(1)

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

typedef struct heap_block_node heap_block_node;
typedef struct heap_block heap_block;

struct heap_block {
	usize              Offset;
	usize              Size;
	heap_block_node*   Node;
	heap_block*        Prev;
	heap_block*        Next;
#ifdef DEBUG_BUILD
	heap_block* NextAllocated;
	heap_block* PrevAllocated;
#endif
};

struct heap_block_node {
	heap_block*      Block;
	usize            Color;
	heap_block_node* LeftChild;
	heap_block_node* RightChild;
};

typedef struct {
	usize 			 NodeCapacity;
	usize 			 NodeCount;
	heap_block_node* Nodes;
	heap_block_node* Root;
	heap_block_node* FreeNodes;
} heap_block_tree;

typedef struct {
	u8*   			BaseAddress;
	usize 			PageSize;
	heap_block_tree FreeBlockTree;

	#ifdef DEBUG_BUILD
	heap_block* AllocatedList;
	#endif
} heap;

typedef struct {
	u8*   BaseAddress;
	usize ReserveSize;
	usize CommitSize;
	usize PageSize;
	usize Used;
} arena;

typedef struct {
	arena* Arena;
	usize  Used;
} arena_marker;

typedef struct {
	arena* Arena;
} scratch;

#define MAX_SCRATCH_COUNT 32
typedef struct {
	u32 		 ScratchIndex;
	arena* 		 ScratchArenas[MAX_SCRATCH_COUNT];
	arena_marker ScratchMarkers[MAX_SCRATCH_COUNT];
} thread_context;

typedef struct {
	u8*   Ptr;
	usize Size;
} buffer;

typedef struct {
	const char* Ptr;
	usize 	    Size;
} string;

typedef struct {
	const wchar_t* Ptr;
	usize 		   Size;
} wstring;

typedef struct {
	union {
		s32 Data[2];
		struct {
			s32 x, y;
		};
		struct {
			s32 w, h;
		};
	};
} vec2i;

typedef struct {
	union {
		f32 Data[2];
		struct {
			f32 x, y;
		};
		struct {
			f32 w, h;
		};
	};
} vec2;

typedef struct {
	union {
		f32 Data[4];
		struct {
			f32 x, y, z, w;
		};
	};
} vec4; 

#endif