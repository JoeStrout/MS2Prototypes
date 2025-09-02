# C++ String Class with Intern Pooling

This directory contains lightweight C#-style string and list implementations in C++ that use intern pooling for efficient string management.

## Features

- **C# API**: Mimics the C# API closely, for easy conversion from C# code
- **Lightweight**: String objects are just 3 bytes (pool number + index)
- **Intern Pooling**: Strings are automatically deduplicated within pools
- **Multiple Pools**: Support for 256 different intern pools (0-255)
- **Custom Allocators**: Pluggable memory allocation for string storage
- **UTF-8 Support**: Proper handling of UTF-8 encoded strings
- **Zero-Copy Access**: Direct access to C strings without copying

## Architecture

### Core Files

- `string.h/.cpp` - Main string class with C# API
- `StringPool.h/.cpp` - String intern pool management
- `unicodeUtil.h/.c` - UTF-8 Unicode utilities

### Core Components

- `StringStorage`: Heap-allocated structure containing string data, length info, and hash
- `StringPool`: Namespace containing pool management functions (256 separate pools)
- `string`: Lightweight class containing pool number (8-bit) and index (16-bit)
- `StringStorageAllocator`: Function pointer type for custom string allocation

### String Storage Structure

```cpp
struct StringStorage {
    int lenB;           // Length in bytes
    int lenC;           // Length in characters (UTF-8)
    uint32_t hash;      // String hash for fast comparison
    char data[];        // Flexible array member for string data
};
```

### String Class

```cpp
class string {
private:
    uint8_t poolNum;    // Intern pool number (0-255)
    uint16_t index;     // Index within the pool
    // ... methods
};
```

## Usage

```cpp
#include "string.h"

// Basic usage
string s1("Hello");
string s2 = "World";	// either style works

// Concatenation
string combined = s1 + s2;  // "HelloWorld"

// Length queries
int bytes = s1.lengthB();   // Length in bytes
int chars = s1.lengthC();   // Length in UTF-8 characters

// C string access (no-copy)
const char* cstr = s1.c_str();

// Comparison
bool equal = (s1 == s2);

// Different pools
string s3("Hello", 1);      // Pool 1
string s4("Hello", 2);      // Pool 2

// Custom allocator (global setting)
StringStorageAllocator oldAllocator = string::allocator;
string::allocator = myCustomAllocator;
string s5("Custom");
string::allocator = oldAllocator;  // Restore

// Pool-aware allocator (automatic deduplication across pools)
string::allocator = StringPool::poolAwareAllocator;
StringPool::setDefaultPool(1);  // Use pool 1 for global deduplication
```

## Building

```bash
make          # Build all targets
make test     # Build and run tests
make clean    # Clean build artifacts
```

## C# String API Compatibility

The string class implements most of the C# System.String API with identical method names and behavior:

### Properties and Basic Operations
- `Length()` - Get character count (same as `lengthC()`)
- `Empty()` - Check if string is empty
- `IsNullOrEmpty()` - Check if string is null or empty
- `IsNullOrWhiteSpace()` - Check if string is null, empty, or whitespace
- `operator[]` - Character access by index (byte-based)

### Search Methods
- `IndexOf(string/char)` - Find first occurrence
- `IndexOf(string/char, startIndex)` - Find first occurrence from index
- `Contains(string)` - Check if string contains substring
- `StartsWith(string)` - Check if string starts with prefix
- `EndsWith(string)` - Check if string ends with suffix

### Manipulation Methods
- `Substring(startIndex)` - Get substring from index to end
- `Substring(startIndex, length)` - Get substring of specified length
- `Replace(oldValue, newValue)` - Replace all occurrences (not yet implemented)
- `Insert(startIndex, value)` - Insert string at index (not yet implemented)
- `Remove(startIndex)` - Remove from index to end (not yet implemented)

### Case Conversion (ASCII only)
- `ToLower()` - Convert to lowercase
- `ToUpper()` - Convert to uppercase

### Trimming (Unicode-aware)
- `Trim()` - Remove leading and trailing whitespace
- `TrimStart()` - Remove leading whitespace
- `TrimEnd()` - Remove trailing whitespace

### Splitting
- `Split(char separator, int* count)` - Split by character, returns array
- `Split(string separator, int* count)` - Split by string (not yet implemented)

## Design Notes

- **STL-Free**: Uses only standard C library and custom Unicode utilities
- **Intern Pooling**: Strings within the same pool are automatically deduplicated
- **Optimized Comparison**: Same pool + index = instant equality check
- **Callback Allocation**: Uses callback-based allocation for memory management flexibility
- **UTF-8 Aware**: Proper handling of UTF-8 encoded strings with Unicode utilities
- **Hash Caching**: Hash values computed once and cached for fast operations
- **C# Compatible**: Method names and behavior match C# System.String where possible