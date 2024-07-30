#ifndef AK_ATOMIC_H
#define AK_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#   define AK_ATOMIC_WIN32_OS
#   define AK_ATOMIC_MSVC_COMPILER
#   if defined(_M_X64)
#       define AK_ATOMIC_PTR_SIZE 8
#   elif defined(_M_IX86)
#       define AK_ATOMIC_PTR_SIZE 4
#   else
#       error "Unrecognized platform!"
#   endif
#elif defined(__GNUC__)
#   define AK_ATOMIC_GCC_COMPILER
#   define AK_ATOMIC_POSIX_OS

#   if defined(__APPLE__)
#       define AK_ATOMIC_OSX_OS
#   endif

#   if defined(__arm__)
#       define AK_ATOMIC_ARM_CPU
#       define AK_ATOMIC_PTR_SIZE 4
#       if defined(__thumb__)
            #define AK_ATOMIC_ARM_THUMB 1
#       endif
#   elif defined(__aarch64__)
#       define AK_ATOMIC_AARCH64_CPU
#       define AK_ATOMIC_PTR_SIZE 8
#   elif defined(__x86_64__)
#       define AK_ATOMIC_X86_64_CPU
#       define AK_ATOMIC_PTR_SIZE 8   
#   elif defined(__i386__)
#       define AK_ATOMIC_X86_64_CPU
#       define AK_ATOMIC_PTR_SIZE 4    
#   else
#   error "Not Implemented"
#   endif
#endif

#ifndef AKATOMICDEF
# ifdef AK_ATOMIC_STATIC
# define AKATOMICDEF static
# else
# define AKATOMICDEF extern
# endif
#endif

#define AK_ATOMIC__STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
#define AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L) AK_ATOMIC__STATIC_ASSERT(X,static_assertion_at_line_##L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT2(X,L) AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT(X)    AK_ATOMIC__COMPILE_TIME_ASSERT2(X,__LINE__)

#if defined(AK_ATOMIC_POSIX_OS)
/*c89 posix won't seem to recognized the time functions
  without this (nanosleep clock_getttime) */
#define _POSIX_C_SOURCE 199309L
#define _DARWIN_C_SOURCE
#endif

/*
If stdint is not included for your platform, define AK_ATOMIC_CUSTOM_TYPES
and define the stdint types:
-uint32_t -> 32 bit unsigned integer
-int32_t -> 32 bit signed integer
-uint64_t -> 64 bit unsigned integer
-int64_t -> 64 bit signed integer
-bool -> 8 bit signed integer
*/

#ifndef AK_ATOMIC_CUSTOM_TYPES
#include <stdint.h>
#include <stdbool.h>
#endif


#if defined(AK_ATOMIC_MSVC_COMPILER)

#include <intrin.h>

typedef struct ak_atomic_u32 {
	uint32_t Nonatomic;
} ak_atomic_u32;

typedef struct ak_atomic_u64 {
	uint64_t Nonatomic;
} ak_atomic_u64;

typedef struct ak_atomic_ptr {
	void* Nonatomic;
} ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() _ReadBarrier()
#define AK_Atomic_Compiler_Fence_Rel() _WriteBarrier()
#define AK_Atomic_Compiler_Fence_Seq_Cst() _ReadWriteBarrier()

#define AK_Atomic_Thread_Fence_Acq() _ReadBarrier()
#define AK_Atomic_Thread_Fence_Rel() _WriteBarrier()
#define AK_Atomic_Thread_Fence_Seq_Cst() MemoryBarrier()

#elif defined(AK_ATOMIC_GCC_COMPILER) && (defined(AK_ATOMIC_AARCH64_CPU) || defined(AK_ATOMIC_ARM_CPU))

/*Atomic operators on this architecture need to be aligned properly*/

typedef struct ak_atomic_u32 {
	volatile uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct ak_atomic_u64 {
	volatile uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct ak_atomic_ptr {
	void* volatile Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Rel() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Seq_Cst() __asm__ volatile("" ::: "memory")

#define AK_Atomic_Thread_Fence_Acq() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Rel() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("dmb ish" ::: "memory")

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_X86_64_CPU)

typedef struct ak_atomic_u32 {
	volatile uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct ak_atomic_u64 {
	volatile uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct ak_atomic_ptr {
	volatile void* Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Rel() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Seq_Cst() __asm__ volatile("" ::: "memory")

#define AK_Atomic_Thread_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Thread_Fence_Rel() __asm__ volatile("" ::: "memory")

#if AK_ATOMIC_PTR_SIZE == 8
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%rsp)" ::: "memory")
#else
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%esp)" ::: "memory")
#endif

#else
#error "Not Implemented"
#endif

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u32) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u64) == 8);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_ptr) == AK_ATOMIC_PTR_SIZE);

/*Compiler specific functions (all other atomics are built ontop of these)*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object);
AKATOMICDEF void     AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object);
AKATOMICDEF void      AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object);

/*Ptr type (is either 32 bit or 64 bit wrappers)*/
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object);
AKATOMICDEF void  AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue);

/*Compare exchange for boolean results*/
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);

typedef enum ak_atomic_memory_order {
    AK_ATOMIC_MEMORY_ORDER_RELAXED,
    AK_ATOMIC_MEMORY_ORDER_ACQUIRE,
    AK_ATOMIC_MEMORY_ORDER_RELEASE,
    AK_ATOMIC_MEMORY_ORDER_ACQ_REL
} ak_atomic_memory_order;

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void      AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF void* AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void  AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);


/*Thread primitives*/
typedef struct ak_thread ak_thread;
#define AK_THREAD_CALLBACK_DEFINE(name) int32_t name(ak_thread* Thread, void* UserData)
typedef AK_THREAD_CALLBACK_DEFINE(ak_thread_callback_func);

#define AK_CONDITION_VARIABLE_PREDICATE_DEFINE(name) bool name(void* UserData)
typedef AK_CONDITION_VARIABLE_PREDICATE_DEFINE(ak_condition_variable_predicate_func);

#if defined(AK_ATOMIC_WIN32_OS)
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

struct ak_thread {
    HANDLE                   Handle;
    ak_thread_callback_func* Callback;
    void*                    UserData;
};

typedef struct ak_mutex {
    CRITICAL_SECTION CriticalSection;
} ak_mutex;

typedef struct ak_semaphore {
    HANDLE Handle;
} ak_semaphore;

typedef struct ak_condition_variable {
    CONDITION_VARIABLE Variable;
} ak_condition_variable;

typedef struct ak_tls {
    DWORD Index;
} ak_tls;

#elif defined(AK_ATOMIC_POSIX_OS)
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct ak_thread {
    pthread_t                Thread;
    ak_thread_callback_func* Callback;
    void*                    UserData;
};

typedef struct ak_mutex {
    pthread_mutex_t Mutex;
} ak_mutex;

#ifdef AK_ATOMIC_OSX_OS
#include <mach/mach.h>

typedef struct ak_semaphore {
    semaphore_t Semaphore;
} ak_semaphore;

#else
typedef struct ak_semaphore {
    sem_t Semaphore;
} ak_semaphore;
#endif

typedef struct ak_condition_variable {
    pthread_cond_t Variable;
} ak_condition_variable;

typedef struct ak_tls {
    pthread_key_t Key;
} ak_tls;

#else
#error "Not Implemented"
#endif

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void);
AKATOMICDEF void     AK_Sleep(uint32_t Milliseconds);
AKATOMICDEF uint64_t AK_Query_Performance_Counter(void);
AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void);

AKATOMICDEF bool     AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData);
AKATOMICDEF void     AK_Thread_Delete(ak_thread* Thread);
AKATOMICDEF void     AK_Thread_Wait(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void);

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex);
AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex);

AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount);
AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend);

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex);
AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable);

AKATOMICDEF bool  AK_TLS_Create(ak_tls* TLS);
AKATOMICDEF void  AK_TLS_Delete(ak_tls* TLS);
AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS);
AKATOMICDEF void  AK_TLS_Set(ak_tls* TLS, void* Data);

typedef struct ak_lw_semaphore {
    ak_semaphore  InternalSem;
    ak_atomic_u32 Count;
    uint32_t      MaxSpinCount;
} ak_lw_semaphore;

