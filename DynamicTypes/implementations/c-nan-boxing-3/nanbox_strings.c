// nanbox_strings.c
//
// String operations for the NaN-boxing implementation
// These functions are too complex to be inlined and need proper GC management

#include "nanbox.h"
#include "nanbox_gc.h"
#include "unicodeUtil.h"
#include <stdlib.h>
#include <string.h>

// String interning system
#define INTERN_THRESHOLD 128  // Intern strings under 128 bytes
#define INTERN_TABLE_SIZE 1024  // Hash table size (must be power of 2)

// Hash table entry for string interning
typedef struct InternEntry {
    Value string_value;      // The interned string (heap string Value)
    struct InternEntry* next;  // Collision chain
} InternEntry;

// Global intern table
static InternEntry* intern_table[INTERN_TABLE_SIZE];
static bool intern_table_initialized = false;

// Initialize intern table
static void init_intern_table() {
    if (intern_table_initialized) return;
    for (int i = 0; i < INTERN_TABLE_SIZE; i++) {
        intern_table[i] = NULL;
    }
    intern_table_initialized = true;
}

// FNV-1a hash function for strings
// Returns 0 to indicate "not computed" is reserved, so we use 1 as minimum hash
uint32_t string_hash(const char* data, int len) {
    // FNV-1a constants
    const uint32_t FNV_PRIME = 0x01000193;
    const uint32_t FNV_OFFSET_BASIS = 0x811c9dc5;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; i < len; i++) {
        hash ^= (unsigned char)data[i];
        hash *= FNV_PRIME;
    }
    
    // Ensure hash is never 0 (reserved for "not computed")
    return hash == 0 ? 1 : hash;
}

// Find or create an interned string
// Returns existing interned string if found, or NULL if not found
static Value find_interned_string(const char* data, int lenB, uint32_t hash) {
    init_intern_table();
    
    int bucket = hash & (INTERN_TABLE_SIZE - 1);  // hash % table_size
    InternEntry* entry = intern_table[bucket];
    
    while (entry != NULL) {
        if (is_heap_string(entry->string_value)) {
            String* s = as_string(entry->string_value);
            if (s->lenB == lenB && s->hash == hash && memcmp(s->data, data, lenB) == 0) {
                return entry->string_value;  // Found existing interned string
            }
        }
        entry = entry->next;
    }
    
    return make_nil();  // Not found
}

// Add a string to the intern table
// The string_value must be a heap string with hash already computed
static void intern_string(Value string_value) {
    if (!is_heap_string(string_value)) return;
    
    String* s = as_string(string_value);
    if (s->hash == 0) return;  // Hash must be computed
    
    init_intern_table();
    
    int bucket = s->hash & (INTERN_TABLE_SIZE - 1);
    
    // Allocate new entry (this is not GC'd - it's part of the intern table)
    InternEntry* new_entry = malloc(sizeof(InternEntry));
    new_entry->string_value = string_value;
    new_entry->next = intern_table[bucket];
    intern_table[bucket] = new_entry;
}

// Interning-aware string creation
// Interned strings use malloc() directly (not gc_allocate) since they're immortal
Value make_string_interned(const char* str) {
    if (str == NULL) return make_nil();
    int lenB = strlen(str);
    
    // Use tiny string for very short strings
    if (lenB <= TINY_STRING_MAX_LEN) {
        return make_tiny_string(str, lenB);
    }
    
    // Use interning for strings under threshold
    if (lenB < INTERN_THRESHOLD) {
        // Compute hash for interning
        uint32_t hash = string_hash(str, lenB);
        
        // Check if already interned
        Value existing = find_interned_string(str, lenB, hash);
        if (!is_nil(existing)) {
            return existing;
        }
        
        // Create new interned string with malloc (not GC'd - immortal by design)
        String* s = (String*)malloc(sizeof(String) + lenB + 1);
        s->lenB = lenB;
        s->hash = hash;  // Store computed hash
        strcpy(s->data, str);
        Value new_string = STRING_MASK | ((uintptr_t)s & 0xFFFFFFFFFFFFULL);
        
        // Add to intern table
        intern_string(new_string);
        
        return new_string;
    } else {
        // For longer strings, use regular heap allocation with lazy hashing
        String* s = (String*)gc_allocate(sizeof(String) + lenB + 1);
        s->lenB = lenB;
        s->hash = 0;  // Hash will be computed when needed
        strcpy(s->data, str);
        return STRING_MASK | ((uintptr_t)s & 0xFFFFFFFFFFFFULL);
    }
}


