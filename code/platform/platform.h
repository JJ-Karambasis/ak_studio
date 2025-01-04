#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct tls tls;
typedef struct platform platform;

#define PLATFORM_RESERVE_MEMORY_DEFINE(name)  void* name(size_t Size)
#define PLATFORM_COMMIT_MEMORY_DEFINE(name)   void* name(void* Address, size_t Size)
#define PLATFORM_DECOMMIT_MEMORY_DEFINE(name) void  name(void* Address, size_t Size)
#define PLATFORM_RELEASE_MEMORY_DEFINE(name)  void  name(void* Address, size_t Size)

typedef PLATFORM_RESERVE_MEMORY_DEFINE(platform_reserve_memory_func);
typedef PLATFORM_COMMIT_MEMORY_DEFINE(platform_commit_memory_func);
typedef PLATFORM_DECOMMIT_MEMORY_DEFINE(platform_decommit_memory_func);
typedef PLATFORM_RELEASE_MEMORY_DEFINE(platform_release_memory_func);

#define PLATFORM_ALLOCATE_MEMORY_DEFINE(name) void* name(size_t Size)
#define PLATFORM_FREE_MEMORY_DEFINE(name) void name(void* Memory)

typedef PLATFORM_ALLOCATE_MEMORY_DEFINE(platform_allocate_memory_func);
typedef PLATFORM_FREE_MEMORY_DEFINE(platform_free_memory_func);

#define PLATFORM_TLS_CREATE_DEFINE(name) tls* name()
#define PLATFORM_TLS_GET_DEFINE(name) void* name(tls* TLS)
#define PLATFORM_TLS_SET_DEFINE(name) void name(tls* TLS, void* Data)
#define PLATFORM_TLS_DELETE_DEFINE(name) void name(tls* TLS)

typedef PLATFORM_TLS_CREATE_DEFINE(platform_tls_create_func);
typedef PLATFORM_TLS_GET_DEFINE(platform_tls_get_func);
typedef PLATFORM_TLS_SET_DEFINE(platform_tls_set_func);
typedef PLATFORM_TLS_DELETE_DEFINE(platform_tls_delete_func);


typedef struct {
	platform_reserve_memory_func* ReserveMemoryFunc;
	platform_commit_memory_func* CommitMemoryFunc;
	platform_decommit_memory_func* DecommitMemoryFunc;
	platform_release_memory_func* ReleaseMemoryFunc;
	
	platform_allocate_memory_func* AllocateMemoryFunc;
	platform_free_memory_func* FreeMemoryFunc;

	platform_tls_create_func* TLSCreateFunc;
	platform_tls_get_func* TLSGetFunc;
	platform_tls_set_func* TLSSetFunc;
	platform_tls_delete_func* TLSDeleteFunc;
} platform_vtable;

struct platform {
	platform_vtable* VTable;
	size_t 			 PageSize;
	tls* 			 ThreadContextTLS;
	struct 			 gdi* GDI;
};

#endif