AKATOMICDEF bool AK_LW_Semaphore_Create_With_Spin_Count(ak_lw_semaphore* Semaphore, int32_t InitialCount, uint32_t SpinCount);
AKATOMICDEF bool AK_LW_Semaphore_Create(ak_lw_semaphore* Semaphore, int32_t InitialCount);
AKATOMICDEF void AK_LW_Semaphore_Delete(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Increment(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Decrement(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Add(ak_lw_semaphore* Semaphore, int32_t Addend);

typedef struct ak_event {
    ak_mutex              Mutex;
    ak_condition_variable CondVar;
    uint32_t              State;
    uint32_t              Padding;
} ak_event;

AKATOMICDEF bool AK_Event_Create(ak_event* Event);
AKATOMICDEF void AK_Event_Delete(ak_event* Event);
AKATOMICDEF void AK_Event_Signal(ak_event* Event);
AKATOMICDEF void AK_Event_Wait(ak_event* Event);
AKATOMICDEF void AK_Event_Reset(ak_event* Event);

typedef struct ak_auto_reset_event {
    /*
     1- Signaled
     0- Reset and no threads are waiting
    -N- Reset and N threads are waiting
    */
    ak_lw_semaphore Semaphore;
    ak_atomic_u32   Status; 
    uint32_t        Unused__Padding;
} ak_auto_reset_event;

AKATOMICDEF bool AK_Auto_Reset_Event_Create(ak_auto_reset_event* Event, int32_t InitialStatus);
AKATOMICDEF void AK_Auto_Reset_Event_Delete(ak_auto_reset_event* Event);
AKATOMICDEF void AK_Auto_Reset_Event_Signal(ak_auto_reset_event* Event);
AKATOMICDEF void AK_Auto_Reset_Event_Wait(ak_auto_reset_event* Event);

typedef struct ak_rw_lock {
    ak_lw_semaphore ReadSemaphore;
    ak_lw_semaphore WriteSemaphore;
    ak_atomic_u32   Status;
    uint32_t        Unused__Padding;
} ak_rw_lock;

AKATOMICDEF bool AK_RW_Lock_Create(ak_rw_lock* Lock);
AKATOMICDEF void AK_RW_Lock_Delete(ak_rw_lock* Lock);
AKATOMICDEF void AK_RW_Lock_Reader(ak_rw_lock* Lock);
AKATOMICDEF void AK_RW_Unlock_Reader(ak_rw_lock* Lock);
AKATOMICDEF void AK_RW_Lock_Writer(ak_rw_lock* Lock);
AKATOMICDEF void AK_RW_Unlock_Writer(ak_rw_lock* Lock);

#ifndef AK_DISABLE_ASYNC_STACK_INDEX

#define AK_ASYNC_STACK_INDEX32_INVALID ((uint32_t)-1) 

typedef struct {
    void*         AllocUserData;
    uint32_t*     NextIndices;
    ak_atomic_u64 Head;
    uint32_t      Capacity;
    uint32_t      Unused__Padding;
} ak_async_stack_index32;

AKATOMICDEF void     AK_Async_Stack_Index32_Init_Raw(ak_async_stack_index32* StackIndex, uint32_t* IndicesPtr, uint32_t Capacity);
AKATOMICDEF void     AK_Async_Stack_Index32_Push_Sync(ak_async_stack_index32* StackIndex, uint32_t Index);
AKATOMICDEF void     AK_Async_Stack_Index32_Push(ak_async_stack_index32* StackIndex, uint32_t Index);
AKATOMICDEF uint32_t AK_Async_Stack_Index32_Pop(ak_async_stack_index32* StackIndex);
AKATOMICDEF bool     AK_Async_Stack_Index32_Alloc(ak_async_stack_index32* StackIndex, uint32_t Capacity, void* AllocUserData);
AKATOMICDEF void     AK_Async_Stack_Index32_Free(ak_async_stack_index32* StackIndex);

#endif

#ifndef AK_DISABLE_ASYNC_SPMC_QUEUE_INDEX

#define AK_ASYNC_SPMC_QUEUE_INDEX32_INVALID ((uint32_t)-1)

typedef struct ak_async_spmc_queue_index32 {
    void*         AllocUserData;
    ak_atomic_u32 BottomIndex;
    ak_atomic_u32 TopIndex;
    uint32_t*     Indices;
    uint32_t      Capacity;
    uint32_t      Unused__Padding0;
} ak_async_spmc_queue_index32;

AKATOMICDEF void     AK_Async_SPMC_Queue_Index32_Init_Raw(ak_async_spmc_queue_index32* QueueIndex, uint32_t* IndicesPtr, uint32_t Capacity);
AKATOMICDEF void     AK_Async_SPMC_Queue_Index32_Enqueue(ak_async_spmc_queue_index32* QueueIndex, uint32_t Index);
AKATOMICDEF uint32_t AK_Async_SPMC_Queue_Index32_Dequeue(ak_async_spmc_queue_index32* QueueIndex);
AKATOMICDEF bool     AK_Async_SPMC_Queue_Index32_Alloc(ak_async_spmc_queue_index32* QueueIndex, uint32_t Capacity, void* AllocUserData);
AKATOMICDEF void     AK_Async_SPMC_Queue_Index32_Free(ak_async_spmc_queue_index32* QueueIndex);

#endif

#ifndef AK_DISABLE_ASYNC_SLOT_MAP

typedef uint64_t ak_slot64;

typedef union {
    ak_slot64 Slot;
    struct {
        uint32_t      Index;
        ak_atomic_u32 Generation;
    } Data;
} ak_slot64__internal;

typedef struct {
    ak_async_stack_index32 FreeIndices;
    ak_slot64__internal*   Slots; /*Array size is specified by FreeIndices.Capacity*/
    void*                  AllocUserData;
} ak_async_slot_map64;

AKATOMICDEF void      AK_Async_Slot_Map64_Init_Raw(ak_async_slot_map64* SlotMap, uint32_t* IndicesPtr, ak_slot64* SlotsPtr, uint32_t Capacity);
AKATOMICDEF bool      AK_Async_Slot_Map64_Is_Allocated(const ak_async_slot_map64* SlotMap, ak_slot64 Slot);
AKATOMICDEF ak_slot64 AK_Async_Slot_Map64_Alloc_Slot(ak_async_slot_map64* SlotMap);
AKATOMICDEF void      AK_Async_Slot_Map64_Free_Slot(ak_async_slot_map64* SlotMap, ak_slot64 Slot);
AKATOMICDEF bool      AK_Async_Slot_Map64_Alloc(ak_async_slot_map64* SlotMap, uint32_t Capacity, void* AllocUserData);
AKATOMICDEF void      AK_Async_Slot_Map64_Free(ak_async_slot_map64* SlotMap);

#define AK_Async_Slot_Map64_Capacity(slotmap) ((slotmap)->FreeIndices.Capacity)

#define AK_Slot64(index, key) ((uint64_t)(index) | (((uint64_t)(key) << 32)))
#define AK_Slot64_Index(slot) ((uint32_t)(slot))
#define AK_Slot64_Key(slot) ((uint32_t)(slot >> 32))

#endif

#ifndef AK_DISABLE_QSBR


typedef int16_t ak_qsbr_context;

#define AK_QSBR_CALLBACK_DEFINE(name) void name(void* UserData)
typedef AK_QSBR_CALLBACK_DEFINE(ak_qsbr_callback_func);

typedef struct ak_qsbr ak_qsbr;
ak_qsbr*        AK_QSBR_Create(void* MallocUserData);
void            AK_QSBR_Delete(ak_qsbr* QSBR);
ak_qsbr_context AK_QSBR_Create_Context(ak_qsbr* QSBR);
void            AK_QSBR_Delete_Context(ak_qsbr* QSBR, ak_qsbr_context Context);
void            AK_QSBR_Update(ak_qsbr* QSBR, ak_qsbr_context Context);
void            AK_QSBR_Enqueue(ak_qsbr* QSBR, ak_qsbr_callback_func* Callback, void* UserData, size_t UserDataSize);

#endif

#ifndef AK_DISABLE_JOB

#if !defined(AK_JOB_SYSTEM_FAST_USERDATA_SIZE)
#   if AK_ATOMIC_PTR_SIZE == 8
#       define AK_JOB_SYSTEM_FAST_USERDATA_SIZE 83
#   else
#       define AK_JOB_SYSTEM_FAST_USERDATA_SIZE 95
#   endif
#endif

typedef uint64_t ak_job_id;
typedef struct ak_job_system   ak_job_system;

#define AK_JOB_THREAD_BEGIN_DEFINE(name)  void name(ak_thread* Thread, void* UserData)
#define AK_JOB_THREAD_UPDATE_DEFINE(name) void name(ak_thread* Thread, void* UserData)
#define AK_JOB_THREAD_END_DEFINE(name)    void name(ak_thread* Thread, void* UserData)

typedef AK_JOB_THREAD_BEGIN_DEFINE(ak_job_thread_begin_func);
typedef AK_JOB_THREAD_UPDATE_DEFINE(ak_job_thread_update_func);
typedef AK_JOB_THREAD_END_DEFINE(ak_job_thread_end_func);

typedef enum ak_job_bit_flag {
    AK_JOB_FLAG_NONE,
    AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT = (1 << 0),
    AK_JOB_FLAG_FREE_WHEN_DONE_BIT = (1 << 1)
} ak_job_bit_flag;
typedef uint32_t ak_job_flags;

typedef struct ak_job_thread_callbacks {
    ak_job_thread_begin_func*  JobThreadBegin;
    ak_job_thread_update_func* JobThreadUpdate;
    ak_job_thread_end_func*    JobThreadEnd;
    void*                      UserData;
} ak_job_thread_callbacks;

#define AK_JOB_SYSTEM_CALLBACK_DEFINE(name)   void          name(ak_job_system* JobSystem, ak_job_id JobID, void* JobUserData)
typedef AK_JOB_SYSTEM_CALLBACK_DEFINE(ak_job_system_callback_func);

typedef struct ak_job_system_data {
    ak_job_system_callback_func* JobCallback;
    void*                        Data;
    size_t                       DataByteSize;
} ak_job_system_data;

AKATOMICDEF ak_job_system* AK_Job_System_Create(uint32_t MaxJobCount, uint32_t ThreadCount, uint32_t MaxDependencyCount, const ak_job_thread_callbacks* Callbacks, void* MallocUserData);
AKATOMICDEF void           AK_Job_System_Delete(ak_job_system* JobSystem);
AKATOMICDEF ak_job_id      AK_Job_System_Alloc_Job(ak_job_system* JobSystem, ak_job_system_data JobData, ak_job_id ParentID, ak_job_flags Flags);
AKATOMICDEF ak_job_id      AK_Job_System_Alloc_Empty_Job(ak_job_system* JobSystem, ak_job_flags Flags);
AKATOMICDEF void           AK_Job_System_Free_Job(ak_job_system* JobSystem, ak_job_id JobID);
AKATOMICDEF void           AK_Job_System_Add_Job(ak_job_system* JobSystem, ak_job_id JobID);
AKATOMICDEF void           AK_Job_System_Wait_For_Job(ak_job_system* JobSystem, ak_job_id JobID);
AKATOMICDEF void           AK_Job_System_Add_Dependency(ak_job_system* JobSystem, ak_job_id Job, ak_job_id DependencyJob);

#endif

#ifdef __cplusplus
}
#endif

#endif

#ifdef AK_ATOMIC_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#define AK_ATOMIC__UNREFERENCED_PARAMETER(param) (void)(param)

/*Compiler specific functions (all other atomics are built ontop of these)*/

#if defined(AK_ATOMIC_MSVC_COMPILER)
AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    /*Do a volatile load so that compiler doesn't duplicate loads, which makes
      them nonatomic.*/ 
    return ((volatile ak_atomic_u32*)Object)->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    ((volatile ak_atomic_u32*)Object)->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    return (uint32_t)_InterlockedExchange((volatile LONG*)Object, (LONG)NewValue);
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    return (uint32_t)_InterlockedCompareExchange((volatile LONG*)Object, (LONG)NewValue, (LONG)OldValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)_InterlockedCompareExchange((volatile LONG*)Object, (LONG)NewValue, (LONG)Old);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    return (uint32_t)_InterlockedExchangeAdd((volatile LONG*)Object, (LONG)Operand);
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return (uint32_t)_InterlockedIncrement((volatile LONG*)Object);
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return (uint32_t)_InterlockedDecrement((volatile LONG*)Object);
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    /*Do a volatile load so that compiler doesn't duplicate loads, which makes
      them nonatomic.*/ 
    return ((volatile ak_atomic_u64*)Object)->Nonatomic;
#else
    /*Interlocked compare exchange is the most compatibile way to get an atomic 
      64 bit load on 32 bit x86*/
    return (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed((ak_atomic_u64*)Object, 0, 0);
#endif
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    ((volatile ak_atomic_u64*)Object)->Nonatomic = Value;
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((LONGLONG*)Object, (LONGLONG)Value, (LONGLONG)Expected);
        if(Previous == Expected) break;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedExchange64((volatile LONGLONG*)Object, (LONGLONG)NewValue);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, (LONGLONG)NewValue, (LONGLONG)Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    return (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, (LONGLONG)NewValue, (LONGLONG)OldValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, (LONGLONG)NewValue, (LONGLONG)Old);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedExchangeAdd64((volatile LONGLONG*)Object, (LONGLONG)Operand);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, (LONGLONG)(Expected+Operand), (LONGLONG)Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedIncrement64((volatile LONGLONG*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedDecrement64((volatile LONGLONG*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
#endif
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_AARCH64_CPU)
AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;
    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   stxr %w1, %w3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;

    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   cmp  %w0, %w3\n"
        "   b.ne 2f\n"
        "   stxr %w1, %w4, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "Ir" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    uint32_t Status;
    uint32_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   add  %w3, %w0, %w4\n"
        "   stxr %w1, %w3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r" (TempAddRegister)
        : "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1) - 1;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;
    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   stxr %w1, %3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;

    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   cmp  %0, %3\n"
        "   b.ne 2f\n"
        "   stxr %w1, %4, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "Ir" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint32_t Status;
    uint64_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   add  %3, %0, %4\n"
        "   stxr %w1, %3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r" (TempAddRegister)
        : "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_ARM_CPU)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;
    __asm__ volatile(
        "1: ldrex %0, [%3]\n"
        "   strex %1, %4, [%3]\n"
        "   cmp   %1, #0\n"
        "   bne   1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(Object), "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    size_t Status;
    uint32_t Previous;

    __asm__ volatile(
        "1: ldrex   %0, [%3]\n"
        "   mov     %1, #0\n"
        "   teq     %0, %4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   strexeq %1, %5, [%3]\n"
        "   cmp     %1, #0\n"
        "   bne     1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    uint32_t Status;
    uint32_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldrex %0, [%4]\n" /*Stores the value of Object into the result*/
        "   mov   %3, %0\n" /*Stores the result into a temp register for addition*/
        "   add   %3, %5\n" /*Adds the temp register and operand and stores it into the temp register*/
        "   strex %1, %3, [%4]\n" /*Copy from the temp register to the final object*/
        "   cmp   %1, #0\n"
        "   bne   1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r"(TempAddRegister)
        : "r"(Object), "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1) - 1;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    uint64_t Result;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%1]"
        : "=&r"(Result)
        : "r"(Object), "Qo"(Object->Nonatomic)
    );
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    uint64_t Status;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%2]\n"
        "   strexd %0, %3, %H3, [%2]\n"
        "   teq    %0, #0\n"
        "   bne    1b"
        : "=&r"(Status), "=Qo" (Object->Nonatomic)
        : "r"(Object), "r"(Value)
        : "cc"
    );
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%3]\n"
        "   strexd %1, %4, %H4, [%3]\n"
        "   cmp    %1, #0\n"
        "   bne    1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    size_t Status;
    uint64_t Previous;

    __asm__ volatile(
        "1: ldrexd   %0, %H0, [%3]\n"
        "   mov      %1, #0\n"
        "   teq      %0, %4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   teqeq    %H0, %H4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   strexdeq %1, %5, %H5, [%3]\n"
        "   cmp      %1, #0\n"
        "   bne      1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint32_t Status;
    uint64_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldrexd %0,  %H0, [%4]\n" /*Stores the value of Object into the result*/
        "   adds   %3,  %0,  %5\n"
        "   adc    %H3, %H0, %H5\n"
        "   strexd %1,  %3, %H3, [%4]\n" /*Copy from the temp register to the final object*/
        "   teq    %1,  #0\n"
        "   bne    1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic), "=&r"(TempAddRegister)
        : "r"(Object), "r" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_X86_64_CPU)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    /*
    "=r"(previous) chooses any general register, makes that %0, and outputs
    this register to previous after the
    block.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "0"(operand) puts operand into the same register as %0 before the block.
    volatile is required. Otherwise, if the return value (previous) is unused,
    the asm block
    No lock prefix is necessary for XCHG.
    */
    uint32_t Result;
    __asm__ volatile(
        "xchgl %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(NewValue)
    );
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    /*
    "=a"(previous) means the asm block outputs EAX to previous, because CMPXCHG
    puts the old value in EAX.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "q"(desired) puts desired into any of EBX, ECX or EDX before the block.
    This becomes %2.
    "0"(expected) puts expected in the same register as "=a"(previous), which
    is EAX, before the block.*/
    uint32_t Result;
    __asm__ volatile(
        "lock; cmpxchgl %2, %1"
        : "=a"(Result), "+m"(Object->Nonatomic)
        : "q"(NewValue), "0"(OldValue) 
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    /*
    See AK_Atomic_Exchange_U32_Relaxed for register constraint explanations
    Lock prefix is necessary of xaddl
    */
   uint32_t Result;
   __asm__ volatile(
        "lock; xaddl %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(Operand)
   );
   return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;    
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1)-1;    
}

#if AK_ATOMIC_PTR_SIZE == 8
AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint64_t Result;
    __asm__ volatile(
        "xchgq %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(NewValue)
    );
    return Result;
}

AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    uint64_t Result;
    __asm__ volatile(
        "lock; cmpxchgq %2, %1"
        : "=a"(Result), "+m"(Object->Nonatomic)
        : "q"(NewValue), "0"(OldValue)
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint64_t Result;
    __asm__ volatile(
        "lock; xaddq %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(Operand)
    );
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1) + 1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#else

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    /* 
    On 32-bit x86, the most compatible way to get an atomic 64-bit load is with
    cmpxchg8b.
    "=&A"(previous) outputs EAX:EDX to previous after the block, while telling
    the compiler that these registers are clobbered before %1 is used, so don't 
    use EAX or EDX for %1.
    "m"(object->nonatomic) loads object's address into a register, which
    becomes %1, before the block.*/
    uint64_t Result;
    __asm__ volatile(
        "movl %%ebx, %%eax\n"
        "movl %%ecx, %%edx\n"
        "lock; cmpxchg8b %1"
        : "=&A"(Result)
        : "m"(Object->Nonatomic)
    );
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    /*
    On 32-bit x86, the most compatible way to get an atomic 64-bit store is
    with cmpxchg8b.
    Essentially, we perform turf_compareExchange64Relaxed(object, object->nonatomic, desired)
    in a loop until it returns the previous value.
    According to the Linux kernel (atomic64_cx8_32.S), we don't need the
    "lock;" prefix on cmpxchg8b since aligned 64-bit writes are already atomic 
    on 586 and newer.
    "=m"(object->nonatomic) loads object's address into a register, which
    becomes %0, before the block, and tells the compiler the variable at 
    address will be modified by the block.
    "b" and "c" move desired to ECX:EBX before the block.
    "A"(expected) loads the previous value of object->nonatomic into EAX:EDX
    before the block. */
    uint64_t OldValue = Object->Nonatomic;
    __asm__ volatile(
        "1: cmpxchg8b %0\n"
        "   jne 1b"
        : "=m"(Object->Nonatomic)
        : "b"((uint32_t)NewValue), "c"((uint32_t)(NewValue >> 32)), "A"(OldValue)
    );
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint64_t OldValue = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue);
        if(Previous == OldValue)
            return OldValue;
        OldValue = Previous; 
    }
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    /*
    cmpxchg8b is the only way to do 64-bit RMW operations on 32-bit x86.
    "=A"(previous) outputs EAX:EDX to previous after the block.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "b" and "c" move desired to ECX:EBX before the block.
    "0"(expected) puts expected in the same registers as "=a"(previous), which
    are EAX:EDX, before the block.*/
    uint64_t Result;
    __asm__ volatile(
        "lock; cmpxchg8b %1"
        : "=A"(Result), "+m"(Object->Nonatomic)
        : "b"((uint32_t)NewValue), "c"((uint32_t)(NewValue >> 32)), "0"(OldValue)
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    for(;;) {
        uint64_t OldValue = Object->Nonatomic;
        if(AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, OldValue+Operand))
            return OldValue;
    }
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1) + 1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}
#endif