Value string_concat(Value a, Value b) {
    GC_PUSH_SCOPE();
    
    Value result = make_nil();
    GC_PROTECT(&a);
    GC_PROTECT(&b);
    GC_PROTECT(&result);
    
    // Use TRUE zero-copy access - no copying for tiny strings!
    int lenB_a, lenB_b;
    const char* sa = get_string_data_zerocopy(&a, &lenB_a);
    const char* sb = get_string_data_zerocopy(&b, &lenB_b);
    
    if (!sa || !sb) {
        GC_POP_SCOPE();
        return make_nil();
    }
    
    int total_lenB = lenB_a + lenB_b;
    
    // Use tiny string if result is small enough (in bytes)
    if (total_lenB <= TINY_STRING_MAX_LEN) {
        char result_buffer[TINY_STRING_MAX_LEN + 1];
        memcpy(result_buffer, sa, lenB_a);
        memcpy(result_buffer + lenB_a, sb, lenB_b);
        result_buffer[total_lenB] = '\0';
        result = make_tiny_string(result_buffer, total_lenB);
    } else {
        // Use heap string for longer results
        String* result_str = (String*)gc_allocate(sizeof(String) + total_lenB + 1);
        result_str->lenB = total_lenB;
        result_str->hash = 0;  // Hash not computed yet
        memcpy(result_str->data, sa, lenB_a);
        memcpy(result_str->data + lenB_a, sb, lenB_b);
        result_str->data[total_lenB] = '\0';
        result = STRING_MASK | ((uintptr_t)result_str & 0xFFFFFFFFFFFFULL);
    }
    
    GC_POP_SCOPE();
    return result;
}

