#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../vm/vm.h"
#include "../types/nanbox.h"

// Maximum number of labels, references, and functions supported
#define MAX_LABELS 256
#define MAX_REFS 256
#define MAX_FUNCTIONS 16

// Label table entry
typedef struct {
    char name[64];
    int32_t address;
    bool defined;
} Label;

// Unresolved reference
typedef struct {
    char label_name[64];
    int32_t instruction_addr;
    bool is_jump;  // true for JMP, false for IFLT
} Reference;

// Function definition
typedef struct {
    char name[64];
    uint32_t *code;
    size_t code_len;
    size_t code_capacity;
    Label labels[MAX_LABELS];
    size_t label_count;
    Reference refs[MAX_REFS];
    size_t ref_count;
    Value *constants;
    size_t const_len;
    size_t const_capacity;
    uint16_t max_regs;
    bool is_main;
} Function;

// Assembler state
typedef struct {
    Function functions[MAX_FUNCTIONS];
    size_t function_count;
    Function *current_function;  // Currently being assembled
    int32_t current_address;     // Current instruction address within function
} Assembler;

// Public API
void asm_init(Assembler *asm);
void asm_free(Assembler *asm);

// Function management
bool asm_start_function(Assembler *asm, const char *name, bool is_main);
bool asm_end_function(Assembler *asm);

// Instruction assembly
bool asm_instruction(Assembler *asm, const char *line);
bool asm_label(Assembler *asm, const char *name);
bool asm_resolve_labels(Assembler *asm);

// Code generation
Function *asm_find_function(Assembler *asm, const char *name);
Function *asm_get_main_function(Assembler *asm);

// Helper functions
int32_t asm_find_label(Function *func, const char *name);
bool asm_add_label(Function *func, const char *name, int32_t address);
bool asm_add_reference(Function *func, const char *label_name, int32_t addr, bool is_jump);

// Constants table management
int asm_add_constant(Function *func, Value value);
int asm_find_constant(Function *func, Value value);

#endif // ASSEMBLER_H