#else
#   error "Not Implemented"
#endif

/*Ptr type (is either 32 bit or 64 bit wrappers)*/
#if (AK_ATOMIC_PTR_SIZE == 8)
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object) {
    return (void*)AK_Atomic_Load_U64_Relaxed((const ak_atomic_u64*)Object);
}

AKATOMICDEF void AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value) {
    AK_Atomic_Store_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)Value);
}
 
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue) {
    return (void*)AK_Atomic_Exchange_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)NewValue);
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return (void*)AK_Atomic_Compare_Exchange_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)OldValue, (uint64_t)NewValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_U64_Weak_Relaxed((ak_atomic_u64*)Object, (uint64_t*)OldValue, (uint64_t)NewValue);
}
#else
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object) {
    return (void*)AK_Atomic_Load_U32_Relaxed((const ak_atomic_u32*)Object);
}

AKATOMICDEF void AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value) {
    AK_Atomic_Store_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)Value);
}
 
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue) {
    return (void*)AK_Atomic_Exchange_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)NewValue);
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return (void*)AK_Atomic_Compare_Exchange_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)OldValue, (uint32_t)NewValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_U32_Weak_Relaxed((ak_atomic_u32*)Object, (uint32_t*)OldValue, (uint32_t)NewValue);
}
#endif

/*Compare exchange for boolean results*/
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    return AK_Atomic_Compare_Exchange_U32_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    return AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_Ptr_Relaxed(Object, OldValue, NewValue) == OldValue;
}

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    uint32_t Result = AK_Atomic_Load_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_U32_Relaxed(Object, Value);
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Exchange_U32_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Compare_Exchange_U32_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Fetch_Add_U32_Relaxed(Object, Operand);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Increment_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Decrement_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    uint64_t Result = AK_Atomic_Load_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_U64_Relaxed(Object, Value);
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Exchange_U64_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
} 

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Fetch_Add_U64_Relaxed(Object, Operand);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Increment_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Decrement_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void* AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder) {
    void* Result = AK_Atomic_Load_Ptr_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_Ptr_Relaxed(Object, Value);
}

AKATOMICDEF void* AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    void* Result = AK_Atomic_Exchange_Ptr_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    void* Result = AK_Atomic_Compare_Exchange_Ptr_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

/*OS Thread primitives*/
#if !defined(AK_OS_THREAD_ASSERT)
#include <assert.h>
#define AK_OS_THREAD_ASSERT(cond) assert(cond)
#endif

#if !defined(AK_OS_THREAD_MEMSET)
#define AK_OS_THREAD_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if defined(AK_ATOMIC_WIN32_OS)
AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    SYSTEM_INFO SystemInfo;
    AK_OS_THREAD_MEMSET(&SystemInfo, 0, sizeof(SYSTEM_INFO));
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
}

AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    Sleep(Milliseconds);
}

AKATOMICDEF uint64_t AK_Query_Performance_Counter(void) {
    uint64_t Result;
    QueryPerformanceCounter((LARGE_INTEGER*)&Result);
    return Result;
}

AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void) {
    uint64_t Result;
    QueryPerformanceFrequency((LARGE_INTEGER*)&Result);
    return Result;  
}

static DWORD WINAPI AK_Thread__Internal_Proc(LPVOID Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return (DWORD)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData) {
    Thread->Callback = Callback;
    Thread->UserData = UserData;
    Thread->Handle = CreateThread(NULL, 0, AK_Thread__Internal_Proc, Thread, 0, NULL);
    return Thread->Handle != NULL;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
    AK_Thread_Wait(Thread);
    if(Thread->Handle != NULL) {
        CloseHandle(Thread->Handle);
        Thread->Handle = NULL;
        Thread->Callback = NULL;
        Thread->UserData = NULL;
    }
}
AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
    if(Thread->Handle != NULL) {
        WaitForSingleObject(Thread->Handle, INFINITE);
    }
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
    if(Thread->Handle != NULL) {
        return GetThreadId(Thread->Handle);
    }
    return 0;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
    return GetCurrentThreadId();
}

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex) {
    InitializeCriticalSection(&Mutex->CriticalSection);
    return true;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
    DeleteCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
    LeaveCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    EnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return TryEnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    Semaphore->Handle = CreateSemaphoreA(NULL, InitialCount, 0x7fffffff, NULL);
    return Semaphore->Handle != NULL;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
    CloseHandle(Semaphore->Handle);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
    ReleaseSemaphore(Semaphore->Handle, 1, NULL);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
    WaitForSingleObject(Semaphore->Handle, INFINITE);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
    ReleaseSemaphore(Semaphore->Handle, Addend, NULL);
}

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
    InitializeConditionVariable(&ConditionVariable->Variable);
    return true;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
    /*Noop on win32*/
    AK_ATOMIC__UNREFERENCED_PARAMETER(ConditionVariable);
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex) {
    SleepConditionVariableCS(&ConditionVariable->Variable, &Mutex->CriticalSection, INFINITE);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
    WakeConditionVariable(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
    WakeAllConditionVariable(&ConditionVariable->Variable);
}

AKATOMICDEF bool AK_TLS_Create(ak_tls* TLS) {
    TLS->Index = TlsAlloc();
    return TLS->Index != TLS_OUT_OF_INDEXES;
}

AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS) {
    TlsFree(TLS->Index);
}

AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS) {
    return TlsGetValue(TLS->Index);
}
AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data) {
    TlsSetValue(TLS->Index, Data);
}

#elif defined(AK_ATOMIC_POSIX_OS)

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
}

AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    struct timespec Time;
    Time.tv_sec = Milliseconds / 1000;
    Time.tv_nsec = (Milliseconds % 1000) * 1000000;
    nanosleep(&Time, NULL);
}

#define AK__NS_PER_SECOND 1000000000
AKATOMICDEF uint64_t AK_Query_Performance_Counter() {
    struct timespec Now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &Now);

    uint64_t Result = Now.tv_sec;
    Result *= AK__NS_PER_SECOND;
    Result += Now.tv_nsec;

    return Result;
}

AKATOMICDEF uint64_t AK_Query_Performance_Frequency() {
    return AK__NS_PER_SECOND;
}