Value string_replace(Value str, Value from, Value to) {
    GC_PUSH_SCOPE();
    
    Value result = make_nil();
    GC_PROTECT(&str);
    GC_PROTECT(&from);
    GC_PROTECT(&to);
    GC_PROTECT(&result);
    
    int from_lenB = string_lengthB(from);
    int to_lenB = string_lengthB(to);
    int str_lenB = string_lengthB(str);
    
    if (from_lenB == 0 || str_lenB == 0) {
        GC_POP_SCOPE();
        return str; // Can't replace empty string, or nothing to replace in empty string
    }
    
    // Get null-terminated versions of the strings
    char tiny_buffer_s[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_f[TINY_STRING_MAX_LEN + 1];  
    char tiny_buffer_t[TINY_STRING_MAX_LEN + 1];
    
    const char* s = get_string_data_nullterm(&str, tiny_buffer_s);
    const char* f = get_string_data_nullterm(&from, tiny_buffer_f);
    const char* t = get_string_data_nullterm(&to, tiny_buffer_t);
    
    if (!s || !f || !t) {
        GC_POP_SCOPE();
        return make_nil();
    }
    
    // Count occurrences to calculate final length
    int count = 0;
    const char* temp = s;
    while ((temp = strstr(temp, f)) != NULL) {
        count++;
        temp += from_lenB;
    }
    
    if (count == 0) {
        GC_POP_SCOPE();
        return str; // Return original if not found
    }
    
    // Calculate new byte length
    int new_lenB = str_lenB + count * (to_lenB - from_lenB);
    
    // Use static buffer for small results, heap for larger ones
    char static_buffer[256];
    char* temp_result;
    bool use_static = ((size_t)new_lenB < sizeof(static_buffer));
    
    if (use_static) {
        temp_result = static_buffer;
    } else {
        temp_result = malloc(new_lenB + 1);
    }
    
    // Build the result string
    char* dest = temp_result;
    const char* src = s;
    
    while (*src) {
        const char* found = strstr(src, f);
        if (found == NULL) {
            strcpy(dest, src);
            break;
        }
        
        // Copy text before the match
        int before_len = found - src;
        strncpy(dest, src, before_len);
        dest += before_len;
        
        // Copy replacement text
        strcpy(dest, t);
        dest += to_lenB;
        
        // Move source pointer past the match
        src = found + from_lenB;
    }
    
    result = make_string(temp_result);
    
    // Clean up heap allocation if used
    if (!use_static) {
        free(temp_result);
    }
    
    GC_POP_SCOPE();
    return result;
}

Value string_split(Value str, Value delimiter) {
    GC_PUSH_SCOPE();
    
    Value result = make_nil();
    GC_PROTECT(&str);
    GC_PROTECT(&delimiter);
    GC_PROTECT(&result);
    
    int str_lenB = string_lengthB(str);
    int delim_lenB = string_lengthB(delimiter);
    
    if (str_lenB == 0) {
        result = make_list(0);
        GC_POP_SCOPE();
        return result;
    }
    
    // Get string data
    char tiny_buffer_s[TINY_STRING_MAX_LEN + 1];
    const char* s = get_string_data_nullterm(&str, tiny_buffer_s);
    if (!s) {
        GC_POP_SCOPE();
        return make_nil();
    }
    
    bool has_delimiter = (delim_lenB > 0);
    const char* delim = NULL;
    char tiny_buffer_delim[TINY_STRING_MAX_LEN + 1];
    
    if (has_delimiter) {
        delim = get_string_data_nullterm(&delimiter, tiny_buffer_delim);
        if (!delim) {
            GC_POP_SCOPE();
            return make_nil();
        }
    }
    
    // Handle empty delimiter (split into characters)
    if (!has_delimiter || strlen(delim) == 0) {
        int charCount = UTF8CharacterCount((const unsigned char*)s, str_lenB);
        result = make_list(charCount);
        
        unsigned char* ptr = (unsigned char*)s;
        unsigned char* end = ptr + str_lenB;
        
        while (ptr < end) {
            unsigned char* charStart = ptr;
            UTF8DecodeAndAdvance(&ptr);
            
            // Create string from this character
            int charLenB = ptr - charStart;
            char charBuffer[5];  // Max UTF-8 character is 4 bytes + null
            memcpy(charBuffer, charStart, charLenB);
            charBuffer[charLenB] = '\0';
            
            Value char_val = make_string(charBuffer);
            list_add(result, char_val);
        }
    }
    // Handle space delimiter as special case - manual split to preserve empty tokens
    else if (strcmp(delim, " ") == 0) {
        result = make_list(100); // Rough estimate
        int start = 0;
        
        for (int i = 0; i <= str_lenB; i++) {
            if (i == str_lenB || s[i] == ' ') {
                // Found delimiter or end of string
                int token_len = i - start;
                char token_buffer[256];
                if ((size_t)token_len < sizeof(token_buffer)) {
                    strncpy(token_buffer, s + start, token_len);
                    token_buffer[token_len] = '\0';
                    Value token_val = make_string(token_buffer);
                    list_add(result, token_val);
                } else {
                    char* token = malloc(token_len + 1);
                    strncpy(token, s + start, token_len);
                    token[token_len] = '\0';
                    Value token_val = make_string(token);
                    list_add(result, token_val);
                    free(token);
                }
                start = i + 1;
            }
        }
    }
    // General case: split on specific delimiter
    else {
        result = make_list(50); // Rough estimate
        char s_buffer[256];
        char* s_copy;
        bool use_static = ((size_t)str_lenB < sizeof(s_buffer));
        
        if (use_static) {
            strcpy(s_buffer, s);
            s_copy = s_buffer;
        } else {
            s_copy = malloc(str_lenB + 1);
            strcpy(s_copy, s);
        }
        
        char* token = strtok(s_copy, delim);
        while (token != NULL) {
            Value token_val = make_string(token);
            list_add(result, token_val);
            token = strtok(NULL, delim);
        }
        
        if (!use_static) {
            free(s_copy);
        }
    }
    
    GC_POP_SCOPE();
    return result;
}

Value string_substring(Value str, int startIndex, int len) {
    GC_PUSH_SCOPE();
    
    Value result = make_nil();
    GC_PROTECT(&str);
    GC_PROTECT(&result);
    
    if (!is_string(str) || startIndex < 0 || len < 0) {
        GC_POP_SCOPE();
        return make_nil();
    }
    
    int strLenB = string_lengthB(str);
    if (strLenB == 0) {
        result = make_string("");
        GC_POP_SCOPE();
        return result;
    }
    
    // Get string data
    char tiny_buffer[TINY_STRING_MAX_LEN + 1];
    const char* data = get_string_data_nullterm(&str, tiny_buffer);
    if (!data) {
        GC_POP_SCOPE();
        return make_nil();
    }
    
    // Convert character indexes to byte indexes
    int startByteIndex = UTF8CharIndexToByteIndex((const unsigned char*)data, startIndex, strLenB);
    if (startByteIndex < 0) {
        result = make_string("");
        GC_POP_SCOPE();
        return result;
    }
    
    int endCharIndex = startIndex + len;
    int endByteIndex = UTF8CharIndexToByteIndex((const unsigned char*)data, endCharIndex, strLenB);
    if (endByteIndex < 0) endByteIndex = strLenB;  // Use end of string if out of range
    
    int subLenB = endByteIndex - startByteIndex;
    if (subLenB <= 0) {
        result = make_string("");
        GC_POP_SCOPE();
        return result;
    }
    
    // Create substring
    char* subBuffer = malloc(subLenB + 1);
    memcpy(subBuffer, data + startByteIndex, subLenB);
    subBuffer[subLenB] = '\0';
    
    result = make_string(subBuffer);
    free(subBuffer);
    
    GC_POP_SCOPE();
    return result;
}

Value string_charAt(Value str, int index) {
    return string_substring(str, index, 1);
}