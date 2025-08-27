#include "List.h"
#include "string.h"
#include <iostream>
#include <type_traits>

int main() {
    std::cout << "Testing memcpy optimization\n\n";
    
    // Test with int (trivially copyable)
    std::cout << "int is trivially copyable: " << (std::is_trivially_copyable<int>::value ? "true" : "false") << "\n";
    List<int> intList;
    for (int i = 1; i <= 5; i++) {
        intList.Add(i * 10);
    }
    
    // Copy constructor should use memcpy
    List<int> intList2 = intList;
    std::cout << "Int list copy: ";
    for (int i = 0; i < intList2.Count(); i++) {
        std::cout << intList2[i] << " ";
    }
    std::cout << "\n";
    
    // Test with string (should also be trivially copyable)
    std::cout << "string is trivially copyable: " << (std::is_trivially_copyable<string>::value ? "true" : "false") << "\n";
    List<string> stringList;
    stringList.Add("Hello");
    stringList.Add("World");
    stringList.Add("memcpy");
    
    // Copy constructor should use memcpy for string too
    List<string> stringList2 = stringList;
    std::cout << "String list copy: ";
    for (int i = 0; i < stringList2.Count(); i++) {
        std::cout << "'" << stringList2[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    // Test ToArray
    int arraySize;
    string* array = stringList.ToArray(&arraySize);
    std::cout << "String ToArray: ";
    for (int i = 0; i < arraySize; i++) {
        std::cout << "'" << array[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    free(array);
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}