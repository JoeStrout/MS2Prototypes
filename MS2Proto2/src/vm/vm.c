// VM with NaN-boxed values and computed-goto dispatch.
// VM implementation

#include "../../include/vm/vm.h"
#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Feature detection (override with -DVM_USE_COMPUTED_GOTO=0/1)
#ifndef VM_USE_COMPUTED_GOTO
/*
 * Auto-detect: enable computed-goto for any GCC-like compiler (GCC or Clang)
 * when not in strict ANSI mode. Clang defines __GNUC__ for compatibility, so
 * this covers Apple Clang as well. If you compile with -std=c11 (strict), this
 * will resolve to 0 unless you pass -DVM_USE_COMPUTED_GOTO=1 or use -std=gnu11.
 */
#  if (defined(__GNUC__) || defined(__clang__)) && !defined(__STRICT_ANSI__)
#    define VM_USE_COMPUTED_GOTO 1
#  else
#    define VM_USE_COMPUTED_GOTO 0
#  endif
#endif

// Dispatch macros (computed-goto OR portable switch)
#if VM_USE_COMPUTED_GOTO
	// Computed-goto: build a label table inside the function.
	// Important: include commas between entries!
	#define VM_LABEL_ADDR(OP) &&L_##OP
	#define VM_LABEL_LIST(OP) VM_LABEL_ADDR(OP),

	#define VM_DISPATCH_BEGIN()                                                     \
		static void* const vm_labels[op__COUNT] = { VM_OPCODES(VM_LABEL_LIST) };      \
		vm_dispatch_next: \
		cycle_count++; \
		if (max_cycles > 0 && cycle_count > max_cycles) { \
			fprintf(stderr, "VM: Hit cycle limit of %u\n", max_cycles); \
			return make_null(); \
		} \
		ins = *pc++; \
		if (debug) { \
			printf("PC: %ld, Cycle: %u, Ins: 0x%08x, Op: %u\n", \
				   (long)(pc - entry->code - 1), cycle_count, ins, OP(ins)); \
		} \
		goto *vm_labels[OP(ins)];

	#define VM_CASE(OP)     L_##OP:
	#define VM_NEXT()       goto vm_dispatch_next
	#define VM_DISPATCH_END() /* nothing */
