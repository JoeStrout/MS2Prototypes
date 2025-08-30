#include "../../include/assembler/assembler.h"
#include "../../include/types/strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void init_function(Function *func) {
    func->code_capacity = 256;
    func->code = (uint32_t*)calloc(func->code_capacity, sizeof(uint32_t));
    func->code_len = 0;
    func->label_count = 0;
    func->ref_count = 0;
    func->const_capacity = 64;
    func->constants = (Value*)calloc(func->const_capacity, sizeof(Value));
    func->const_len = 0;
    func->max_regs = 16;  // Default
    func->is_main = false;
    memset(func->labels, 0, sizeof(func->labels));
    memset(func->refs, 0, sizeof(func->refs));
}

void asm_init(Assembler *asm) {
    asm->function_count = 0;
    asm->current_function = NULL;
    asm->current_address = 0;
    memset(asm->functions, 0, sizeof(asm->functions));
}

void asm_free(Assembler *asm) {
    for (size_t i = 0; i < asm->function_count; i++) {
        free(asm->functions[i].code);
        free(asm->functions[i].constants);
    }
    asm->function_count = 0;
    asm->current_function = NULL;
}

bool asm_start_function(Assembler *asm, const char *name, bool is_main) {
    if (asm->function_count >= MAX_FUNCTIONS) {
        return false;
    }
    
    Function *func = &asm->functions[asm->function_count++];
    init_function(func);
    strncpy(func->name, name, 63);
    func->name[63] = '\0';
    func->is_main = is_main;
    
    asm->current_function = func;
    asm->current_address = 0;
    return true;
}

bool asm_end_function(Assembler *asm) {
    if (!asm->current_function) {
        return false;
    }
    asm->current_function = NULL;
    return true;
}

static void ensure_capacity(Function *func) {
    if (func->code_len >= func->code_capacity) {
        func->code_capacity *= 2;
        func->code = (uint32_t*)realloc(func->code, func->code_capacity * sizeof(uint32_t));
    }
}

static bool ensure_function(Assembler *asm) {
    if (asm->current_function) return true;
	fprintf(stderr, "Error: No current function for instruction\n");
	return false;
}

static void emit_instruction(Assembler *asm, uint32_t ins) {
	if (!ensure_function(asm)) return;
    Function *func = asm->current_function;
    ensure_capacity(func);
    func->code[func->code_len++] = ins;
    asm->current_address++;
}

static bool is_register(const char *str, uint8_t *reg) {
    if (str[0] == 'r' && isdigit(str[1])) {
        int r = atoi(str + 1);
        if (r >= 0 && r <= 255) {
            *reg = (uint8_t)r;
            return true;
        }
    }
    return false;
}

static bool is_immediate(const char *str, int32_t *imm) {
    char *endptr;
    *imm = strtol(str, &endptr, 0);
    return *endptr == '\0';
}

static bool parse_constant(const char *str, Value *value) {
	// Check for null keyword
	if (strcmp(str, "null") == 0) {
		*value = make_null();
		return true;
	}
    // Check for string literal (enclosed in double quotes)
    if (str[0] == '"') {
        int len = strlen(str);
        if (len >= 2 && str[len-1] == '"') {
            // Remove quotes and create string
            char *content = malloc(len - 1);
            memcpy(content, str + 1, len - 2);
            content[len - 2] = '\0';
            *value = make_string(content);
            free(content);
            return true;
        }
        return false;
    }
    
    // Check for double (contains decimal point)
    if (strchr(str, '.') != NULL) {
        char *endptr;
        double d = strtod(str, &endptr);
        if (*endptr == '\0') {
            *value = make_double(d);
            return true;
        }
        return false;
    }
    
    // Check for integer
    int32_t imm;
    if (is_immediate(str, &imm)) {
        *value = make_int(imm);
        return true;
    }
    
    return false;
}

static char *trim(char *str) {
    while (isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) *end-- = '\0';
    return str;
}

static bool parse_tokens(const char *line, char tokens[4][64], int *token_count) {
    *token_count = 0;
    char *copy = strdup(line);
    char *token = strtok(copy, " \t,");
    
    while (token && *token_count < 4) {
        strcpy(tokens[*token_count], trim(token));
        (*token_count)++;
        token = strtok(NULL, " \t,");
    }
    
    free(copy);
    return *token_count > 0;
}

