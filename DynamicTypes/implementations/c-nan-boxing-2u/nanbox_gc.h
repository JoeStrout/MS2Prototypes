#ifndef NANBOX_GC_H
#define NANBOX_GC_H

// This header combines nanbox and gc functionality properly
// Core NaN-boxing types and operations are defined in nanbox.h

#include "nanbox.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

// #define GC_DEBUG 1
// #define GC_AGGRESSIVE 1  // Collect on every allocation (for testing)

// GC Object header - minimal overhead
typedef struct GCObject {
    struct GCObject* next;  // Linked list of all objects
    bool marked;           // Mark bit for GC
    size_t size;          // Size for sweep phase
} GCObject;

// Scope management for RAII-style protection
typedef struct GCScope {
    int start_index;      // Where this scope starts in the root stack
} GCScope;

// Root set management - shadow stack of pointers to local Values
typedef struct GCRootSet {
    Value** roots;        // Array of pointers to Values (shadow stack)
    int count;
    int capacity;
} GCRootSet;

// GC state
typedef struct GC {
    GCObject* all_objects;    // Linked list of all allocated objects
    GCRootSet root_set;       // Stack of root values
    GCScope scope_stack[64];  // Stack of scopes for RAII-style protection
    int scope_count;          // Number of active scopes
    size_t bytes_allocated;   // Total allocated memory
    size_t gc_threshold;      // Trigger collection when exceeded
    int disable_count;        // Counter for nested disable/enable calls
} GC;

// Global GC instance
extern GC gc;

// GC lifecycle functions
void gc_init(void);
void gc_shutdown(void);
void gc_collect(void);

// Root set management (shadow stack approach)
void gc_protect_value(Value* val_ptr);
void gc_unprotect_value(void);

// Scope management
void gc_push_scope(void);
void gc_pop_scope(void);

// Critical section management
void gc_disable(void);
void gc_enable(void);

// Object allocation
void* gc_allocate(size_t size);

// Mark functions for different object types
void gc_mark_value(Value v);
void gc_mark_string(String* str);
void gc_mark_list(List* list);

// Utility macros for root management
#define GC_PROTECT(val_ptr) do { (void)sizeof(_gc_scope_token_); gc_protect_value(val_ptr); } while (0)
#define GC_UNPROTECT() gc_unprotect_value()

// Scope-based protection macros
#define GC_PUSH_SCOPE() bool _gc_scope_token_ = (gc_push_scope(), true)
#define GC_POP_SCOPE() do { (void)sizeof(_gc_scope_token_); gc_pop_scope(); } while (0)

// Nasty C macro trickery all to give us a GC_LOCALS macro that lets
// us declare and protect up to 16 local variables at once:
//static inline void gc_protect_many(size_t n, Value **ptrs) {
//	for (size_t i = 0; i < n; i++) {
//		GC_PROTECT(ptrs[i]);
//	}
//}

#define FE_1(F,X) F(X)
#define FE_2(F,X,...) F(X), FE_1(F,__VA_ARGS__)
#define FE_3(F,X,...) F(X), FE_2(F,__VA_ARGS__)
#define FE_4(F,X,...) F(X), FE_3(F,__VA_ARGS__)
#define FE_5(F,X,...) F(X), FE_4(F,__VA_ARGS__)
#define FE_6(F,X,...) F(X), FE_5(F,__VA_ARGS__)
#define FE_7(F,X,...) F(X), FE_6(F,__VA_ARGS__)
#define FE_8(F,X,...) F(X), FE_7(F,__VA_ARGS__)
#define FE_9(F,X,...) F(X), FE_8(F,__VA_ARGS__)
#define FE_10(F,X,...) F(X), FE_9(F,__VA_ARGS__)
#define FE_11(F,X,...) F(X), FE_10(F,__VA_ARGS__)
#define FE_12(F,X,...) F(X), FE_11(F,__VA_ARGS__)
#define FE_13(F,X,...) F(X), FE_12(F,__VA_ARGS__)
#define FE_14(F,X,...) F(X), FE_13(F,__VA_ARGS__)
#define FE_15(F,X,...) F(X), FE_14(F,__VA_ARGS__)
#define FE_16(F,X,...) F(X), FE_15(F,__VA_ARGS__)

#define GET_FE_MACRO( \
	 _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME

#define FOR_EACH(F, ...) \
	 GET_FE_MACRO(__VA_ARGS__, \
	   FE_16,FE_15,FE_14,FE_13,FE_12,FE_11,FE_10,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1) \
	   (F,__VA_ARGS__)

#define ADDR(x) &(x)

#define GC_LOCALS(...) \
  Value __VA_ARGS__; \
  do { \
	Value * _p[] = { FOR_EACH(ADDR, __VA_ARGS__) }; \
	for (size_t _i = 0; _i < sizeof _p / sizeof *_p; ++_i) GC_PROTECT(_p[_i]); \
  } while (0)



// SHADOW STACK PROTECTION STANDARD:
// 1. Call GC_PUSH_SCOPE() at start of function
// 2. GC_PROTECT(&local_var) for all local Value variables (pass pointer!)
// 3. Call GC_POP_SCOPE() before return
// 4. Collection only happens at safe points (allocation, explicit gc_collect)
// 5. When reassigning protected values, the shadow stack automatically tracks them
//
// Example usage:
//   void my_function() {
//       GC_PUSH_SCOPE();
//       Value str = make_nil();
//       Value list = make_nil();
//       GC_PROTECT(&str);    // Protect pointer to str
//       GC_PROTECT(&list);   // Protect pointer to list
//       
//       str = make_string("hello");  // Reassignment is fine
//       list = make_list(10);        // Shadow stack sees new values
//       
//       GC_POP_SCOPE();
//   }

#endif
