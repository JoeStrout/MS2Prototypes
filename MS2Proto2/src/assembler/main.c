#include "../../include/assembler/assembler.h"
#include "../../include/vm/vm.h"
#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog_name) {
    printf("Usage: %s [input.asm]\n", prog_name);
    printf("  Assembles VM assembly code into binary format\n");
    printf("  If no input file is specified, reads from stdin.\n");
}

static bool process_line(Assembler *asm, char *line) {
    // Trim whitespace
    char *start = line;
    while (*start && (*start == ' ' || *start == '\t')) start++;
    if (*start == '\0' || *start == '#' || *start == ';') {
        return true; // empty line or comment
    }
    
    // Remove inline comments
    char *comment = strchr(start, '#');
    if (comment) *comment = '\0';
    comment = strchr(start, ';');
    if (comment) *comment = '\0';
    
    // Trim trailing whitespace after comment removal
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) *end-- = '\0';
    
    // Check for function definition (@name:)
    if (start[0] == '@') {
        char *colon = strchr(start, ':');
        if (colon) {
            *colon = '\0';
            char *func_name = start + 1; // Skip '@'
            
            // End previous function if any
            if (asm->current_function) {
                asm_end_function(asm);
            }
            
            // Start new function
            bool is_main = (strcmp(func_name, "main") == 0);
            if (!asm_start_function(asm, func_name, is_main)) {
                fprintf(stderr, "Error: Failed to start function '%s'\n", func_name);
                return false;
            }
            return true;
        }
    }
    
    // Check for label definition (ends with colon)
    char *colon = strchr(start, ':');
    if (colon) {
        *colon = '\0';
        char *label_name = start;
        // Trim label name
        char *end = label_name + strlen(label_name) - 1;
        while (end > label_name && (*end == ' ' || *end == '\t')) *end-- = '\0';
        
        if (!asm_label(asm, label_name)) {
            fprintf(stderr, "Error: Failed to add label '%s'\n", label_name);
            return false;
        }
        
        // Process remainder of line after colon
        char *remainder = colon + 1;
        while (*remainder && (*remainder == ' ' || *remainder == '\t')) remainder++;
        if (*remainder && *remainder != '#' && *remainder != ';') {
            return asm_instruction(asm, remainder);
        }
        return true;
    }
    
    // Regular instruction
    return asm_instruction(asm, start);
}

static bool assemble_file(FILE *file, Assembler *asm) {
    char line[256];
    int line_num = 0;
    bool success = true;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove newline
        char *nl = strchr(line, '\r');
        if (nl){
		    *nl = '\0';
	    }
	    else {
		    nl = strchr(line, '\n');
	        if (nl) *nl = '\0';
	    }
        
        if (!process_line(asm, line)) {
            fprintf(stderr, "Error on line %d: %s\n", line_num, line);
            success = false;
            break;
        }
    }
    
    fclose(file);
    
    // End the last function if any
    if (asm->current_function) {
        asm_end_function(asm);
    }
    
    if (success) {
        success = asm_resolve_labels(asm);
    }
    
    return success;
}

static Proto *create_proto_from_function(Function *func) {
    Proto *proto = (Proto*)calloc(1, sizeof(Proto));
    proto->code = (uint32_t*)malloc(func->code_len * sizeof(uint32_t));
    memcpy(proto->code, func->code, func->code_len * sizeof(uint32_t));
    proto->code_len = (int)func->code_len;
    proto->max_regs = func->max_regs;
    
    // Copy constants table
    proto->const_len = (int)func->const_len;
    if (func->const_len > 0) {
        proto->constants = (Value*)malloc(func->const_len * sizeof(Value));
        memcpy(proto->constants, func->constants, func->const_len * sizeof(Value));
    } else {
        proto->constants = NULL;
    }
    
    return proto;
}

static void execute_code(Assembler *asm) {
    if (asm->function_count == 0) {
        printf("No functions to execute\n");
        return;
    }
    
    Function *main_func = asm_get_main_function(asm);
    if (!main_func) {
        printf("No main function found\n");
        return;
    }
    
    // Initialize VM and GC
    gc_init();
    VM vm;
    vm_init(&vm, 4096, 1024);
    
    // Create protos for all functions and register them in the VM
    Proto *protos[MAX_FUNCTIONS] = {0};
    int func_index = 0;
    
    for (size_t i = 0; i < asm->function_count; i++) {
        Function *func = &asm->functions[i];
        if (!func->is_main) {  // Non-main functions get registered for CALLF
            protos[func_index] = create_proto_from_function(func);
            vm.funcs[func_index] = protos[func_index];
            printf("Registered function '%s' at index %d\n", func->name, func_index);
            func_index++;
        }
    }
    
    // Create proto for main function
    Proto *main_proto = create_proto_from_function(main_func);
    
    printf("Executing main function (%zu instructions)...\n", main_func->code_len);
    
    Value result = vm_exec(&vm, main_proto, 0); // No cycle limit for main execution
    
    if (is_int(result)) {
        printf("Result: %d\n", as_int(result));
    } else if (is_double(result)) {
        printf("Result: %g\n", as_double(result));
    } else {
        printf("Result: ");
        debug_print_value(result);
        printf("\n");
    }
    
    // Cleanup
    for (int i = 0; i < func_index; i++) {
        if (protos[i]) {
            free(protos[i]->code);
            free(protos[i]->constants);
            free(protos[i]);
        }
    }
    free(main_proto->code);
    free(main_proto->constants);
    free(main_proto);
    vm_free(&vm);
    gc_shutdown();
}

int main(int argc, char **argv) {
    if (argc > 2 || (argc == 2 && argv[1][0] == '-')) {
        print_usage(argv[0]);
        return 1;
    }
    
    FILE *input_stream = stdin;
    if (argc == 2) {
	    input_stream = fopen(argv[1], "r");
        if (!input_stream) {
			fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
            return 1;
        }
	}
    
    Assembler asm;
    asm_init(&asm);
    
    if (!assemble_file(input_stream, &asm)) {
        asm_free(&asm);
        return 1;
    }
    
	execute_code(&asm);
    
    asm_free(&asm);
    return 0;
}