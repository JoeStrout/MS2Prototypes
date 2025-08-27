#include "List.h"
#include "string.h"
#include <iostream>
#include <type_traits>

int main() {
    std::cout << "Testing List<T> with MemPool\n\n";
    
    // Test 1: Verify trivial copyability
    std::cout << "Test 1: Trivial copyability check\n";
    std::cout << "List<int> is trivially copyable: " << (std::is_trivially_copyable<List<int>>::value ? "true" : "false") << "\n";
    std::cout << "List<string> is trivially copyable: " << (std::is_trivially_copyable<List<string>>::value ? "true" : "false") << "\n";
    std::cout << "MemRef is trivially copyable: " << (std::is_trivially_copyable<MemRef>::value ? "true" : "false") << "\n";
    
    // Test 2: Basic functionality
    std::cout << "\nTest 2: Basic functionality\n";
    List<int> list1(1);  // Use pool 1
    std::cout << "Empty list count: " << list1.Count() << "\n";
    std::cout << "Empty list is valid: " << (list1.isValid() ? "true" : "false") << "\n";
    
    // Test 3: Adding elements
    std::cout << "\nTest 3: Adding elements\n";
    for (int i = 1; i <= 5; i++) {
        list1.Add(i * 10);
    }
    std::cout << "After adding 5 elements: count=" << list1.Count() << ", capacity=" << list1.Capacity() << "\n";
    std::cout << "Elements: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 4: Copy constructor (trivial copy!)
    std::cout << "\nTest 4: Copy constructor (trivial copy)\n";
    List<int> list2 = list1;  // This is just copying 5 bytes!
    std::cout << "Copied list count: " << list2.Count() << "\n";
    std::cout << "Copied elements: ";
    for (int i = 0; i < list2.Count(); i++) {
        std::cout << list2[i] << " ";
    }
    std::cout << "\n";
    std::cout << "Both lists share same storage: " << (list1.getMemRef() == list2.getMemRef() ? "true" : "false") << "\n";
    
    // Test 5: Working with strings
    std::cout << "\nTest 5: Working with strings\n";
    List<string> stringList(2);  // Use pool 2
    stringList.Add("Hello");
    stringList.Add("MemPool");
    stringList.Add("World");
    
    std::cout << "String list: ";
    for (int i = 0; i < stringList.Count(); i++) {
        std::cout << "'" << stringList[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    // Test 6: AddRange
    std::cout << "\nTest 6: AddRange\n";
    List<string> moreStrings(2);
    moreStrings.Add("Amazing");
    moreStrings.Add("Performance");
    
    stringList.AddRange(moreStrings);
    std::cout << "After AddRange: ";
    for (int i = 0; i < stringList.Count(); i++) {
        std::cout << "'" << stringList[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    // Test 7: Pool lifecycle
    std::cout << "\nTest 7: Pool lifecycle management\n";
    std::cout << "Creating lists in pool 3...\n";
    {
        List<int> tempList(3);
        for (int i = 0; i < 10; i++) {
            tempList.Add(i);
        }
        std::cout << "Temp list in pool 3 has " << tempList.Count() << " elements\n";
        
        // Copy the list - still points to same storage
        List<int> copyOfTemp = tempList;
        std::cout << "Copy of temp list has " << copyOfTemp.Count() << " elements\n";
        
        // Destroy pool 3
        MemPoolManager::destroyPool(3);
        
        // Now both lists should return invalid data
        std::cout << "After destroying pool 3:\n";
        std::cout << "  tempList.Count(): " << tempList.Count() << " (should be 0 - invalid storage)\n";
        std::cout << "  copyOfTemp.Count(): " << copyOfTemp.Count() << " (should be 0 - invalid storage)\n";
        std::cout << "  tempList.isValid(): " << (tempList.isValid() ? "true" : "false") << "\n";
        std::cout << "  copyOfTemp.isValid(): " << (copyOfTemp.isValid() ? "true" : "false") << "\n";
    }
    
    // Test 8: Iterator support
    std::cout << "\nTest 8: Iterator support\n";
    std::cout << "Using range-based for loop: ";
    for (const int& value : list1) {
        std::cout << value << " ";
    }
    std::cout << "\n";
    
    // Test 9: Memory efficiency comparison
    std::cout << "\nTest 9: Memory efficiency\n";
    std::cout << "Size of old List<int>: " << sizeof(int*) + sizeof(int) + sizeof(int) << " bytes (3 pointers)\n";
    std::cout << "Size of List<int>: " << sizeof(List<int>) << " bytes (should be 5: poolNum + index)\n";
    std::cout << "Size of MemRef: " << sizeof(MemRef) << " bytes\n";
    
    std::cout << "\nAll tests completed!\n";
    
    // Clean up pools
    MemPoolManager::destroyAllPools();
    
    return 0;
}