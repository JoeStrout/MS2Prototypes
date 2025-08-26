// Tiny VM Fibonacci demo
// Main program that demonstrates the VM with recursive Fibonacci computation

#include "tiny_vm.h"
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

	int k = 0;
	p->code[k++] = INS_ABC(LOADK, 1, 2, 0);            // r1 = 2
	int iflt_at = k;                                    // if (r0 < r1) goto base_case
	p->code[k++] = INS_ABC(IFLT, 0, 1, 0);              // patch later

	p->code[k++] = INS_ABC(MOVE, 1, 0, 0);              // r1 = n
	p->code[k++] = INS_ABC(LOADK, 2, 1, 0);             // r2 = 1
	p->code[k++] = INS_ABC(SUB,  1, 1, 2);              // r1 = r1 - r2
	p->code[k++] = INS_ABC(CALLF,1, 1, 0);              // r1 = fib(r1)

	p->code[k++] = INS_ABC(MOVE, 3, 0, 0);              // r3 = n
	p->code[k++] = INS_ABC(LOADK, 4, 2, 0);             // r4 = 2
	p->code[k++] = INS_ABC(SUB,  3, 3, 4);              // r3 = r3 - r4
	p->code[k++] = INS_ABC(CALLF,3, 1, 0);              // r3 = fib(r3)

	p->code[k++] = INS_ABC(ADD,  0, 1, 3);              // r0 = r1 + r3
	p->code[k++] = INS_ABC(RETURN,0, 1, 0);             // return r0

	int base_case_pc = k;                                // label target
	p->code[k++] = INS_ABC(RETURN,0, 1, 0);             // return n

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
	p->code[k++] = INS_ABC(LOADK, 0, (uint8_t)nval, 0); // r0 = n
	p->code[k++] = INS_ABC(CALLF, 0, 1, 0);             // r0 = fib(r0)
	p->code[k++] = INS_ABC(RETURN,0, 1, 0);             // return r0

	return p;
}

int main(void) {
	if (vm_uses_goto()) {
		printf("VM using computed goto\n");
	} else {
		printf("VM using portable switch\n");
	}

	VM vm; vm_init(&vm, /*stack_slots*/ 4096, /*call_slots*/ 1024);

	int n = 40;	// Fibonacci number to compute
	Proto *fib  = make_fib_proto();
	Proto *mainp = make_main_proto(n);

	vm.funcs[0] = fib; // CALLF C=0 targets fib

	int result = vm_exec(&vm, mainp);
	printf("fib(%d) = %d\n", n, result);

	// cleanup
	free(fib->code); free(fib);
	free(mainp->code); free(mainp);
	vm_free(&vm);
	return 0;
}