static void* AK_Thread__Internal_Proc(void* Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return (void*)(size_t)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData) {
    Thread->Callback = Callback;
    Thread->UserData = UserData;
    return pthread_create(&Thread->Thread, NULL, AK_Thread__Internal_Proc, Thread) == 0;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
    AK_Thread_Wait(Thread);
    Thread->Thread = 0;
    Thread->Callback = NULL;
    Thread->UserData = NULL;
}
AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
    if(Thread->Thread != 0) {
        pthread_join(Thread->Thread, NULL);
        Thread->Thread = 0;
    }
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
    if(Thread->Thread != 0) {
        return (uint64_t)Thread->Thread;
    }
    return 0;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
    return (uint64_t)pthread_self();
}

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex) {
    return pthread_mutex_init(&Mutex->Mutex, NULL) == 0;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
    pthread_mutex_destroy(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
    pthread_mutex_unlock(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    pthread_mutex_lock(&Mutex->Mutex);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return pthread_mutex_trylock(&Mutex->Mutex);
}
#ifdef AK_ATOMIC_OSX_OS
AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    return semaphore_create(mach_task_self(), &Semaphore->Semaphore, SYNC_POLICY_FIFO, 0) == KERN_SUCCESS;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
    semaphore_destroy(mach_task_self(), Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
    semaphore_signal(Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
    semaphore_wait(Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
    int32_t i;
    for(i = 0; i < Addend; i++)
        semaphore_signal(Semaphore->Semaphore);
}
#else
AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    return sem_init(&Semaphore->Semaphore, 0, InitialCount) == 0;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
    sem_destroy(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
    sem_post(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
    sem_wait(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
    int32_t i;
    for(i = 0; i < Addend; i++)
        sem_post(&Semaphore->Semaphore);
}
#endif

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
    return pthread_cond_init(&ConditionVariable->Variable, NULL) == 0;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
    pthread_cond_destroy(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex) {
    pthread_cond_wait(&ConditionVariable->Variable, &Mutex->Mutex);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
    pthread_cond_signal(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
    pthread_cond_broadcast(&ConditionVariable->Variable);
}

AKATOMICDEF bool AK_TLS_Create(ak_tls* TLS) {
    return pthread_key_create(&TLS->Key, NULL) == 0;
}

AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS) {
    pthread_key_delete(TLS->Key);
}

AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS) {
    return pthread_getspecific(TLS->Key);
}
AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data) {
    pthread_setspecific(TLS->Key, Data);
}

#else
#error "Not Implemented"
#endif

static bool AK_LW_Semaphore__Internal_Try(ak_lw_semaphore* Semaphore) {
    int32_t OldCount = (int32_t)AK_Atomic_Load_U32_Relaxed(&Semaphore->Count);
    while(OldCount > 0) {
        if(AK_Atomic_Compare_Exchange_U32_Weak_Explicit(&Semaphore->Count, (uint32_t*)&OldCount, (uint32_t)(OldCount-1), AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return true;
        }
    }
    return false;
}

static void AK_LW_Semaphore__Internal_Partial_Spin_Wait(ak_lw_semaphore* Semaphore) {
    int32_t OldCount;
    uint32_t SpinCount = 0;
    while(SpinCount++ < Semaphore->MaxSpinCount) {
        OldCount = (int32_t)AK_Atomic_Load_U32_Relaxed(&Semaphore->Count);
        if((OldCount > 0) && AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&Semaphore->Count, (uint32_t)OldCount, (uint32_t)(OldCount-1), AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED))
            return;
        AK_Atomic_Compiler_Fence_Acq(); /*Prevent compiler from collapsing the loop*/
    }
    OldCount = (int32_t)AK_Atomic_Fetch_Add_U32(&Semaphore->Count, -1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(OldCount > 0) return;   
    AK_Semaphore_Decrement(&Semaphore->InternalSem);
    return;
}

AKATOMICDEF bool AK_LW_Semaphore_Create_With_Spin_Count(ak_lw_semaphore* Semaphore, int32_t InitialCount, uint32_t SpinCount) {
    AK_Atomic_Store_U32_Relaxed(&Semaphore->Count, (uint32_t)InitialCount);
    Semaphore->MaxSpinCount = SpinCount;
    return AK_Semaphore_Create(&Semaphore->InternalSem, 0);
}

AKATOMICDEF bool AK_LW_Semaphore_Create(ak_lw_semaphore* Semaphore, int32_t InitialCount) {
    return AK_LW_Semaphore_Create_With_Spin_Count(Semaphore, InitialCount, 10000);
}

AKATOMICDEF void AK_LW_Semaphore_Delete(ak_lw_semaphore* Semaphore) {
    AK_Semaphore_Delete(&Semaphore->InternalSem);
}

AKATOMICDEF void AK_LW_Semaphore_Decrement(ak_lw_semaphore* Semaphore) {
    if(!AK_LW_Semaphore__Internal_Try(Semaphore)) {
        AK_LW_Semaphore__Internal_Partial_Spin_Wait(Semaphore);
    }
}

AKATOMICDEF void AK_LW_Semaphore_Increment(ak_lw_semaphore* Semaphore) {
    AK_LW_Semaphore_Add(Semaphore, 1);
}

AKATOMICDEF void AK_LW_Semaphore_Add(ak_lw_semaphore* Semaphore, int32_t Addend) {
    int32_t OldCount = (int32_t)AK_Atomic_Fetch_Add_U32(&Semaphore->Count, Addend, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    int32_t ToRelease = -OldCount < Addend ? -OldCount : Addend;
    if(ToRelease > 0) {
        AK_Semaphore_Add(&Semaphore->InternalSem, ToRelease);
    }
}

AKATOMICDEF bool AK_Event_Create(ak_event* Event) {
    Event->State = false;
    if(!AK_Mutex_Create(&Event->Mutex)) return false;
    if(!AK_Condition_Variable_Create(&Event->CondVar)) return false;
    return true;
}

AKATOMICDEF void AK_Event_Delete(ak_event* Event) {
    AK_Condition_Variable_Delete(&Event->CondVar);
    AK_Mutex_Delete(&Event->Mutex);
}

AKATOMICDEF void AK_Event_Signal(ak_event* Event) {
    AK_Mutex_Lock(&Event->Mutex);
    Event->State = true;
    AK_Condition_Variable_Wake_All(&Event->CondVar);
    AK_Mutex_Unlock(&Event->Mutex);
}

AKATOMICDEF void AK_Event_Wait(ak_event* Event) {
    AK_Mutex_Lock(&Event->Mutex);
    while(!Event->State) {
        AK_Condition_Variable_Wait(&Event->CondVar, &Event->Mutex);
    }
    AK_Mutex_Unlock(&Event->Mutex);
}

AKATOMICDEF void AK_Event_Reset(ak_event* Event) {
    AK_Mutex_Lock(&Event->Mutex);
    Event->State = false;
    AK_Mutex_Unlock(&Event->Mutex);
}

AKATOMICDEF bool AK_Auto_Reset_Event_Create(ak_auto_reset_event* Event, int32_t InitialStatus) {
    if(InitialStatus < 0 || InitialStatus > 1) {
        return false;
    }
    bool Result = AK_LW_Semaphore_Create(&Event->Semaphore, 0);
    AK_Atomic_Store_U32(&Event->Status, (uint32_t)InitialStatus, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    return Result;
}

AKATOMICDEF void AK_Auto_Reset_Event_Delete(ak_auto_reset_event* Event) {
    AK_LW_Semaphore_Delete(&Event->Semaphore);
}

AKATOMICDEF void AK_Auto_Reset_Event_Signal(ak_auto_reset_event* Event) {
    int32_t OldStatus = (int32_t)AK_Atomic_Load_U32_Relaxed(&Event->Status);
    for(;;) {
        AK_OS_THREAD_ASSERT(OldStatus <= 1);
        int32_t NewStatus = OldStatus < 1 ? OldStatus + 1 : 1;
        if(AK_Atomic_Compare_Exchange_U32_Weak_Explicit(&Event->Status, (uint32_t*)&OldStatus, (uint32_t)NewStatus, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            break;
        }
    }
    if(OldStatus < 0) {
        AK_LW_Semaphore_Increment(&Event->Semaphore);
    }
}

AKATOMICDEF void AK_Auto_Reset_Event_Wait(ak_auto_reset_event* Event) {
    int32_t OldStatus = (int32_t)AK_Atomic_Fetch_Add_U32(&Event->Status, -1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    AK_OS_THREAD_ASSERT(OldStatus <= 1);
    if(OldStatus < 1) {
        AK_LW_Semaphore_Decrement(&Event->Semaphore);
    }
}

#define AK_RW_LOCK_STATUS__MAX_VALUE ((1 << 10) - 1)

typedef union {
    uint32_t Value;
    struct {
        uint32_t Readers    : 10;
        uint32_t WaitToRead : 10;
        uint32_t Writers    : 10;
    } Bits;
} ak_rw_lock__status;

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_rw_lock__status) == 4);

AKATOMICDEF bool AK_RW_Lock_Create(ak_rw_lock* Lock) {
    Lock->Status.Nonatomic = 0;
    if(!AK_LW_Semaphore_Create(&Lock->ReadSemaphore, 0)) return false;
    if(!AK_LW_Semaphore_Create(&Lock->WriteSemaphore, 0)) return false;
    return true;
}

AKATOMICDEF void AK_RW_Lock_Delete(ak_rw_lock* Lock) {
    AK_LW_Semaphore_Delete(&Lock->WriteSemaphore);
    AK_LW_Semaphore_Delete(&Lock->ReadSemaphore);
    Lock->Status.Nonatomic = 0;
}

AKATOMICDEF void AK_RW_Lock_Reader(ak_rw_lock* Lock) {
    ak_rw_lock__status OldStatus = {AK_Atomic_Load_U32_Relaxed(&Lock->Status)};
    ak_rw_lock__status NewStatus;

    do {
        NewStatus = OldStatus;
        if(OldStatus.Bits.Writers > 0) {
            NewStatus.Bits.WaitToRead++;
        } else {
            NewStatus.Bits.Readers++;
        }
    } while(!AK_Atomic_Compare_Exchange_U32_Weak_Explicit(&Lock->Status, (uint32_t*)&OldStatus, NewStatus.Value, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED));

    if(OldStatus.Bits.Writers > 0) {
        AK_LW_Semaphore_Decrement(&Lock->ReadSemaphore);
    }
}

AKATOMICDEF void AK_RW_Unlock_Reader(ak_rw_lock* Lock) {
    const int32_t Value = (int32_t)(1 << 0);
    ak_rw_lock__status OldStatus = {AK_Atomic_Fetch_Add_U32(&Lock->Status, -Value, AK_ATOMIC_MEMORY_ORDER_RELEASE)};
    AK_OS_THREAD_ASSERT(OldStatus.Bits.Readers > 0);
    if(OldStatus.Bits.Readers == 1 && OldStatus.Bits.Writers > 0) {
        AK_LW_Semaphore_Increment(&Lock->WriteSemaphore);
    }
}

AKATOMICDEF void AK_RW_Lock_Writer(ak_rw_lock* Lock) {
    const uint32_t Value = (1 << 20);
    ak_rw_lock__status OldStatus = {AK_Atomic_Fetch_Add_U32(&Lock->Status, Value, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)};

    AK_OS_THREAD_ASSERT(OldStatus.Bits.Writers + 1 <= AK_RW_LOCK_STATUS__MAX_VALUE);
    if(OldStatus.Bits.Readers > 0 || OldStatus.Bits.Writers > 0) {
        AK_LW_Semaphore_Decrement(&Lock->WriteSemaphore);
    }
}

AKATOMICDEF void AK_RW_Unlock_Writer(ak_rw_lock* Lock) {
    ak_rw_lock__status OldStatus = {AK_Atomic_Load_U32_Relaxed(&Lock->Status)};
    ak_rw_lock__status NewStatus;

    uint32_t WaitToRead = 0;
    do {
        AK_OS_THREAD_ASSERT(OldStatus.Bits.Readers == 0);
        NewStatus = OldStatus;
        NewStatus.Bits.Writers--;
        WaitToRead = OldStatus.Bits.WaitToRead;
        if(WaitToRead > 0) {
            NewStatus.Bits.WaitToRead = 0;
            NewStatus.Bits.Readers = WaitToRead;
        }
    } while(!AK_Atomic_Compare_Exchange_U32_Weak_Explicit(&Lock->Status, (uint32_t*)&OldStatus, NewStatus.Value, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED));

    if(WaitToRead > 0) {
        AK_LW_Semaphore_Add(&Lock->ReadSemaphore, (int32_t)WaitToRead);
    } else if(OldStatus.Bits.Writers > 1) {
        AK_LW_Semaphore_Increment(&Lock->WriteSemaphore);
    }
}

#ifndef AK_DISABLE_ASYNC_STACK_INDEX

#ifndef AK_ASYNC_STACK_INDEX_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_ASYNC_STACK_INDEX_MALLOC)
#include <stdlib.h>
#define AK_ASYNC_STACK_INDEX_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_ASYNC_STACK_INDEX_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_ASYNC_STACK_INDEX_MEMSET)
#define AK_ASYNC_STACK_INDEX_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_ASYNC_STACK_INDEX_ASSERT)
#include <assert.h>
#define AK_ASYNC_STACK_INDEX_ASSERT(cond) assert(cond)
#endif

typedef union {
    uint64_t ID;
    struct {
        uint32_t Index;
        uint32_t Key;
    } KeyIndex;
} ak_async_stack_index32__key;

static ak_async_stack_index32__key AK_Async_Stack_Index32__Make_Key(uint32_t Index, uint32_t Key) {
    ak_async_stack_index32__key Result;
    Result.KeyIndex.Index = Index;
    Result.KeyIndex.Key = Key;
    return Result;
}

AKATOMICDEF void AK_Async_Stack_Index32_Init_Raw(ak_async_stack_index32* StackIndex, uint32_t* IndicesPtr, uint32_t Capacity) {
    /*Make sure atomic is properly aligned*/
    AK_ASYNC_STACK_INDEX_ASSERT((((size_t)&StackIndex->Head) % 8) == 0);
    StackIndex->NextIndices = IndicesPtr;
    StackIndex->Capacity = Capacity;
    ak_async_stack_index32__key* HeadKey = (ak_async_stack_index32__key*)&StackIndex->Head.Nonatomic; 
    HeadKey->KeyIndex.Index = AK_ASYNC_STACK_INDEX32_INVALID;
    HeadKey->KeyIndex.Key = 0;
}

AKATOMICDEF void AK_Async_Stack_Index32_Push_Sync(ak_async_stack_index32* StackIndex, uint32_t Index) {
    ak_async_stack_index32__key* CurrentTop = (ak_async_stack_index32__key*)&StackIndex->Head.Nonatomic;
    uint32_t Current = CurrentTop->KeyIndex.Index;
    StackIndex->NextIndices[Index] = Current;
    CurrentTop->KeyIndex.Index = Index;
}

AKATOMICDEF void AK_Async_Stack_Index32_Push(ak_async_stack_index32* StackIndex, uint32_t Index) {
    for(;;) {
        ak_async_stack_index32__key CurrentTop = { AK_Atomic_Load_U64_Relaxed(&StackIndex->Head)};
        uint32_t Current = CurrentTop.KeyIndex.Index;
        StackIndex->NextIndices[Index] = Current;
        ak_async_stack_index32__key NewTop = AK_Async_Stack_Index32__Make_Key(Index, CurrentTop.KeyIndex.Key+1); /*Increment key to avoid ABA problem*/
        /*Add job index to the freelist atomically*/
        if(AK_Atomic_Compare_Exchange_Bool_U64_Explicit(&StackIndex->Head, CurrentTop.ID, NewTop.ID, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return;
        }
    }
}

AKATOMICDEF uint32_t AK_Async_Stack_Index32_Pop(ak_async_stack_index32* StackIndex) {
    for(;;) {
        ak_async_stack_index32__key CurrentTop = { AK_Atomic_Load_U64_Relaxed(&StackIndex->Head)};

        uint32_t Index = CurrentTop.KeyIndex.Index;
        if(Index == AK_ASYNC_STACK_INDEX32_INVALID) {
            /*No more jobs avaiable*/
            return AK_ASYNC_STACK_INDEX32_INVALID;
        }
        AK_ASYNC_STACK_INDEX_ASSERT(Index < StackIndex->Capacity); /*Overflow*/
        uint32_t Next = StackIndex->NextIndices[Index];
        ak_async_stack_index32__key NewTop = AK_Async_Stack_Index32__Make_Key(Next, CurrentTop.KeyIndex.Key+1); /*Increment key to avoid ABA problem*/
        /*Atomically update the job freelist*/
        if(AK_Atomic_Compare_Exchange_Bool_U64_Explicit(&StackIndex->Head, CurrentTop.ID, NewTop.ID, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return Index;
        }
    }
}

AKATOMICDEF bool AK_Async_Stack_Index32_Alloc(ak_async_stack_index32* StackIndex, uint32_t Capacity, void* AllocUserData) {
    size_t AllocationSize = Capacity*sizeof(uint32_t);
    void* Memory = AK_ASYNC_STACK_INDEX_MALLOC(AllocationSize, AllocUserData);
    if(!Memory) return false;

    AK_ASYNC_STACK_INDEX_MEMSET(Memory, 0, AllocationSize);
    AK_Async_Stack_Index32_Init_Raw(StackIndex, (uint32_t*)Memory, Capacity);
    StackIndex->AllocUserData = AllocUserData;
    return true;
}

AKATOMICDEF void AK_Async_Stack_Index32_Free(ak_async_stack_index32* StackIndex) {
    if(StackIndex->NextIndices) {
        AK_ASYNC_STACK_INDEX_FREE(StackIndex->NextIndices, StackIndex->AllocUserData);
    }
    AK_ASYNC_STACK_INDEX_MEMSET(StackIndex, 0, sizeof(ak_async_stack_index32));
}

#endif

#ifndef AK_DISABLE_ASYNC_SPMC_QUEUE_INDEX

#ifndef AK_ASYNC_SPMC_QUEUE_INDEX_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_ASYNC_SPMC_QUEUE_INDEX_MALLOC)
#define AK_ASYNC_SPMC_QUEUE_INDEX_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_ASYNC_SPMC_QUEUE_INDEX_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_ASYNC_SPMC_QUEUE_INDEX_MEMSET)
#define AK_ASYNC_SPMC_QUEUE_INDEX_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_ASYNC_SPMC_QUEUE_INDEX_ASSERT)
#include <assert.h>
#define AK_ASYNC_SPMC_QUEUE_INDEX_ASSERT(cond) assert(cond)
#endif

AKATOMICDEF void AK_Async_SPMC_Queue_Index32_Init_Raw(ak_async_spmc_queue_index32* QueueIndex, uint32_t* IndicesPtr, uint32_t Capacity) {
    AK_ASYNC_SPMC_QUEUE_INDEX_ASSERT((((size_t)&QueueIndex->TopIndex) % 4) == 0);
    AK_ASYNC_SPMC_QUEUE_INDEX_ASSERT((((size_t)&QueueIndex->BottomIndex) % 4) == 0);
    QueueIndex->TopIndex.Nonatomic = 0;
    QueueIndex->BottomIndex.Nonatomic = 0;
    QueueIndex->Indices = IndicesPtr;
    QueueIndex->Capacity = Capacity;
}

AKATOMICDEF void AK_Async_SPMC_Queue_Index32_Enqueue(ak_async_spmc_queue_index32* QueueIndex, uint32_t Index) {
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32_Relaxed(&QueueIndex->BottomIndex);
    int32_t Top = (int32_t)AK_Atomic_Load_U32(&QueueIndex->TopIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(((int32_t)QueueIndex->Capacity-1) < (Bottom-Top)) {
        AK_ASYNC_SPMC_QUEUE_INDEX_ASSERT(false);
        return;
    }
    QueueIndex->Indices[Bottom % QueueIndex->Capacity] = Index;
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();
    AK_Atomic_Store_U32(&QueueIndex->BottomIndex, (uint32_t)(Bottom+1), AK_ATOMIC_MEMORY_ORDER_RELEASE);
}

AKATOMICDEF uint32_t AK_Async_SPMC_Queue_Index32_Dequeue(ak_async_spmc_queue_index32* QueueIndex) {
    int32_t Top = (int32_t)AK_Atomic_Load_U32(&QueueIndex->TopIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    /*Bottom needs to be read after top so an acquire barrier is sufficient (LoadLoad situation)*/
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32(&QueueIndex->BottomIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);

    uint32_t Result = AK_ASYNC_SPMC_QUEUE_INDEX32_INVALID;
    if(Top < Bottom) {
        Result = QueueIndex->Indices[Top % QueueIndex->Capacity];
        if(!AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&QueueIndex->TopIndex, (uint32_t)Top, (uint32_t)(Top+1), AK_ATOMIC_MEMORY_ORDER_ACQ_REL, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            Result = AK_ASYNC_SPMC_QUEUE_INDEX32_INVALID;
        }
    } 
    return Result;
}

AKATOMICDEF bool AK_Async_SPMC_Queue_Index32_Alloc(ak_async_spmc_queue_index32* QueueIndex, uint32_t Capacity, void* AllocUserData) {
    size_t AllocationSize = Capacity*sizeof(uint32_t);
    void* Memory = AK_ASYNC_SPMC_QUEUE_INDEX_MALLOC(AllocationSize, AllocUserData);
    if(!Memory) return false;

    AK_ASYNC_SPMC_QUEUE_INDEX_MEMSET(Memory, 0, AllocationSize);
    AK_Async_SPMC_Queue_Index32_Init_Raw(QueueIndex, (uint32_t*)Memory, Capacity);
    QueueIndex->AllocUserData = AllocUserData;
    return true;
}

AKATOMICDEF void AK_Async_SPMC_Queue_Index32_Free(ak_async_spmc_queue_index32* QueueIndex) {
    if(QueueIndex->Indices) {
        AK_ASYNC_SPMC_QUEUE_INDEX_FREE(QueueIndex->Indices, QueueIndex->AllocUserData);
    }
    AK_ASYNC_SPMC_QUEUE_INDEX_MEMSET(QueueIndex, 0, sizeof(ak_async_spmc_queue_index32));
}

#endif

#ifndef AK_DISABLE_ASYNC_SLOT_MAP

#ifndef AK_ASYNC_SLOT_MAP_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_ASYNC_SLOT_MAP_MALLOC)
#define AK_ASYNC_SLOT_MAP_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_ASYNC_SLOT_MAP_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_ASYNC_SLOT_MAP_MEMSET)
#define AK_ASYNC_SLOT_MAP_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_ASYNC_SLOT_MAP_ASSERT)
#include <assert.h>
#define AK_ASYNC_SLOT_MAP_ASSERT(cond) assert(cond)
#endif

AKATOMICDEF void AK_Async_Slot_Map64_Init_Raw(ak_async_slot_map64* SlotMap, uint32_t* IndicesPtr, ak_slot64* SlotsPtr, uint32_t Capacity) {
    AK_Async_Stack_Index32_Init_Raw(&SlotMap->FreeIndices, IndicesPtr, Capacity);
    SlotMap->Slots = (ak_slot64__internal*)SlotsPtr;

    uint32_t i; 
    for(i = 0; i < Capacity; i++) {
        /*Add all the entries to the freelist since every job is free to begin with. 
        This can be done synchronously without worrying about the stack key and aba problem */
        AK_Async_Stack_Index32_Push_Sync(&SlotMap->FreeIndices, i);

        SlotMap->Slots[i].Data.Index = i;
        SlotMap->Slots[i].Data.Generation.Nonatomic = 1;
    }
}

AKATOMICDEF bool AK_Async_Slot_Map64_Is_Allocated(const ak_async_slot_map64* SlotMap, ak_slot64 Slot) {
    ak_slot64__internal SlotID = {Slot};
    AK_ASYNC_SLOT_MAP_ASSERT(SlotID.Data.Index < SlotMap->FreeIndices.Capacity); /*Overflow!*/
    return AK_Atomic_Load_U32_Relaxed(&SlotMap->Slots[SlotID.Data.Index].Data.Generation) == SlotID.Data.Generation.Nonatomic;
}

AKATOMICDEF ak_slot64 AK_Async_Slot_Map64_Alloc_Slot(ak_async_slot_map64* SlotMap) {
    uint32_t Index = AK_Async_Stack_Index32_Pop(&SlotMap->FreeIndices);
    if(Index == AK_ASYNC_STACK_INDEX32_INVALID) return 0;

    AK_ASYNC_SLOT_MAP_ASSERT(Index < SlotMap->FreeIndices.Capacity); /*Overflow*/
    ak_slot64__internal Result = SlotMap->Slots[Index];
    AK_ASYNC_SLOT_MAP_ASSERT(Index == Result.Data.Index); /*Index corruption!*/
    return Result.Slot;
}

AKATOMICDEF void AK_Async_Slot_Map64_Free_Slot(ak_async_slot_map64* SlotMap, ak_slot64 Slot) {
    ak_slot64__internal SlotID = {Slot};
    uint32_t NextGenerationIndex = SlotID.Data.Generation.Nonatomic+1;
    if(NextGenerationIndex == 0) NextGenerationIndex = 1; /*Generation cannot be 0*/
    AK_ASYNC_SLOT_MAP_ASSERT(SlotID.Data.Index < SlotMap->FreeIndices.Capacity); /*Overflow*/
    ak_slot64__internal* SlotMapSlotID = SlotMap->Slots + SlotID.Data.Index;
    AK_ASYNC_SLOT_MAP_ASSERT(SlotMapSlotID->Data.Index == SlotID.Data.Index);
    if(AK_Atomic_Compare_Exchange_Bool_U32(&SlotMapSlotID->Data.Generation, SlotID.Data.Generation.Nonatomic, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
        AK_Async_Stack_Index32_Push(&SlotMap->FreeIndices, SlotID.Data.Index);
    } else {
        AK_ASYNC_SLOT_MAP_ASSERT(false); /*Tried to delete an invalid slot*/
    }
}
AKATOMICDEF bool AK_Async_Slot_Map64_Alloc(ak_async_slot_map64* SlotMap, uint32_t Capacity, void* AllocUserData) {
    size_t AllocationSize = (sizeof(uint32_t)+sizeof(ak_slot64))*Capacity;
    void* Memory = AK_ASYNC_SLOT_MAP_MALLOC(AllocationSize, AllocUserData);
    if(!Memory) return false;

    uint32_t* IndicesPtr = (uint32_t*)Memory;
    ak_slot64* SlotsPtr = (ak_slot64*)(IndicesPtr+Capacity);
    AK_Async_Slot_Map64_Init_Raw(SlotMap, IndicesPtr, SlotsPtr, Capacity);
    SlotMap->AllocUserData = AllocUserData;
    return true;
}

AKATOMICDEF void AK_Async_Slot_Map64_Free(ak_async_slot_map64* SlotMap) {
    if(SlotMap->Slots) {
        AK_ASYNC_SLOT_MAP_FREE(SlotMap->FreeIndices.NextIndices, SlotMap->AllocUserData);
    }
    AK_ASYNC_SLOT_MAP_MEMSET(SlotMap, 0, sizeof(ak_async_slot_map64));
}

#endif

#ifndef AK_DISABLE_QSBR

#ifndef AK_QSBR_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_QSBR_MALLOC)
#define AK_QSBR_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_QSBR_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_QSBR_MEMSET)
#define AK_QSBR_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_QSBR_MEMCPY)
#define AK_QSBR_MEMCPY(mem, index, size) memcpy(mem, index, size)
#endif

#if !defined(AK_QSBR_ASSERT)
#include <assert.h>
#define AK_QSBR_ASSERT(cond) assert(cond)
#endif

#define AK_QSBR__ALIGNMENT 16

#if AK_ATOMIC_PTR_SIZE == 8
#define AK_QSBR_USERDATA_SIZE 55
#else
#define AK_QSBR_USERDATA_SIZE 59
#endif

enum {
    AK__QSBR_ACTION_FLAG_NONE,
    AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT = (1 << 0)
};

static void* AK_QSBR__Malloc(size_t Size, void* MallocUserData) {
    size_t Offset = AK_QSBR__ALIGNMENT - 1 + sizeof(void*);
    void* P1 = AK_QSBR_MALLOC(Size+Offset, MallocUserData);
    if(!P1) return NULL;

    void** P2 = (void**)(((size_t)(P1) + Offset) & ~(AK_QSBR__ALIGNMENT - 1));
    P2[-1] = P1;
         
    return P2;
}

static void AK_QSBR__Free(void* Memory, void* MallocUserData) {
    if(Memory) {
        void** P2 = (void**)Memory;
        AK_QSBR_FREE(P2[-1], MallocUserData);
    }
}

typedef struct {
    ak_qsbr_callback_func* Callback;
    uint8_t                UserData[AK_QSBR_USERDATA_SIZE];
    uint8_t                Flags;
} ak_qsbr__action;

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_qsbr__action) == 64);

static void AK_QSBR__Allocate_Action(ak_qsbr__action* Action, ak_qsbr_callback_func* Callback, void* UserData, size_t UserDataSize, void* MallocUserData) {
    void* Data;
    if(UserDataSize <= sizeof(Action->UserData)) {
        Data = Action->UserData;
        Action->Flags &= ~AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT;
    } else {
        void* SlowData = AK_QSBR__Malloc(UserDataSize, MallocUserData);
        AK_QSBR_ASSERT(SlowData);

        /*Copy pointer address to user data*/
        AK_QSBR_MEMCPY(Action->UserData, &SlowData, sizeof(void*));
        Data = SlowData;
        Action->Flags |= AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT;
    }
    Action->Callback = Callback;
    AK_QSBR_MEMCPY(Data, UserData, UserDataSize);
}

#define AK_QSBR__Get_User_Data_Heap(user_data) (void*)(*(size_t*)(user_data))

static void AK_QSBR__Free_Action(ak_qsbr__action* Action, void* AllocUserData) {
    if(Action->Flags & AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT) {
        void* Ptr = AK_QSBR__Get_User_Data_Heap(Action->UserData);
        AK_QSBR__Free(Ptr, AllocUserData);
        Action->Flags &= ~AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT;
    }
}

typedef struct {
    ak_qsbr__action* Ptr;
    size_t           Count;
    size_t           Capacity;
} ak_qsbr__action_list;

#define AK_QSBR__List_Append(List, Entry, type, MallocUserData) \
    if((List)->Count == (List)->Capacity) { \
        (List)->Capacity = (List)->Capacity ? (List)->Capacity*2 : 32; \
        type* Ptr = (type*)AK_QSBR__Malloc(sizeof(type)*(List)->Capacity, MallocUserData); \
        if((List)->Ptr) { \
            AK_QSBR_MEMCPY(Ptr, (List)->Ptr, (List)->Count*sizeof(type)); \
            AK_QSBR__Free((List)->Ptr, MallocUserData); \
        } \
        (List)->Ptr = Ptr; \
    } \
    (List)->Ptr[(List)->Count++] = Entry

#define AK_QSBR__List_Delete(List, MallocUserData) \
    if((List)->Ptr) { \
        AK_QSBR__Free((List)->Ptr, MallocUserData); \
    } \
    (List)->Ptr = NULL; \
    (List)->Count = 0; \
    (List)->Capacity = 0

typedef struct {
    uint16_t InUse    : 1;
    uint16_t WasIdle  : 1;
    int16_t  NextFree : 14;
} ak_qsbr__status;

typedef struct {
    ak_qsbr__status* Ptr;
    size_t           Count;
    size_t           Capacity;
} ak_qsbr__status_list;

static void AK_QSBR__Status_Init(ak_qsbr__status* Status) {
    Status->InUse    = 1;
    Status->WasIdle  = 0;
    Status->NextFree = 0;
}

struct ak_qsbr {
    void*    MallocUserData;
    ak_mutex Mutex;
    ak_qsbr__status_list Status;
    ak_qsbr__action_list DeferredActions;
    ak_qsbr__action_list PendingActions;
    int16_t FreeIndex;
    int16_t NumContexts;
    int16_t NumRemaining;
};

static void AK_QSBR__On_All_Quiescent_States_Passed(ak_qsbr* QSBR, ak_qsbr__action_list* ActionList) {
    *ActionList           = QSBR->PendingActions;
    QSBR->PendingActions  = QSBR->DeferredActions;
    AK_QSBR_MEMSET(&QSBR->DeferredActions, 0, sizeof(ak_qsbr__action_list));
    QSBR->NumRemaining = QSBR->NumContexts;
    
    size_t i;
    for(i = 0; i < QSBR->Status.Count; i++) {
        QSBR->Status.Ptr[i].WasIdle = 0;
    }
}

static void AK_QSBR__Process_All_Actions(ak_qsbr* QSBR, ak_qsbr__action_list* ActionList) {
    if(ActionList->Count) {
        /*Process all actions and then delete their data*/
        size_t i;
        for(i = 0; i < ActionList->Count; i++) {
            ak_qsbr__action* Action = ActionList->Ptr + i;
            void* UserData = 
                ((Action->Flags & AK__QSBR_ACTION_FLAG_HEAP_ALLOCATED_BIT) ? 
                 AK_QSBR__Get_User_Data_Heap(Action->UserData) : 
                 (void*)Action->UserData);
            Action->Callback(UserData);
        }

        for(i = 0; i < ActionList->Count; i++) {
            AK_QSBR__Free_Action(&ActionList->Ptr[i], QSBR->MallocUserData);
        }

        AK_QSBR__List_Delete(ActionList, QSBR->MallocUserData);
    }
}

ak_qsbr* AK_QSBR_Create(void* MallocUserData) {
    ak_qsbr* QSBR = (ak_qsbr*)AK_QSBR__Malloc(sizeof(ak_qsbr), MallocUserData);
    if(!QSBR) return NULL;

    AK_QSBR_MEMSET(QSBR, 0, sizeof(ak_qsbr));

    QSBR->MallocUserData = MallocUserData;
    AK_Mutex_Create(&QSBR->Mutex);

    QSBR->FreeIndex    = -1;
    QSBR->NumContexts  = 0;
    QSBR->NumRemaining = 0;
    return QSBR;
}

void AK_QSBR_Delete(ak_qsbr* QSBR) {
    if(QSBR) {
        AK_QSBR__List_Delete(&QSBR->PendingActions, QSBR->MallocUserData);
        AK_QSBR__List_Delete(&QSBR->DeferredActions, QSBR->MallocUserData);
        AK_QSBR__List_Delete(&QSBR->Status, QSBR->MallocUserData);
        AK_Mutex_Delete(&QSBR->Mutex);
        AK_QSBR__Free(QSBR, QSBR->MallocUserData);
    }
}

ak_qsbr_context AK_QSBR_Create_Context(ak_qsbr* QSBR) {
    AK_Mutex_Lock(&QSBR->Mutex);
    QSBR->NumContexts++;
    QSBR->NumRemaining++;
    AK_QSBR_ASSERT(QSBR->NumContexts < (1 << 14));
    ak_qsbr_context Result = (ak_qsbr_context)QSBR->FreeIndex;
    if(Result >= 0) {
        AK_QSBR_ASSERT(Result < (int16_t)QSBR->Status.Count);
        AK_QSBR_ASSERT(!QSBR->Status.Ptr[Result].InUse);
        QSBR->FreeIndex = QSBR->Status.Ptr[Result].NextFree;
        AK_QSBR__Status_Init(&QSBR->Status.Ptr[Result]);
    } else {
        Result = (ak_qsbr_context)QSBR->Status.Count;
        ak_qsbr__status Status;
        AK_QSBR__Status_Init(&Status);
        AK_QSBR__List_Append(&QSBR->Status, Status, ak_qsbr__status, QSBR->MallocUserData);
    }

    AK_Mutex_Unlock(&QSBR->Mutex);
    return Result;
}

void AK_QSBR_Delete_Context(ak_qsbr* QSBR, ak_qsbr_context Context) {
    /*I don't like that we have to allocate memory for the action list
      everytime we want to process actions. But I'm not sure how to save
      allocations and being memory safe*/
    ak_qsbr__action_list ActionList = {0};

    AK_Mutex_Lock(&QSBR->Mutex);
    AK_QSBR_ASSERT(Context < (int16_t)QSBR->Status.Count);
    if(QSBR->Status.Ptr[Context].InUse && !QSBR->Status.Ptr[Context].WasIdle) {
        AK_QSBR_ASSERT(QSBR->NumRemaining > 0);
        --QSBR->NumRemaining;
    }
    QSBR->Status.Ptr[Context].InUse = 0;
    QSBR->Status.Ptr[Context].NextFree = QSBR->FreeIndex;
    QSBR->FreeIndex = (int16_t)Context;
    QSBR->NumContexts--;
    if(!QSBR->NumRemaining) {
        AK_QSBR__On_All_Quiescent_States_Passed(QSBR, &ActionList);
    }
    
    AK_Mutex_Unlock(&QSBR->Mutex);
    AK_QSBR__Process_All_Actions(QSBR, &ActionList);
}

void AK_QSBR_Update(ak_qsbr* QSBR, ak_qsbr_context Context) {
    /*I don't like that we have to allocate memory for the action list
      everytime we want to process actions. But I'm not sure how to save
      allocations and being memory safe*/
    ak_qsbr__action_list ActionList = {0};

    AK_Mutex_Lock(&QSBR->Mutex);
    AK_QSBR_ASSERT(Context < (int16_t)QSBR->Status.Count);
    ak_qsbr__status* Status = QSBR->Status.Ptr + Context;
    AK_QSBR_ASSERT(Status->InUse);
    if(Status->WasIdle) {
        AK_Mutex_Unlock(&QSBR->Mutex);
        return;
    }

    Status->WasIdle = 1;
    AK_QSBR_ASSERT(QSBR->NumRemaining > 0);
    --QSBR->NumRemaining;
    if(!QSBR->NumRemaining) {
        AK_QSBR__On_All_Quiescent_States_Passed(QSBR, &ActionList);
    }

    AK_Mutex_Unlock(&QSBR->Mutex);
    AK_QSBR__Process_All_Actions(QSBR, &ActionList);
}

void AK_QSBR_Enqueue(ak_qsbr* QSBR, ak_qsbr_callback_func* Callback, void* UserData, size_t UserDataSize) {
    ak_qsbr__action Action;
    AK_QSBR__Allocate_Action(&Action, Callback, UserData, UserDataSize, QSBR->MallocUserData);

    AK_Mutex_Lock(&QSBR->Mutex);
    AK_QSBR__List_Append(&QSBR->DeferredActions, Action, ak_qsbr__action, QSBR->MallocUserData);
    AK_Mutex_Unlock(&QSBR->Mutex);
}

#endif

#ifndef AK_DISABLE_JOB

#ifndef AK_JOB_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_JOB_MALLOC)
#define AK_JOB_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_JOB_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_JOB_MEMSET)
#define AK_JOB_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_SYSTEM_MEMCPY)
#define AK_JOB_MEMCPY(mem, index, size) memcpy(mem, index, size)
#endif

#if !defined(AK_JOB_ASSERT)
#include <assert.h>
#define AK_JOB_ASSERT(cond) assert(cond)
#endif

#if AK_ATOMIC_PTR_SIZE == 8
#define AK_JOB__ALIGNMENT 16
#else
#define AK_JOB__ALIGNMENT 16
#endif

#define AK_Job__Align_Pow2(value, alignment) (((value) + (alignment)-1) & ~((alignment)-1))

typedef struct ak__job ak__job;
typedef struct ak__job_dependency ak__job_dependency;

enum {
    AK__JOB_INFO_FLAG_NONE,
    AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT = (1 << 0),
    AK__JOB_INFO_FLAG_FREE_WHEN_DONE_BIT = (1 << 1)
};

#define AK__INVALID_JOB_INDEX ((uint32_t)-1)
struct ak__job {
    uint32_t      Index;
    ak_atomic_u32 Generation;
    void*         JobCallback;
    ak__job*      ParentJob;
    ak_atomic_ptr DependencyList;
    ak_atomic_u32 PendingDependencies;
    ak_atomic_u32 PendingJobs;
    ak_atomic_u32 IsProcessing;
    uint8_t       UserData[AK_JOB_SYSTEM_FAST_USERDATA_SIZE];
    uint8_t       JobInfoFlags;
};

struct ak__job_dependency {
    uint32_t            Index;
    ak__job*            Job;
    ak__job_dependency* Next;
};

typedef struct {
    ak_async_stack_index32 FreeJobDependencies;
    ak__job_dependency*    JobDependencies;
    uint32_t               MaxDependencyCount;
	uint32_t               Unused__Padding;
} ak__job_dependencies;

typedef struct {
    int32_t             ProcessJob;
    ak__job*            Job;
    ak__job_dependency* Dependency;
} ak__job_dependency_iter;

typedef struct {
    ak_async_stack_index32 FreeJobIndices;
    ak__job*               Jobs; /*Array of max job count*/
} ak__job_storage;

/*Highly recommend job size to not go past 64 bytes. Comment out this static 
  assertion if you need more space*/
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak__job) == 128);

#if AK_ATOMIC_PTR_SIZE == 8
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak__job_dependency) == 24);
#else
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak__job_dependency) == 12);
#endif

static void* AK_Job__Malloc(size_t Size, void* MallocUserData) {
    size_t Offset = AK_JOB__ALIGNMENT - 1 + sizeof(void*);
    void* P1 = AK_JOB_MALLOC(Size+Offset, MallocUserData);
    if(!P1) return NULL;

    void** P2 = (void**)(((size_t)(P1) + Offset) & ~(AK_JOB__ALIGNMENT - 1));
    P2[-1] = P1;
         
    return P2;
}

static void AK_Job__Free(void* Memory, void* MallocUserData) {
    if(Memory) {
        void** P2 = (void**)Memory;
        AK_JOB_FREE(P2[-1], MallocUserData);
    }
}

#define AK__Job_Validate_Ptr(Ptr) ((((size_t)Ptr) & (AK_JOB__ALIGNMENT-1)) == 0)
#define AK__Job_Byte_Offset(ptr, offset, type) (type*)(((uint8_t*)(ptr))+offset)

static ak_job_id AK__Job_Make_ID(ak__job* Job) {
    ak_job_id JobID = (uint64_t)(Job->Index) | ((uint64_t)AK_Atomic_Load_U32_Relaxed(&Job->Generation) << 32);
    return JobID;
}


static void AK_Job__Allocate_Data(ak__job* Job, void* Bytes, size_t ByteSize, void* AllocUserData) {
    void* Data;
    if(ByteSize <= sizeof(Job->UserData)) {
        Data = Job->UserData;
        Job->JobInfoFlags &= ~AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT;
    } else {
        void* SlowData = AK_Job__Malloc(ByteSize, AllocUserData);
        AK_JOB_ASSERT(SlowData);

        /*Copy pointer address to user data*/
        AK_JOB_MEMCPY(Job->UserData, &SlowData, sizeof(void*));
        Data = SlowData;
        Job->JobInfoFlags |= AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT;
    }
    AK_JOB_MEMCPY(Data, Bytes, ByteSize);
}

#define AK_Job__Get_User_Data_Heap(user_data) (void*)(*(size_t*)(user_data))

static void AK_Job__Free_Data(ak__job* Job, void* AllocUserData) {
    if(Job->JobInfoFlags & AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT) {
        void* Ptr = AK_Job__Get_User_Data_Heap(Job->UserData);
        AK_Job__Free(Ptr, AllocUserData);
        Job->JobInfoFlags &= ~AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT;
    }
}

static void AK_Job__Storage_Init(ak__job_storage* JobStorage, uint32_t* FreeJobIndices, ak__job* Jobs, uint32_t JobCount) {
    AK_Async_Stack_Index32_Init_Raw(&JobStorage->FreeJobIndices, FreeJobIndices, JobCount);
    AK_JOB_ASSERT(AK__Job_Validate_Ptr(Jobs));
    JobStorage->Jobs = Jobs;

    uint32_t i;
    for(i = 0; i < JobCount; i++) {
        /*Add all the entries to the freelist since every job is free to begin with. 
          This can be done synchronously without worrying about the stack key and aba problem */
        AK_Async_Stack_Index32_Push_Sync(&JobStorage->FreeJobIndices, i);

        /*Set index and generation for the job. Generation of 0 is not valid*/
        JobStorage->Jobs[i].Index = i;
        JobStorage->Jobs[i].Generation.Nonatomic = 1;
        AK_JOB_ASSERT((((size_t)&JobStorage->Jobs[i].PendingJobs) % 4) == 0);
        AK_JOB_ASSERT((((size_t)&JobStorage->Jobs[i].Generation) % 4) == 0);
    }
}

static ak__job* AK_Job__Storage_Get(ak__job_storage* JobStorage, ak_job_id JobID) {
    uint32_t Index = (uint32_t)JobID;
    uint32_t Generation = (uint32_t)(JobID >> 32);
    AK_JOB_ASSERT(Index < JobStorage->FreeJobIndices.Capacity);
    ak__job* Job = JobStorage->Jobs + Index;
    if(AK_Atomic_Load_U32_Relaxed(&Job->Generation) == Generation) {
        return Job;
    }
    return NULL;
}

static ak__job* AK_Job__Storage_Alloc(ak__job_storage* JobStorage) {
    /*Need to find a free job index atomically*/
    uint32_t FreeIndex = AK_Async_Stack_Index32_Pop(&JobStorage->FreeJobIndices);
    if(FreeIndex == AK_ASYNC_STACK_INDEX32_INVALID) {
        /*No more jobs avaiable*/
        AK_JOB_ASSERT(false);
        return NULL;
    }

    ak__job* Job = JobStorage->Jobs+FreeIndex;
    AK_JOB_ASSERT(Job->Index == FreeIndex); /*Invalid indices!*/
    return Job;  
}

static uint32_t AK_Job__Storage_Capacity(ak__job_storage* JobStorage) {
    return JobStorage->FreeJobIndices.Capacity;
}

static void AK_Job__Storage_Free(ak__job_storage* JobStorage, ak_job_id JobID, void* MallocUserData) {
    uint32_t Index = (uint32_t)(JobID);
    AK_JOB_ASSERT(Index < JobStorage->FreeJobIndices.Capacity); /*Overflow*/
    ak__job* Job = JobStorage->Jobs + Index;
    AK_JOB_ASSERT(!AK_Atomic_Load_U32_Relaxed(&Job->IsProcessing));
    AK_Job__Free_Data(Job, MallocUserData);
    uint32_t Generation = (uint32_t)(JobID >> 32);
    uint32_t NextGenerationIndex = Generation+1;
    if(NextGenerationIndex == 0) NextGenerationIndex = 1;
    AK_Atomic_Store_U32(&Job->Generation, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Async_Stack_Index32_Push(&JobStorage->FreeJobIndices, Index);
}

static void AK_Job__Init_Dependencies(ak__job_dependencies* Dependencies, uint32_t* FreeIndices, ak__job_dependency* DependenciesPtr, uint32_t DependencyCount) {
    
    AK_Async_Stack_Index32_Init_Raw(&Dependencies->FreeJobDependencies, FreeIndices, DependencyCount);
    AK_JOB_ASSERT(AK__Job_Validate_Ptr(DependenciesPtr));
    Dependencies->JobDependencies = DependenciesPtr;
    Dependencies->MaxDependencyCount = DependencyCount;
    
    uint32_t i;
    for(i = 0; i < DependencyCount; i++) {
        /*Add all the entries to the freelist since every job is free to begin with. 
        This can be done synchronously without worrying about the stack key and aba problem */
        AK_Async_Stack_Index32_Push_Sync(&Dependencies->FreeJobDependencies, i);
        Dependencies->JobDependencies[i].Index = i;
    }
}

static void AK_Job__Add_Dependency(ak__job_dependencies* Dependencies, ak__job* Job, ak__job* DependencyJob) {
    uint32_t FreeDependencyIndex = AK_Async_Stack_Index32_Pop(&Dependencies->FreeJobDependencies);
    if(FreeDependencyIndex == AK_ASYNC_STACK_INDEX32_INVALID) {
        AK_JOB_ASSERT(false);
        return;
    }

    ak__job_dependency* Dependency = Dependencies->JobDependencies + FreeDependencyIndex;
    AK_JOB_ASSERT(Dependency->Index == FreeDependencyIndex);
    
    Dependency->Job = DependencyJob;
    AK_Atomic_Increment_U32_Relaxed(&DependencyJob->PendingDependencies);

    for(;;) {
        ak__job_dependency* OldTop = (ak__job_dependency*)AK_Atomic_Load_Ptr_Relaxed(&Job->DependencyList);
        Dependency->Next = OldTop;
        if(AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(&Job->DependencyList, OldTop, Dependency, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return;
        }
    }
}

static ak__job_dependency_iter AK__Job_Dependency_Begin_Iter(ak__job* Job) {
    ak__job_dependency_iter Iter = {0};
    Iter.Dependency = (ak__job_dependency*)AK_Atomic_Load_Ptr_Relaxed(&Job->DependencyList);
    return Iter;
}

static bool AK__Job_Next_Dependency(ak__job_dependency_iter* Iter, ak__job_dependencies* Dependencies) {
    if(!Iter->Dependency) {
        return false;
    }

    ak__job_dependency* DependencyToDelete = Iter->Dependency;
    Iter->Dependency = Iter->Dependency->Next;
    ak__job* JobDependency = DependencyToDelete->Job;
    if(AK_Atomic_Decrement_U32_Relaxed(&JobDependency->PendingDependencies) == 0) {
        Iter->ProcessJob = true;
        Iter->Job = JobDependency;
    }
    DependencyToDelete->Next = NULL;
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Async_Stack_Index32_Push(&Dependencies->FreeJobDependencies, DependencyToDelete->Index);
    return true;
}

typedef struct ak__job_system_queue  ak__job_system_queue;

typedef struct {
    ak_job_system*         JobSystem;
    ak_thread              Thread;
} ak__job_system_thread;

struct ak__job_system_queue {
    ak_atomic_u32         BottomIndex;
    ak_atomic_u32         TopIndex;
    ak__job**             Queue;
    uint64_t              ThreadID;
    ak__job_system_queue* Next;
};

struct ak_job_system {
    /*System information*/
    void*  MallocUserData;
    ak_tls TLS;

    /*Job information*/
    ak__job_storage JobStorage;
    ak__job_dependencies Dependencies;
    ak_lw_semaphore JobSemaphore;

    /*Threads and queues*/
    ak_job_thread_callbacks ThreadCallbacks;
    ak_atomic_ptr           NonThreadRunnerQueueTopPtr; /*Link list of ak__job_system_queue queues that are not thread runners*/    
    ak_atomic_ptr           ThreadRunnerQueueTopPtr;  /*Link list of ak__job_system_queue queues that are thread runners*/
    uint32_t                ThreadCount;
    ak__job_system_thread*  Threads;
    ak_atomic_u32           IsRunning;
};

ak__job_system_queue* AK_Job_System__Create_Queue(ak_job_system* JobSystem, ak_atomic_ptr* TopQueuePtr) {
    size_t AllocationSize = sizeof(ak__job_system_queue)+(sizeof(ak__job*)*AK_Job__Storage_Capacity(&JobSystem->JobStorage));
    ak__job_system_queue* JobQueue = (ak__job_system_queue*)AK_Job__Malloc(AllocationSize, JobSystem->MallocUserData);
    AK_JOB_ASSERT(JobQueue);
    if(!JobQueue) return NULL;
    AK_JOB_MEMSET(JobQueue, 0, AllocationSize);
    JobQueue->Queue = (ak__job**)(JobQueue+1);

    /*Append to link list atomically*/
    for(;;) {
        ak__job_system_queue* OldTop = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(TopQueuePtr);
        JobQueue->Next = OldTop;
        if(AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(TopQueuePtr, OldTop, JobQueue, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return JobQueue;
        }
    }
}

static ak__job_system_queue* AK_Job_System__Get_Local_Queue(ak_job_system* JobSystem) {
    ak__job_system_queue* JobQueue = (ak__job_system_queue*)AK_TLS_Get(&JobSystem->TLS);
    if(!JobQueue) {
        uint64_t ThreadID = AK_Thread_Get_Current_ID();

        /*If we do not find a queue in our TLS we linear search for it by thread id*/

        ak__job_system_queue* Queue;
        for(Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->NonThreadRunnerQueueTopPtr);
            Queue; Queue = Queue->Next) {
            if(Queue->ThreadID == ThreadID) {
                JobQueue = Queue;
                break;
            }
        }

        for(Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->ThreadRunnerQueueTopPtr);
            Queue; Queue = Queue->Next) {
            if(Queue->ThreadID == ThreadID) {
                JobQueue = Queue;
                break;
            }
        }

        if(JobQueue) AK_TLS_Set(&JobSystem->TLS, JobQueue);
    }
    return JobQueue;
}

ak__job_system_queue* AK_Job_System__Get_Or_Create_Local_Queue(ak_job_system* JobSystem) {
    ak__job_system_queue* JobQueue = AK_Job_System__Get_Local_Queue(JobSystem);
    if(!JobQueue) {
        JobQueue = AK_Job_System__Create_Queue(JobSystem, &JobSystem->NonThreadRunnerQueueTopPtr);
        JobQueue->ThreadID = AK_Thread_Get_Current_ID();
        AK_TLS_Set(&JobSystem->TLS, JobQueue);
    }
    return JobQueue;
}

static ak__job* AK_Job_System__Steal_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue) {
    int32_t Top = (int32_t)AK_Atomic_Load_U32(&JobQueue->TopIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    /*Bottom needs to be read after top so an acquire barrier is sufficient (LoadLoad situation)*/
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32(&JobQueue->BottomIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);

    ak__job* Result = NULL;
    if(Top < Bottom) {
        Result = JobQueue->Queue[Top % AK_Job__Storage_Capacity(&JobSystem->JobStorage)];
        if(!AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&JobQueue->TopIndex, (uint32_t)Top, (uint32_t)(Top+1), AK_ATOMIC_MEMORY_ORDER_ACQ_REL, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            Result = NULL;
        }
    } 
    return Result;
}

static ak__job* AK_Job_System__Pop_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue) {
    int32_t Bottom = ((int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex))-1;

    AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, (uint32_t)Bottom);
    /*We need to make sure Top is read before bottom thus this is a StoreLoad situation
      and needs an explicit memory barrier to handle this case */
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();
    int32_t Top = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->TopIndex);

    ak__job* Result = NULL;
    if(Top <= Bottom) {
        Result = JobQueue->Queue[Bottom % AK_Job__Storage_Capacity(&JobSystem->JobStorage)];
        if(Top == Bottom) {
            if(!AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&JobQueue->TopIndex, (uint32_t)Top, (uint32_t)(Top+1), AK_ATOMIC_MEMORY_ORDER_ACQ_REL, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
                Result = NULL;
            }
            AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, (uint32_t)(Bottom+1));
        }
    } else {
        AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, (uint32_t)(Bottom+1));
    }
    return Result;
}