bool asm_instruction(Assembler *asm, const char *line) {
    char tokens[4][64];
    int token_count;
    
    if (!parse_tokens(line, tokens, &token_count) || token_count == 0) {
        return true; // empty line
    }
    
    const char *opcode = tokens[0];
    
    if (strcmp(opcode, "MOVE") == 0 && token_count == 3) {
        uint8_t a, b;
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b)) {
            emit_instruction(asm, INS_ABC(MOVE, a, b, 0));
            return true;
        }
    }
    else if (strcmp(opcode, "LOADK") == 0 && token_count == 3) {
        uint8_t a;
        int32_t imm;
        if (is_register(tokens[1], &a) && is_immediate(tokens[2], &imm)) {
            if (imm >= -32768 && imm <= 32767) {
                emit_instruction(asm, INS_AB(LOADK, a, (int16_t)imm));
                return true;
            }
        }
    }
    else if (strcmp(opcode, "LOAD") == 0 && token_count == 3) {
        uint8_t a;
        if (!is_register(tokens[1], &a)) {
            return false;
        }
    	if (!ensure_function(asm)) return false;
        
        // Try to parse as immediate integer first
        int32_t imm;
        if (is_immediate(tokens[2], &imm) && imm >= -32768 && imm <= 32767) {
            // Use LOADK for 16-bit signed integers
            emit_instruction(asm, INS_AB(LOADK, a, (int16_t)imm));
            return true;
        } else {
            // Use LOADN for everything else (large integers, doubles, strings)
            Value const_value;
            if (parse_constant(tokens[2], &const_value)) {
                int const_idx = asm_add_constant(asm->current_function, const_value);
                if (const_idx >= 0 && const_idx <= 65535) {
                    emit_instruction(asm, INS_AB(LOADN, a, (uint16_t)const_idx));
                    return true;
                } else {
                    fprintf(stderr, "Error: Too many constants (max 65536)\n");
                    return false;
                }
            }
        }
    }
    else if (strcmp(opcode, "LOADN") == 0 && token_count == 3) {
    	// ToDo: eliminate code duplication.  Perhaps each opcode should
    	// have its own little assembler function, and then pseudo-ops
    	// like LOAD can just call the appropriate function.
    	if (!ensure_function(asm)) return false;
        uint8_t a;
        Value const_value;
        if (is_register(tokens[1], &a) && parse_constant(tokens[2], &const_value)) {
            // Add constant to table and get index
            int const_idx = asm_add_constant(asm->current_function, const_value);
            if (const_idx >= 0 && const_idx <= 65535) {
                emit_instruction(asm, INS_AB(LOADN, a, (uint16_t)const_idx));
                return true;
            } else {
                fprintf(stderr, "Error: Too many constants (max 65535)\n");
                return false;
            }
        }
    }
    else if (strcmp(opcode, "ADD") == 0 && token_count == 4) {
        uint8_t a, b, c;
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b) && is_register(tokens[3], &c)) {
            emit_instruction(asm, INS_ABC(ADD, a, b, c));
            return true;
        }
    }
    else if (strcmp(opcode, "SUB") == 0 && token_count == 4) {
        uint8_t a, b, c;
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b) && is_register(tokens[3], &c)) {
            emit_instruction(asm, INS_ABC(SUB, a, b, c));
            return true;
        }
    }
    else if (strcmp(opcode, "MULT") == 0 && token_count == 4) {
        uint8_t a, b, c;
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b) && is_register(tokens[3], &c)) {
            emit_instruction(asm, INS_ABC(MULT, a, b, c));
            return true;
        }
    }
    else if (strcmp(opcode, "DIV") == 0 && token_count == 4) {
        uint8_t a, b, c;
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b) && is_register(tokens[3], &c)) {
            emit_instruction(asm, INS_ABC(DIV, a, b, c));
            return true;
        }
    }
    else if (strcmp(opcode, "IFLT") == 0 && token_count == 4) {
    	if (!ensure_function(asm)) return false;

        uint8_t a, b;        
        if (is_register(tokens[1], &a) && is_register(tokens[2], &b)) {
            // Third argument could be immediate or label
            int32_t imm;
            if (is_immediate(tokens[3], &imm)) {
                if (imm >= -128 && imm <= 127) {
                    emit_instruction(asm, INS_ABC(IFLT, a, b, (uint8_t)(int8_t)imm));
                    return true;
                }
            } else {
                // Label reference - will be resolved in second pass
                asm_add_reference(asm->current_function, tokens[3], asm->current_address, false);
                emit_instruction(asm, INS_ABC(IFLT, a, b, 0));
                return true;
            }
        }
    }
    else if (strcmp(opcode, "JMP") == 0 && token_count == 2) {
    	if (!ensure_function(asm)) return false;
        int32_t imm;
        if (is_immediate(tokens[1], &imm)) {
            if (imm >= -32768 && imm <= 32767) {
                emit_instruction(asm, INS_AB(JMP, 0, (int16_t)imm));
                return true;
            }
        } else {
            // Label reference - will be resolved in second pass
            asm_add_reference(asm->current_function, tokens[1], asm->current_address, true);
            emit_instruction(asm, INS_AB(JMP, 0, 0));
            return true;
        }
    }
    else if (strcmp(opcode, "CALLF") == 0 && token_count == 4) {
        uint8_t a;
        if (is_register(tokens[1], &a)) {
            int32_t nargs, func_idx;
            if (is_immediate(tokens[2], &nargs) && is_immediate(tokens[3], &func_idx)) {
                if (nargs >= 0 && nargs <= 255 && func_idx >= 0 && func_idx <= 255) {
                    emit_instruction(asm, INS_ABC(CALLF, a, (uint8_t)nargs, (uint8_t)func_idx));
                    return true;
                }
            }
        }
    }
    else if (strcmp(opcode, "RETURN") == 0) {
        emit_instruction(asm, INS(RETURN));
        return true;
    }
    
    fprintf(stderr, "Error: Invalid instruction: %s\n", line);
    return false;
}

