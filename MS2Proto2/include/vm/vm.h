#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../types/nanbox.h"

// Opcode list via X-macro
#define VM_OPCODES(X) \
	X(MOVE)             \
	X(LOADK)            \
	X(ADD)              \
	X(SUB)              \
	X(IFLT)             \
	X(JMP)              \
	X(CALLF)            \
	X(RETURN)

#define X(OP) op_##OP,
enum { VM_OPCODES(X) op__COUNT };
#undef X

// Instruction field helpers
#define OP(i)   ((uint8_t)((i) >> 24))
#define A(i)    ((uint8_t)((i) >> 16))
#define B(i)    ((uint8_t)((i) >>  8))
#define C(i)    ((uint8_t)((i)      ))
#define BC(i)   ((int16_t)((i) & 0xFFFF))

// Encode helpers
#define INS(OPC)               ((uint32_t)((op_##OPC) << 24))
#define INS_ABC(OPC,A_,B_,C_)  ((uint32_t)(((op_##OPC) << 24) | ((A_) << 16) | ((B_) << 8) | (C_)))
#define INS_AB(OPC,A_,BC_)     ((uint32_t)(((op_##OPC) << 24) | ((A_) << 16) | ((uint16_t)(BC_))))
#define OFF8(d)  ((uint8_t)(int8_t)(d))

// Function prototype
typedef struct {
	uint32_t *code;              // instruction stream
	int       code_len;
	uint16_t  max_regs;          // frame reservation size
} Proto;

// Call stack frame (return info)
typedef struct {
	uint32_t *return_pc;         // where to continue in caller
	Value    *return_base;       // caller's base pointer
} CallInfo;

// VM state
typedef struct {
	Value    *stack; size_t stack_size; Value *top;
	CallInfo *cstack; size_t cstack_size; CallInfo *ci; // ci points to next free slot
	Proto    *funcs[256];        // functions addressed by CALLF C-field
} VM;

// Public VM API
void vm_init(VM *vm, size_t stack_slots, size_t call_slots);
void vm_free(VM *vm);
Value vm_exec(VM *vm, Proto *entry, unsigned int max_cycles);
bool vm_uses_goto(void);

#endif // VM_H