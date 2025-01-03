#include <rpmalloc.h>

function void* RPMalloc_Memory_Map(size_t Size, size_t Alignment, size_t* Offset, size_t* MappedSize) {
	size_t ReserveSize = Size + Alignment;
	void* Memory = Reserve_Memory(ReserveSize);
	if (Memory) {
		if (Alignment) {
			size_t Padding = ((uintptr_t)Memory & (uintptr_t)(Alignment - 1));
			if (Padding) Padding = Alignment - Padding;
			Memory = Offset_Pointer(Memory, Padding);
			*Offset = Padding;
		}
		*MappedSize = ReserveSize;
	}

	return Memory;
}

function void RPMalloc_Memory_Commit(void* Address, size_t Size) {
	Commit_Memory(Address, Size);
}

function void RPMalloc_Memory_Decommit(void* Address, size_t Size) {
	Decommit_Memory(Address, Size);
}

function void RPMalloc_Memory_Unmap(void* Address, size_t Offset, size_t MappedSize) {
	Address = Offset_Pointer(Address, -(intptr_t)Offset);
	Release_Memory(Address, MappedSize);
}

global rpmalloc_interface_t Memory_VTable = {
	.memory_map = RPMalloc_Memory_Map,
	.memory_commit = RPMalloc_Memory_Commit,
	.memory_decommit = RPMalloc_Memory_Decommit,
	.memory_unmap = RPMalloc_Memory_Unmap
};

function PLATFORM_ALLOCATE_MEMORY_DEFINE(Platform_Allocate_Memory) {
	return rpzalloc(Size);
}

function PLATFORM_FREE_MEMORY_DEFINE(Platform_Free_Memory) {
	rpfree(Memory);
}

function void Platform_Initialize_Memory() {
	rpmalloc_initialize(&Memory_VTable);
}

#include <rpmalloc.c>