bool asm_label(Assembler *asm, const char *name) {
    if (!asm->current_function) {
        fprintf(stderr, "Error: No current function for label\n");
        return false;
    }
    return asm_add_label(asm->current_function, name, asm->current_address);
}

int32_t asm_find_label(Function *func, const char *name) {
    for (size_t i = 0; i < func->label_count; i++) {
        if (strcmp(func->labels[i].name, name) == 0) {
            return func->labels[i].defined ? func->labels[i].address : -1;
        }
    }
    return -1;
}

bool asm_add_label(Function *func, const char *name, int32_t address) {
    if (func->label_count >= MAX_LABELS) {
        return false;
    }
    
    // Check if label already exists
    for (size_t i = 0; i < func->label_count; i++) {
        if (strcmp(func->labels[i].name, name) == 0) {
            if (address >= 0) {
                func->labels[i].address = address;
                func->labels[i].defined = true;
            }
            return true;
        }
    }
    
    // Add new label
    strncpy(func->labels[func->label_count].name, name, 63);
    func->labels[func->label_count].name[63] = '\0';
    func->labels[func->label_count].address = address;
    func->labels[func->label_count].defined = (address >= 0);
    func->label_count++;
    return true;
}

bool asm_add_reference(Function *func, const char *label_name, int32_t addr, bool is_jump) {
    if (func->ref_count >= MAX_REFS) {
        return false;
    }
    
    strncpy(func->refs[func->ref_count].label_name, label_name, 63);
    func->refs[func->ref_count].label_name[63] = '\0';
    func->refs[func->ref_count].instruction_addr = addr;
    func->refs[func->ref_count].is_jump = is_jump;
    func->ref_count++;
    return true;
}

static bool resolve_function_labels(Function *func) {
    // Resolve all references within this function
    for (size_t i = 0; i < func->ref_count; i++) {
        Reference *ref = &func->refs[i];
        int32_t label_addr = asm_find_label(func, ref->label_name);
        
        if (label_addr < 0) {
            fprintf(stderr, "Error: Undefined label: %s in function %s\n", ref->label_name, func->name);
            return false;
        }
        
        int32_t offset = label_addr - (ref->instruction_addr + 1);
        uint32_t *ins = &func->code[ref->instruction_addr];
        
        if (ref->is_jump) {
            // JMP instruction - update 16-bit BC field
            if (offset >= -32768 && offset <= 32767) {
                *ins = INS_AB(JMP, 0, (int16_t)offset);
            } else {
                fprintf(stderr, "Error: Jump offset too large for label %s\n", ref->label_name);
                return false;
            }
        } else {
            // IFLT instruction - update 8-bit C field
            if (offset >= -128 && offset <= 127) {
                uint8_t a = A(*ins);
                uint8_t b = B(*ins);
                *ins = INS_ABC(IFLT, a, b, OFF8(offset));
            } else {
                fprintf(stderr, "Error: Branch offset too large for label %s\n", ref->label_name);
                return false;
            }
        }
    }
    
    return true;
}

bool asm_resolve_labels(Assembler *asm) {
    // Resolve labels in all functions
    for (size_t i = 0; i < asm->function_count; i++) {
        if (!resolve_function_labels(&asm->functions[i])) {
            return false;
        }
    }
    return true;
}

Function *asm_find_function(Assembler *asm, const char *name) {
    for (size_t i = 0; i < asm->function_count; i++) {
        if (strcmp(asm->functions[i].name, name) == 0) {
            return &asm->functions[i];
        }
    }
    return NULL;
}

Function *asm_get_main_function(Assembler *asm) {
    for (size_t i = 0; i < asm->function_count; i++) {
        if (asm->functions[i].is_main) {
            return &asm->functions[i];
        }
    }
    return NULL;
}

static void ensure_const_capacity(Function *func) {
    if (func->const_len >= func->const_capacity) {
        func->const_capacity *= 2;
        func->constants = (Value*)realloc(func->constants, func->const_capacity * sizeof(Value));
    }
}

int asm_add_constant(Function *func, Value value) {
    // Check if constant already exists
    int index = asm_find_constant(func, value);
    if (index >= 0) return index;
    
    // Add new constant
    ensure_const_capacity(func);
    func->constants[func->const_len] = value;
    return (int)(func->const_len++);
}

int asm_find_constant(Function *func, Value value) {
    for (size_t i = 0; i < func->const_len; i++) {
        if (value_equal(func->constants[i], value)) {
            return (int)i;
        }
    }
    return -1;
}