static void AK_Job_System__Push_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue, ak__job* Job) {
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex);
    int32_t Top = (int32_t)AK_Atomic_Load_U32(&JobQueue->TopIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(((int32_t)AK_Job__Storage_Capacity(&JobSystem->JobStorage)-1) < (Bottom-Top)) {
        AK_JOB_ASSERT(false);
    }
    JobQueue->Queue[Bottom % AK_Job__Storage_Capacity(&JobSystem->JobStorage)] = Job;
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();
    AK_Atomic_Store_U32(&JobQueue->BottomIndex, (uint32_t)(Bottom+1), AK_ATOMIC_MEMORY_ORDER_RELEASE);
}

static size_t AK_Job_System__Get_Queue_Size(ak__job_system_queue* JobQueue) {
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex); 
    int32_t Top = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->TopIndex); 
    return (size_t)(Bottom >= Top ? Bottom-Top : 0);
}

static ak__job_system_queue* AK_Job_System__Get_Largest_Queue(ak_job_system* JobSystem, ak__job_system_queue* CurrentQueue) {
    size_t BestSize = 0;
    ak__job_system_queue* BestQueue = NULL;

    ak__job_system_queue* Queue;
    for(Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->NonThreadRunnerQueueTopPtr);
        Queue; Queue = Queue->Next) {
        if(Queue != CurrentQueue) {
            size_t QueueSize = AK_Job_System__Get_Queue_Size(Queue); 
            if(QueueSize > BestSize) {
                BestSize = QueueSize;
                BestQueue = Queue;
            }
        }
    }

    if(BestQueue) return BestQueue;

    for(Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->ThreadRunnerQueueTopPtr);
        Queue; Queue = Queue->Next) {
        if(Queue != CurrentQueue) {
            size_t QueueSize = AK_Job_System__Get_Queue_Size(Queue); 
            if(QueueSize > BestSize) {
                BestSize = QueueSize;
                BestQueue = Queue;
            }
        }
    }

    return BestQueue;
}