#else
	#define VM_DISPATCH_BEGIN() for (;;) { \
		cycle_count++; \
		if (max_cycles > 0 && cycle_count > max_cycles) { \
			fprintf(stderr, "VM: Hit cycle limit of %u\n", max_cycles); \
			return make_null(); \
		} \
		ins = *pc++; \
		if (debug) { \
			printf("PC: %ld, Cycle: %u, Ins: 0x%08x, Op: %u\n", \
				   (long)(pc - entry->code - 1), cycle_count, ins, OP(ins)); \
		} \
		switch (OP(ins)) {
	#define VM_CASE(OP)        case op_##OP:
	#define VM_NEXT()          continue;
	#define VM_DISPATCH_END()  default: fprintf(stderr, "bad opcode %u\n", OP(ins)); exit(2); } }
#endif

void vm_init(VM *vm, size_t stack_slots, size_t call_slots) {
	vm->stack = (Value*)calloc(stack_slots, sizeof(Value));
	vm->stack_size = stack_slots; vm->top = vm->stack;
	vm->cstack = (CallInfo*)calloc(call_slots, sizeof(CallInfo));
	vm->cstack_size = call_slots; vm->ci = vm->cstack; // empty
	memset(vm->funcs, 0, sizeof(vm->funcs));
	
	// Initialize all stack values to nil
	for (size_t i = 0; i < stack_slots; i++) {
		vm->stack[i] = make_null();
	}
}

void vm_free(VM *vm) {
	free(vm->stack); free(vm->cstack);
}

bool vm_uses_goto(void) {
#if VM_USE_COMPUTED_GOTO
	return true;
#else
	return false;
#endif
}

static void ensure_frame(VM *vm, Value *base, uint16_t need) {
	(void)vm; (void)base; (void)need; // stack is pre-allocated large enough in this demo
}

Value vm_exec(VM *vm, Proto *entry, unsigned int max_cycles) {
	// Current frame state (kept in locals for speed)
	Value    *base = vm->stack;                   // entry executes at stack base
	uint32_t *pc   = entry->code;                 // start at entry code
	uint32_t *entry_code = entry->code;           // remember to avoid unused warning
	(void)entry_code;
	(void)ensure_frame;                           // silence if inlined away

	ensure_frame(vm, base, entry->max_regs);

	uint32_t ins;
	unsigned int cycle_count = 0;
	bool debug = false; // Set to true for debug output

	VM_DISPATCH_BEGIN();

	VM_CASE(MOVE) {
		// NOTE: we are assuming here that we never need more than 255 registers,
		// or if we do, we switch to some other opcode.  A more complete VM might
		// need to keep a "window" for which set of registers we're working with.
		base[A(ins)] = base[B(ins)];
		VM_NEXT();
	}

	VM_CASE(LOADK) {
		// BC is signed 16-bit immediate
		base[A(ins)] = make_int(BC(ins));
		VM_NEXT();
	}

	VM_CASE(LOADN) {
		// Load from constants table: R[A] = constants[B]
		uint16_t const_idx = BC(ins);
		if (const_idx >= entry->const_len) {
			fprintf(stderr, "LOADN: invalid constant index %u\n", const_idx);
			exit(2);
		}
		base[A(ins)] = entry->constants[const_idx];
		VM_NEXT();
	}

	VM_CASE(ADD) {
		base[A(ins)] = value_add(base[B(ins)], base[C(ins)]);
		VM_NEXT();
	}

	VM_CASE(SUB) {
		base[A(ins)] = value_sub(base[B(ins)], base[C(ins)]);
		VM_NEXT();
	}

	VM_CASE(MULT) {
		base[A(ins)] = value_mult(base[B(ins)], base[C(ins)]);
		VM_NEXT();
	}

	VM_CASE(DIV) {
		base[A(ins)] = value_div(base[B(ins)], base[C(ins)]);
		VM_NEXT();
	}

	VM_CASE(IFLT) {
		// if base[A] < base[B], jump by signed 8-bit C from NEXT pc
		// (IFLT keeps 8-bit offset since it needs both A and B for comparison)
		int8_t off = (int8_t)C(ins);
		if (value_lt(base[A(ins)], base[B(ins)])) pc += off;
		VM_NEXT();
	}

	VM_CASE(IFEQ) {
		int8_t off = (int8_t)C(ins);
		if (value_equal(base[A(ins)], base[B(ins)])) pc += off;
		VM_NEXT();
	}

	VM_CASE(IFLE) {
		// if base[A] < base[B], jump by signed 8-bit C from NEXT pc
		// (IFLT keeps 8-bit offset since it needs both A and B for comparison)
		int8_t off = (int8_t)C(ins);
		if (value_lt(base[A(ins)], base[B(ins)]) && value_equal(base[A(ins)], base[B(ins)])) pc += off;
		VM_NEXT();
	}

	VM_CASE(IFNE) {
		// if base[A] < base[B], jump by signed 8-bit C from NEXT pc
		// (IFLT keeps 8-bit offset since it needs both A and B for comparison)
		int8_t off = (int8_t)C(ins);
		if (!value_equal(base[A(ins)], base[B(ins)])) pc += off;
		VM_NEXT();
	}

	VM_CASE(JMP) {
		int16_t off = BC(ins);
		pc += off;
		VM_NEXT();
	}

	VM_CASE(CALLF) {
		// A: arg window start (callee executes with base = base + A)
		// B: nargs (ignored in this minimal VM, but here for shape)
		// C: function index (0..255)
		(void)B(ins);
		Proto *callee = vm->funcs[C(ins)];
		if (!callee) { fprintf(stderr, "CALLF to null func %u\n", (unsigned)C(ins)); exit(3); }

		// Push return info
		if ((size_t)(vm->ci - vm->cstack) >= vm->cstack_size) { fprintf(stderr, "call stack overflow\n"); exit(4); }
		vm->ci->return_pc   = pc;
		vm->ci->return_base = base;
		vm->ci++;

		// Switch to callee frame: base slides to argument window
		base = base + A(ins);
		pc   = callee->code;

		ensure_frame(vm, base, callee->max_regs);
		VM_NEXT();
	}

	VM_CASE(RETURN) {
		// Return value convention: value is in base[A]
		// Since callee base points into caller's frame (at arg window),
		// the result is already in-place for the caller. We just pop.
		(void)ins; // A/B/C unused here in minimal runner
		if (vm->ci == vm->cstack) {
			// Returning from entry: produce the final result from base[0]
			return base[0];
		}
		// Pop
		vm->ci--;
		pc   = vm->ci->return_pc;
		base = vm->ci->return_base;
		VM_NEXT();
	}

	VM_DISPATCH_END();

	// Unreachable
	return make_null();
}