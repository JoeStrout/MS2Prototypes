#include <stdio.h>
#include <string.h>
#include "nanbox.h"
#include "nanbox_gc.h"

int main() {
    printf("Protected split test\n");
    printf("====================\n");
    
    GC_PUSH_SCOPE();
    
    Value unicode_str = make_string("A©");
    GC_PROTECT(&unicode_str);
    
    printf("Original: '%s'\n", as_cstring(unicode_str));
    
    // Get string data and copy it to local buffer before any GC operations
    const char* str_data = as_cstring(unicode_str);
    int str_lenB = string_lengthB(unicode_str);
    char local_copy[50];
    strcpy(local_copy, str_data);
    
    printf("Local copy: '%s' (lenB=%d)\n", local_copy, str_lenB);
    
    int charCount = UTF8CharacterCount((const unsigned char*)local_copy, str_lenB);
    printf("Character count: %d\n", charCount);
    
    // Create list first
    Value list = make_list(charCount);
    GC_PROTECT(&list);
    
    // Create character values and protect them
    Value char_values[10];  // Assume max 10 characters
    
    unsigned char* ptr = (unsigned char*)local_copy;
    unsigned char* end = ptr + str_lenB;
    
    int i = 0;
    while (ptr < end && i < charCount && i < 10) {
        unsigned char* charStart = ptr;
        UTF8DecodeAndAdvance(&ptr);
        
        int charLenB = ptr - charStart;
        char charBuffer[5];
        memcpy(charBuffer, charStart, charLenB);
        charBuffer[charLenB] = '\0';
        
        printf("Creating character %d: '%s' (lenB=%d)\n", i, charBuffer, charLenB);
        
        char_values[i] = make_string(charBuffer);
        GC_PROTECT(&char_values[i]);
        
        printf("  Created and protected: '%s'\n", as_cstring(char_values[i]));
        
        i++;
    }
    
    // Now add them all to the list
    for (int j = 0; j < i; j++) {
        printf("Adding character %d to list: '%s'\n", j, as_cstring(char_values[j]));
        list_add(list, char_values[j]);
    }
    
    printf("Final list count: %d\n", list_count(list));
    for (int j = 0; j < list_count(list); j++) {
        Value item = list_get(list, j);
        printf("  [%d]: '%s'\n", j, as_cstring(item));
    }
    
    GC_POP_SCOPE();
    
    return 0;
}