static void AK_Job_System__Requeue_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue, ak__job* Job) {    
    AK_Job_System__Push_Job(JobSystem, JobQueue, Job);
    AK_LW_Semaphore_Increment(&JobSystem->JobSemaphore);
}

static void AK_Job_System__Add_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue, ak__job* Job) {
    AK_Atomic_Increment_U32(&Job->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(Job->ParentJob) {
        /*Only increment the parent once, and it should not be processing yet*/
        AK_Atomic_Increment_U32(&Job->ParentJob->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    }

    if(AK_Atomic_Compare_Exchange_Bool_U32(&Job->IsProcessing, false, true, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
        AK_Job_System__Requeue_Job(JobSystem, JobQueue, Job);
    } else {
        AK_JOB_ASSERT(false); /*Cannot submit an already submitted job!*/
    }
}

static void AK_Job_System__Finish_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue, ak__job* Job) {
    /*Pending jobs will increment everytime we submit a job. Parents will also get incremented
      when its child is submitted, however parents must be submitted later. To prevent the
      parent*/
    uint32_t IsProcessing = AK_Atomic_Load_U32_Relaxed(&Job->IsProcessing);
    if(AK_Atomic_Decrement_U32_Relaxed(&Job->PendingJobs) == 0 && IsProcessing) {
        

        ak__job_dependency_iter DependencyIter = AK__Job_Dependency_Begin_Iter(Job);
        while(AK__Job_Next_Dependency(&DependencyIter, &JobSystem->Dependencies)) {
            if(DependencyIter.ProcessJob) {
                AK_Job_System__Add_Job(JobSystem, JobQueue, DependencyIter.Job);
            }
        }

        if(Job->ParentJob) {
            AK_Job_System__Finish_Job(JobSystem, JobQueue, Job->ParentJob);
        }

        AK_Atomic_Store_Ptr(&Job->DependencyList, NULL, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        ak_job_id JobID = AK__Job_Make_ID(Job);

        AK_Atomic_Store_U32(&Job->IsProcessing, false, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        if(Job->JobInfoFlags & AK__JOB_INFO_FLAG_FREE_WHEN_DONE_BIT)
            AK_Job__Storage_Free(&JobSystem->JobStorage, JobID, JobSystem->MallocUserData);
    }
}

static void AK_Job_System__Process_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue, ak__job* Job) {    
    ak_job_id JobID = AK__Job_Make_ID(Job);

    if(Job->JobCallback) {
        void* UserData = 
            ((Job->JobInfoFlags & AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT) ? 
                AK_Job__Get_User_Data_Heap(Job->UserData) : 
                (void*)Job->UserData);
        ak_job_system_callback_func* JobCallback = (ak_job_system_callback_func*)Job->JobCallback;
        JobCallback(JobSystem, JobID, UserData);
    }
    
    AK_Job_System__Finish_Job(JobSystem, JobQueue, Job);
}


static ak__job* AK_Job_System__Get_Next_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue) {
    ak__job* Job = AK_Job_System__Pop_Job(JobSystem, JobQueue);
    if(!Job) {
        ak__job_system_queue* StolenJobQueue = AK_Job_System__Get_Largest_Queue(JobSystem, JobQueue);
        if(StolenJobQueue) {
            Job = AK_Job_System__Steal_Job(JobSystem, StolenJobQueue);
        }
    }
    return Job;   
}

static bool AK_Job_System__Process_Next_Job(ak_job_system* JobSystem, ak__job_system_queue* JobQueue) {
    ak__job* Job = AK_Job_System__Get_Next_Job(JobSystem, JobQueue);
    if(Job) {
        AK_Job_System__Process_Job(JobSystem, JobQueue, Job);
        return true;
    }
    return false;
}


static void AK_Job_System__Run(ak_job_system* JobSystem, ak__job_system_thread* JobThread, ak__job_system_queue* JobQueue) {
    ak_job_thread_callbacks* ThreadCallbacks = &JobSystem->ThreadCallbacks;

    while(AK_Atomic_Load_U32_Relaxed(&JobSystem->IsRunning)) {
        if(!AK_Job_System__Process_Next_Job(JobSystem, JobQueue)) {
            AK_LW_Semaphore_Decrement(&JobSystem->JobSemaphore);
        }

        if(ThreadCallbacks->JobThreadUpdate)
            ThreadCallbacks->JobThreadUpdate(&JobThread->Thread, ThreadCallbacks->UserData);
    }

    /*Cleanup remaining jobs*/
    while(AK_Job_System__Process_Next_Job(JobSystem, JobQueue)) {
        if(ThreadCallbacks->JobThreadUpdate)
            ThreadCallbacks->JobThreadUpdate(&JobThread->Thread, ThreadCallbacks->UserData);
    }
}

static void AK_Job_System__Main(ak__job_system_thread* JobThread) {
    ak_job_system* JobSystem = JobThread->JobSystem;
    ak_job_thread_callbacks* ThreadCallbacks = &JobSystem->ThreadCallbacks;
    ak__job_system_queue* JobQueue = AK_Job_System__Create_Queue(JobSystem, &JobSystem->ThreadRunnerQueueTopPtr);
    JobQueue->ThreadID = AK_Thread_Get_Current_ID();
    AK_TLS_Set(&JobSystem->TLS, JobQueue);

    if(ThreadCallbacks->JobThreadBegin)
        ThreadCallbacks->JobThreadBegin(&JobThread->Thread, ThreadCallbacks->UserData);
    
    if(ThreadCallbacks->JobThreadUpdate)
        ThreadCallbacks->JobThreadUpdate(&JobThread->Thread, ThreadCallbacks->UserData);

    AK_Job_System__Run(JobSystem, JobThread, JobQueue);

    if(ThreadCallbacks->JobThreadEnd)
        ThreadCallbacks->JobThreadEnd(&JobThread->Thread, ThreadCallbacks->UserData);
}

static AK_THREAD_CALLBACK_DEFINE(AK_Job_System__Thread_Callback) {
    AK_ATOMIC__UNREFERENCED_PARAMETER(Thread);
    ak__job_system_thread* JobThread = (ak__job_system_thread*)UserData;
    AK_Job_System__Main(JobThread);
    return 0;
}

AKATOMICDEF ak_job_system* AK_Job_System_Create(uint32_t MaxJobCount, uint32_t ThreadCount, uint32_t MaxDependencyCount, const ak_job_thread_callbacks* ThreadCallbacks, void* MallocUserData) {

    /*Get the allocation size. Want to avoid many small allocations so batch them together*/
    /*Data entries should also be 16 byte aligned, so if any entry is not aligned
      we will add some extra padding*/
    
    size_t JobSystemSize = AK_Job__Align_Pow2(sizeof(ak_job_system), AK_JOB__ALIGNMENT);
    size_t JobFreeIndicesSize = AK_Job__Align_Pow2(MaxJobCount*sizeof(uint32_t), AK_JOB__ALIGNMENT);
    size_t JobSize = AK_Job__Align_Pow2(MaxJobCount*sizeof(ak__job), AK_JOB__ALIGNMENT);
    size_t ThreadSize = AK_Job__Align_Pow2(ThreadCount*sizeof(ak__job_system_thread), AK_JOB__ALIGNMENT);
    size_t DependencyFreeIndicesSize = AK_Job__Align_Pow2(MaxDependencyCount*sizeof(uint32_t), AK_JOB__ALIGNMENT);
    size_t DependencySize = AK_Job__Align_Pow2(MaxDependencyCount*sizeof(ak__job_dependencies), AK_JOB__ALIGNMENT);

    size_t AllocationSize = JobSystemSize; /*Space for the job system*/ 
    AllocationSize += JobFreeIndicesSize; /*Space for the job free indices*/
    AllocationSize += JobSize; /*Space for the jobs*/
    AllocationSize += ThreadSize; /*Space for the threads*/
    AllocationSize += DependencyFreeIndicesSize; /*Space for the job free dependency indices*/
    AllocationSize += DependencySize; /*Space for the dependencies*/

    /*Allocate and clear all the memory*/
    ak_job_system* JobSystem = (ak_job_system*)AK_Job__Malloc(AllocationSize, MallocUserData);
    AK_JOB_ASSERT(JobSystem);

    if(!JobSystem) return NULL;
    AK_JOB_ASSERT(AK__Job_Validate_Ptr(JobSystem));

    AK_JOB_MEMSET(JobSystem, 0, AllocationSize);

    /*System information*/
    JobSystem->MallocUserData     = MallocUserData;
    AK_TLS_Create(&JobSystem->TLS);
    AK_LW_Semaphore_Create(&JobSystem->JobSemaphore, 0);

    /*Job information*/
    uint32_t* FreeJobIndices = AK__Job_Byte_Offset(JobSystem, JobSystemSize, uint32_t);
    ak__job* Jobs = AK__Job_Byte_Offset(FreeJobIndices, JobFreeIndicesSize, ak__job);
    AK_Job__Storage_Init(&JobSystem->JobStorage, FreeJobIndices, Jobs, MaxJobCount);

    JobSystem->ThreadCount = ThreadCount;
    JobSystem->Threads = AK__Job_Byte_Offset(Jobs, JobSize, ak__job_system_thread);
    AK_JOB_ASSERT(AK__Job_Validate_Ptr(JobSystem->Threads));

    if(MaxDependencyCount) {
        uint32_t* FreeDependencyIndices = AK__Job_Byte_Offset(JobSystem->Threads, ThreadSize, uint32_t);
        ak__job_dependency* JobDependencies = AK__Job_Byte_Offset(FreeDependencyIndices, DependencyFreeIndicesSize, ak__job_dependency); 
        AK_Job__Init_Dependencies(&JobSystem->Dependencies, FreeDependencyIndices, JobDependencies, MaxDependencyCount);
    }
    /*Thread information*/
    if(ThreadCallbacks) {
        JobSystem->ThreadCallbacks = *ThreadCallbacks;
    }

    AK_Atomic_Store_U32_Relaxed(&JobSystem->IsRunning, true);

    uint32_t i;
    for(i = 0; i < JobSystem->ThreadCount; i++) {
        ak__job_system_thread* Thread = JobSystem->Threads + i;
        Thread->JobSystem = JobSystem;
        if(!AK_Thread_Create(&Thread->Thread, AK_Job_System__Thread_Callback, Thread)) {
            /*todo: cleanup resources*/
            return NULL;
        }
    }

    /*Validate that our memory is correct*/
    return JobSystem;
}

AKATOMICDEF void AK_Job_System_Delete(ak_job_system* JobSystem) {
    /*Turn off the system*/    
    AK_Atomic_Store_U32_Relaxed(&JobSystem->IsRunning, false);
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Atomic_Compiler_Fence_Seq_Cst();

    /*Set all the threads status to IsRunning false before we increment the semaphore*/
    AK_LW_Semaphore_Add(&JobSystem->JobSemaphore, (int32_t)JobSystem->ThreadCount);

    /*Wait for the threads to finish and delete them*/
    uint32_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < JobSystem->ThreadCount; ThreadIndex++) {
        ak__job_system_thread* Thread = JobSystem->Threads + ThreadIndex;
        AK_Thread_Delete(&Thread->Thread);
    }

    /*Finally delete all the queue memory*/
    ak__job_system_queue* Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->NonThreadRunnerQueueTopPtr);
    while(Queue) {
        ak__job_system_queue* QueueToDelete = Queue;
        Queue = Queue->Next;
        AK_Job__Free(QueueToDelete, JobSystem->MallocUserData);
    }
    Queue = (ak__job_system_queue*)AK_Atomic_Load_Ptr_Relaxed(&JobSystem->ThreadRunnerQueueTopPtr);
    while(Queue) {
        ak__job_system_queue* QueueToDelete = Queue;
        Queue = Queue->Next;
        AK_Job__Free(QueueToDelete, JobSystem->MallocUserData);
    }
    AK_LW_Semaphore_Delete(&JobSystem->JobSemaphore);
    AK_TLS_Delete(&JobSystem->TLS);
    AK_Job__Free(JobSystem, JobSystem->MallocUserData);
}

