#include "string.h"
#include <iostream>
#include <cassert>

// Custom allocator for testing
StringStorage* custom_allocator(const char* source, int byteLen, uint32_t hash) {
    std::cout << "Allocating " << byteLen << " bytes for string '" << source << "'\n";
    StringStorage* storage = (StringStorage*)malloc(sizeof(StringStorage) + byteLen + 1);
    if (storage) {
        storage->lenB = byteLen;
        storage->lenC = byteLen;  // Assume ASCII for simplicity
        storage->hash = hash;
        strcpy(storage->data, source);
    }
    return storage;
}

int main() {
    std::cout << "Testing C++ string class with intern pooling\n\n";
    
    // Test 1: Basic construction and assignment
    std::cout << "Test 1: Basic construction\n";
    string s1("Hello");
    string s2("World");
    std::cout << "s1: '" << s1.c_str() << "' (length: " << s1.lengthB() << " bytes, " << s1.lengthC() << " chars)\n";
    std::cout << "s2: '" << s2.c_str() << "' (length: " << s2.lengthB() << " bytes, " << s2.lengthC() << " chars)\n";
    
    // Test 2: String interning (same string should have same pool index)
    std::cout << "\nTest 2: String interning\n";
    string s3("Hello");
    std::cout << "s1 pool=" << (int)s1.getPoolNum() << " index=" << s1.getIndex() << "\n";
    std::cout << "s3 pool=" << (int)s3.getPoolNum() << " index=" << s3.getIndex() << "\n";
    std::cout << "s1 and s3 should have same index: " << (s1.getIndex() == s3.getIndex() ? "YES" : "NO") << "\n";
    
    // Test 3: Concatenation
    std::cout << "\nTest 3: Concatenation\n";
    string combined = s1 + s2;
    std::cout << "s1 + s2 = '" << combined.c_str() << "' (length: " << combined.lengthB() << " bytes)\n";
    
    // Test 4: Equality comparison
    std::cout << "\nTest 4: Equality comparison\n";
    std::cout << "s1 == s3: " << (s1 == s3 ? "TRUE" : "FALSE") << "\n";
    std::cout << "s1 == s2: " << (s1 == s2 ? "TRUE" : "FALSE") << "\n";
    
    // Test 5: Different pools
    std::cout << "\nTest 5: Different pools\n";
    string s4("Hello", 1);  // Pool 1
    string s5("Hello", 2);  // Pool 2
    std::cout << "s4 (pool 1): pool=" << (int)s4.getPoolNum() << " index=" << s4.getIndex() << "\n";
    std::cout << "s5 (pool 2): pool=" << (int)s5.getPoolNum() << " index=" << s5.getIndex() << "\n";
    std::cout << "s4 == s5: " << (s4 == s5 ? "TRUE" : "FALSE") << " (should be TRUE - same content)\n";
    
    // Test 6: Custom allocator (using static member)
    std::cout << "\nTest 6: Custom allocator (using static member)\n";
    StringStorageAllocator oldAllocator = StringStorage::allocator;
    StringStorage::allocator = custom_allocator;
    string s6("Custom");
    std::cout << "s6: '" << s6.c_str() << "'\n";
    StringStorage::allocator = oldAllocator;  // Restore default
    
    // Test 7: UTF-8 strings
    std::cout << "\nTest 7: UTF-8 strings\n";
    string utf8("Hello 世界");
    std::cout << "UTF-8 string: '" << utf8.c_str() << "' (bytes: " << utf8.lengthB() << ", chars: " << utf8.lengthC() << ")\n";
    
    // Test 8: Assignment operator from C string
    std::cout << "\nTest 8: Assignment operator from C string\n";
    string s7;
    std::cout << "Before assignment: '" << s7.c_str() << "'\n";
    s7 = "Assigned";
    std::cout << "After assignment: '" << s7.c_str() << "'\n";
    
    string s8("Initial");
    std::cout << "s8 before reassignment: '" << s8.c_str() << "'\n";
    s8 = "Reassigned";
    std::cout << "s8 after reassignment: '" << s8.c_str() << "'\n";
    
    // Test 9: C# String API - basic properties and search
    std::cout << "\nTest 9: C# String API - Properties and Search\n";
    string text("Hello, World!");
    std::cout << "Text: '" << text.c_str() << "'\n";
    std::cout << "Length(): " << text.Length() << "\n";
    std::cout << "Empty(): " << (text.Empty() ? "true" : "false") << "\n";
    std::cout << "IsNullOrEmpty(): " << (text.IsNullOrEmpty() ? "true" : "false") << "\n";
    std::cout << "Contains(\"World\"): " << (text.Contains("World") ? "true" : "false") << "\n";
    std::cout << "StartsWith(\"Hello\"): " << (text.StartsWith("Hello") ? "true" : "false") << "\n";
    std::cout << "EndsWith(\"!\"): " << (text.EndsWith("!") ? "true" : "false") << "\n";
    std::cout << "IndexOf(\"World\"): " << text.IndexOf("World") << "\n";
    std::cout << "IndexOf('o'): " << text.IndexOf('o') << "\n";
    
    // Test 10: C# String API - manipulation
    std::cout << "\nTest 10: C# String API - Manipulation\n";
    string hello("Hello");
    std::cout << "Original: '" << hello.c_str() << "'\n";
    string sub = hello.Substring(1, 3);
    std::cout << "Substring(1, 3): '" << sub.c_str() << "'\n";
    string sub2 = hello.Substring(2);
    std::cout << "Substring(2): '" << sub2.c_str() << "'\n";
    
    // Test 11: C# String API - whitespace checking
    std::cout << "\nTest 11: C# String API - Whitespace\n";
    string empty("");
    string spaces("   ");
    string mixed("  Hello  ");
    std::cout << "Empty IsNullOrWhiteSpace(): " << (empty.IsNullOrWhiteSpace() ? "true" : "false") << "\n";
    std::cout << "Spaces IsNullOrWhiteSpace(): " << (spaces.IsNullOrWhiteSpace() ? "true" : "false") << "\n";
    std::cout << "Mixed IsNullOrWhiteSpace(): " << (mixed.IsNullOrWhiteSpace() ? "true" : "false") << "\n";
    
    // Test 12: C# String API - Case conversion and trimming
    std::cout << "\nTest 12: C# String API - Case conversion and trimming\n";
    string mixedCase("Hello WORLD");
    std::cout << "Original: '" << mixedCase.c_str() << "'\n";
    std::cout << "ToLower(): '" << mixedCase.ToLower().c_str() << "'\n";
    std::cout << "ToUpper(): '" << mixedCase.ToUpper().c_str() << "'\n";
    
    string padded("  Hello World  ");
    std::cout << "Padded: '" << padded.c_str() << "'\n";
    std::cout << "Trim(): '" << padded.Trim().c_str() << "'\n";
    std::cout << "TrimStart(): '" << padded.TrimStart().c_str() << "'\n";
    std::cout << "TrimEnd(): '" << padded.TrimEnd().c_str() << "'\n";
    
    // Test 13: C# String API - Splitting
    std::cout << "\nTest 13: C# String API - Splitting\n";
    string csv("apple,banana,cherry");
    std::cout << "CSV: '" << csv.c_str() << "'\n";
    int count;
    string* parts = csv.Split(',', &count);
    std::cout << "Split by comma (" << count << " parts):\n";
    for (int i = 0; i < count; i++) {
        std::cout << "  [" << i << "]: '" << parts[i].c_str() << "'\n";
    }
    free(parts);
    
    // Test 14: Pool-aware allocator
    std::cout << "\nTest 14: Pool-aware allocator\n";
    StringStorage::allocator = StringPool::poolAwareAllocator;
    StringPool::setDefaultPool(1);  // Use pool 1 for global pooling
    
    string pooled1("PoolTest");
    string pooled2("PoolTest");  // Should reuse storage from pool
    std::cout << "pooled1: '" << pooled1.c_str() << "' pool=" << (int)pooled1.getPoolNum() << "\n";
    std::cout << "pooled2: '" << pooled2.c_str() << "' pool=" << (int)pooled2.getPoolNum() << "\n";
    std::cout << "Same StringStorage? " << (pooled1.getStorage() == pooled2.getStorage() ? "YES" : "NO") << "\n";
    
    StringStorage::allocator = defaultStringAllocator;  // Restore default
    
    // Test 15: Comparison operators
    std::cout << "\nTest 15: Comparison operators\n";
    string apple("apple");
    string banana("banana");
    string apple2("apple");
    
    std::cout << "apple == apple2: " << (apple == apple2 ? "TRUE" : "FALSE") << "\n";
    std::cout << "apple != banana: " << (apple != banana ? "TRUE" : "FALSE") << "\n";
    std::cout << "apple < banana: " << (apple < banana ? "TRUE" : "FALSE") << "\n";
    std::cout << "apple <= banana: " << (apple <= banana ? "TRUE" : "FALSE") << "\n";
    std::cout << "banana > apple: " << (banana > apple ? "TRUE" : "FALSE") << "\n";
    std::cout << "banana >= apple: " << (banana >= apple ? "TRUE" : "FALSE") << "\n";
    std::cout << "apple <= apple2: " << (apple <= apple2 ? "TRUE" : "FALSE") << "\n";
    std::cout << "apple >= apple2: " << (apple >= apple2 ? "TRUE" : "FALSE") << "\n";
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}