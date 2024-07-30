#ifndef APP_H
#define APP_H

#include "shared.h"
#include "gdi.h"
#include "font.h"

typedef void platform_log(const char* Format, ...);
typedef void* platform_reserve_memory(usize ReserveSize);
typedef void* platform_commit_memory(void* BaseCommit, usize DeltaSize);
typedef void platform_decommit_memory(void* BaseCommit, usize DeltaSize);
typedef buffer platform_read_entire_file(arena* Arena, string Path);

typedef struct {
	platform_log* PlatformLog;
	platform_reserve_memory* ReserveMemory;
	platform_commit_memory* CommitMemory;
	platform_decommit_memory* DecommitMemory;
	platform_read_entire_file* ReadEntireFile;
	ak_tls 		  ThreadContextTLS;
	gdi*          GDI;
	ak_mutex 	  HeapLock;
	heap 		  Heap;
} platform;

global platform* G_Platform;
#define Log(format, ...) G_Platform->PlatformLog(format, __VA_ARGS__)
#define Reserve_Memory(size) G_Platform->ReserveMemory(size)
#define Commit_Memory(base_commit, size) G_Platform->CommitMemory(base_commit, size)
#define Decommit_Memory(base_commit, size) G_Platform->DecommitMemory(base_commit, size)
#define Read_Entire_File(arena, path) G_Platform->ReadEntireFile(arena, path)

typedef struct {
	b32    Initialized;
	arena* Arena;
	font*  Font;
} app;

#define APP_UPDATE_AND_RENDER(name) void name(app* App, platform* Platform)
typedef APP_UPDATE_AND_RENDER(app_update_and_render);

#endif