AKATOMICDEF ak_job_id AK_Job_System_Alloc_Job(ak_job_system* JobSystem, ak_job_system_data JobData, ak_job_id ParentID, ak_job_flags Flags) {
    ak__job* Job = AK_Job__Storage_Alloc(&JobSystem->JobStorage);

    Job->JobCallback = (void*)JobData.JobCallback;
    AK_Job__Allocate_Data(Job, JobData.Data, JobData.DataByteSize, JobSystem->MallocUserData);
    Job->ParentJob = AK_Job__Storage_Get(&JobSystem->JobStorage, ParentID);

    if(Flags & AK_JOB_FLAG_FREE_WHEN_DONE_BIT) {
        Job->JobInfoFlags |= AK__JOB_INFO_FLAG_FREE_WHEN_DONE_BIT;
    } else {
        Job->JobInfoFlags &= ~AK__JOB_INFO_FLAG_FREE_WHEN_DONE_BIT;
    }


    AK_JOB_ASSERT(AK_Atomic_Load_U32(&Job->PendingDependencies, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0);
    AK_JOB_ASSERT(AK_Atomic_Load_Ptr(&Job->DependencyList, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == NULL);
    /*Pending jobs should be 0 for the job*/    
    AK_JOB_ASSERT(AK_Atomic_Load_U32(&Job->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0);
    AK_JOB_ASSERT(!AK_Atomic_Load_U32_Relaxed(&Job->IsProcessing));

    ak_job_id JobID = AK__Job_Make_ID(Job);
    if(Flags & AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT) {
        AK_Job_System_Add_Job(JobSystem, JobID);
    }
    return JobID;
}

AKATOMICDEF void AK_Job_System_Free_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    AK_Job__Storage_Free(&JobSystem->JobStorage, JobID, JobSystem->MallocUserData);
}

AKATOMICDEF ak_job_id AK_Job_System_Alloc_Empty_Job(ak_job_system* JobSystem, ak_job_flags Flags) {
    ak_job_system_data JobData = {0};
    return AK_Job_System_Alloc_Job(JobSystem, JobData, 0, Flags);
}

AKATOMICDEF void AK_Job_System_Add_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    ak__job_system_queue* JobQueue = AK_Job_System__Get_Or_Create_Local_Queue(JobSystem);
    ak__job* Job = AK_Job__Storage_Get(&JobSystem->JobStorage, JobID);
    if(Job) {
        AK_Job_System__Add_Job(JobSystem, JobQueue, Job);
    }
}

AKATOMICDEF void AK_Job_System_Wait_For_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    ak__job* Job = AK_Job__Storage_Get(&JobSystem->JobStorage, JobID);
    while(Job && AK_Atomic_Load_U32_Relaxed(&Job->IsProcessing)) {
        ak__job_system_queue* JobQueue = AK_Job_System__Get_Or_Create_Local_Queue(JobSystem);
        AK_Job_System__Process_Next_Job(JobSystem, JobQueue);
    }
}

AKATOMICDEF void AK_Job_System_Add_Dependency(ak_job_system* JobSystem, ak_job_id JobID, ak_job_id DependencyJobID) {
    ak__job* Job = AK_Job__Storage_Get(&JobSystem->JobStorage, JobID);
    ak__job* DependencyJob = AK_Job__Storage_Get(&JobSystem->JobStorage, DependencyJobID);
    if(!Job || !DependencyJob) {
        AK_JOB_ASSERT(false);
        return;
    }

    AK_Job__Add_Dependency(&JobSystem->Dependencies, Job, DependencyJob);
}

#endif

#ifdef __cplusplus
}
#endif

#endif