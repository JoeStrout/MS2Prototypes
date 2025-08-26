// MS2Proto2 VM Fibonacci demo
// Main program that demonstrates the VM with recursive Fibonacci computation

#include "../include/vm/vm.h"
#include "../include/types/gc.h"
#include "../include/types/nanbox.h"
#include <stdio.h>
#include <stdlib.h>

// Helper to build Fibonacci prototype
static Proto *make_fib_proto(void) {
	// Registers: r0 = n (arg), r1..r4 temps
	// Pseudocode:
	//   if (n < 2) return n;
	//   r1 = n-1; r1 = fib(r1);
	//   r3 = n-2; r3 = fib(r3);
	//   r0 = r1 + r3; return r0;

	Proto *p = (Proto*)calloc(1, sizeof(Proto));
	p->max_regs = 5;               // r0..r4
	p->code_len = 13;              // filled below
	p->code = (uint32_t*)calloc(p->code_len, sizeof(uint32_t));

	int k = 0;  // (current code position)
	p->code[k++] = INS_AB(LOADK, 1, 2);                // r1 = 2
	int iflt_at = k;                                   // if (r0 < r1) goto base_case
	p->code[k++] = INS_ABC(IFLT,  0, 1, 0);            // patch C (offset) later

	// Hand-optimized version:
	p->code[k++] = INS_AB( LOADK, 1, 1);               // r1 = 1
	p->code[k++] = INS_ABC(SUB,   1, 0, 1);            // r1 = n - r1
	p->code[k++] = INS_ABC(CALLF, 1, 1, 0);            // r1 = fib(r1)

	p->code[k++] = INS_AB( LOADK, 2, 2);               // r2 = 2
	p->code[k++] = INS_ABC(SUB,   2, 0, 2);            // r2 = n - r2
	p->code[k++] = INS_ABC(CALLF, 2, 1, 0);            // r2 = fib(r2)

	p->code[k++] = INS_ABC(ADD,   0, 1, 2);            // r0 = r1 + r2
	p->code[k++] = INS(RETURN);                        // return r0

	// Less optimized version:
// 	p->code[k++] = INS_ABC(MOVE,  1, 0, 0);            // r1 = n
// 	p->code[k++] = INS_AB( LOADK, 2, 1);               // r2 = 1
// 	p->code[k++] = INS_ABC(SUB,   1, 1, 2);            // r1 = r1 - r2
// 	p->code[k++] = INS_ABC(CALLF, 1, 1, 0);            // r1 = fib(r1)
// 
// 	p->code[k++] = INS_ABC(MOVE,  3, 0, 0);            // r3 = n
// 	p->code[k++] = INS_AB( LOADK, 4, 2);               // r4 = 2
// 	p->code[k++] = INS_ABC(SUB,   3, 3, 4);            // r3 = r3 - r4
// 	p->code[k++] = INS_ABC(CALLF, 3, 1, 0);            // r3 = fib(r3)
// 
// 	p->code[k++] = INS_ABC(ADD,   0, 1, 3);            // r0 = r1 + r3
// 	p->code[k++] = INS_ABC(RETURN,0, 1, 0);            // return r0

	int base_case_pc = k;                              // label target
	p->code[k++] = INS(RETURN);                        // return n

	// Patch IFLT offset: from NEXT pc (iflt_at+1) to base_case_pc
	int8_t off = (int8_t)(base_case_pc - (iflt_at + 1));
	p->code[iflt_at] = INS_ABC(IFLT, 0, 1, OFF8(off));

	return p;
}

static Proto *make_main_proto(int nval) {
	Proto *p = (Proto*)calloc(1, sizeof(Proto));
	p->max_regs = 4;  // just r0 is used, but keep a little headroom
	p->code_len = 3;
	p->code = (uint32_t*)calloc(p->code_len, sizeof(uint32_t));

	int k = 0;
	p->code[k++] = INS_AB(LOADK, 0, nval);               // r0 = n
	p->code[k++] = INS_ABC(CALLF, 0, 1, 0);              // r0 = fib(r0)
	p->code[k++] = INS_ABC(RETURN,0, 1, 0);             // return r0

	return p;
}

int main(void) {
	// Initialize garbage collector
	gc_init();
	
	if (vm_uses_goto()) {
		printf("VM using computed goto\n");
	} else {
		printf("VM using portable switch\n");
	}

	VM vm; vm_init(&vm, /*stack_slots*/ 4096, /*call_slots*/ 1024);

	int n = 30;	// Fibonacci number to compute
	Proto *fib  = make_fib_proto();
	Proto *mainp = make_main_proto(n);

	vm.funcs[0] = fib; // CALLF C=0 targets fib

	Value result = vm_exec(&vm, mainp);
	
	if (is_int(result)) {
		printf("fib(%d) = %d\n", n, as_int(result));
	} else if (is_double(result)) {
		printf("fib(%d) = %g\n", n, as_double(result));
	} else {
		printf("fib(%d) = ", n);
		debug_print_value(result);
		printf("\n");
	}

	// cleanup
	free(fib->code); free(fib);
	free(mainp->code); free(mainp);
	vm_free(&vm);
	
	// Cleanup garbage collector
	gc_shutdown();
	return 0;
}