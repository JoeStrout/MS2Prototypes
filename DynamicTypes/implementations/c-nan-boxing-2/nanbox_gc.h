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
#define GC_PROTECT(val_ptr) gc_protect_value(val_ptr)
#define GC_UNPROTECT() gc_unprotect_value()

// Scope-based protection macros
#define GC_PUSH_SCOPE() gc_push_scope()
#define GC_POP_SCOPE() gc_pop_scope()

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