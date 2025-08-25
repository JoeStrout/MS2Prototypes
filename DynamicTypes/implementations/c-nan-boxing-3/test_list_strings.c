#include <stdio.h>
#include <string.h>
#include "nanbox.h"
#include "nanbox_gc.h"

int main() {
    printf("Testing tiny string list operations\n");
    printf("===================================\n");
    
    GC_PUSH_SCOPE();
    
    // Create some small strings that should be tiny strings
    Value str_a = make_string("A");
    Value str_b = make_string("©");
    
    GC_PROTECT(&str_a);
    GC_PROTECT(&str_b);
    
    printf("Created strings:\n");
    printf("  A: '%s' is_tiny=%s\n", as_cstring(str_a), is_tiny_string(str_a) ? "true" : "false");
    printf("  ©: '%s' is_tiny=%s\n", as_cstring(str_b), is_tiny_string(str_b) ? "true" : "false");
    
    // Create list and add them
    Value list = make_list(10);
    GC_PROTECT(&list);
    
    printf("Created list\n");
    
    list_add(list, str_a);
    printf("Added A to list\n");
    
    list_add(list, str_b);
    printf("Added © to list\n");
    
    printf("List count: %d\n", list_count(list));
    
    Value item0 = list_get(list, 0);
    Value item1 = list_get(list, 1);
    
    printf("Retrieved from list:\n");
    printf("  [0]: '%s'\n", as_cstring(item0));
    printf("  [1]: '%s'\n", as_cstring(item1));
    
    GC_POP_SCOPE();
    
    return 0;
}