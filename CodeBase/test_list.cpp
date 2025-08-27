#include "List.h"
#include "string.h"
#include <iostream>
#include <cassert>

// Test predicate function for RemoveAll
bool isEven(const int& value) {
    return value % 2 == 0;
}

// Test comparer function for Sort
int compareInts(const int& a, const int& b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int main() {
    std::cout << "Testing C++ List<T> template class\n\n";
    
    // Test 1: Basic construction and properties
    std::cout << "Test 1: Basic construction and properties\n";
    List<int> list1;
    std::cout << "Empty list count: " << list1.Count() << "\n";
    std::cout << "Empty list capacity: " << list1.Capacity() << "\n";
    std::cout << "Empty list is empty: " << (list1.Empty() ? "true" : "false") << "\n";
    
    List<int> list2(10);  // With initial capacity
    std::cout << "List with initial capacity count: " << list2.Count() << "\n";
    std::cout << "List with initial capacity: " << list2.Capacity() << "\n";
    
    // Test 2: Adding elements
    std::cout << "\nTest 2: Adding elements\n";
    list1.Add(10);
    list1.Add(20);
    list1.Add(30);
    std::cout << "After adding 3 elements, count: " << list1.Count() << "\n";
    std::cout << "Elements: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 3: Element access
    std::cout << "\nTest 3: Element access\n";
    std::cout << "list1[0]: " << list1[0] << "\n";
    std::cout << "list1[1]: " << list1[1] << "\n";
    std::cout << "list1.At(2): " << list1.At(2) << "\n";
    std::cout << "list1.First(): " << list1.First() << "\n";
    std::cout << "list1.Last(): " << list1.Last() << "\n";
    
    // Test 4: Copy constructor and assignment
    std::cout << "\nTest 4: Copy constructor and assignment\n";
    List<int> list3(list1);
    std::cout << "Copy constructor - list3 count: " << list3.Count() << "\n";
    std::cout << "list3 elements: ";
    for (int i = 0; i < list3.Count(); i++) {
        std::cout << list3[i] << " ";
    }
    std::cout << "\n";
    
    List<int> list4;
    list4 = list1;
    std::cout << "Assignment operator - list4 count: " << list4.Count() << "\n";
    
    // Test 5: Insertion
    std::cout << "\nTest 5: Insertion\n";
    list1.Insert(1, 15);  // Insert 15 at position 1
    std::cout << "After inserting 15 at position 1: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 6: Searching
    std::cout << "\nTest 6: Searching\n";
    std::cout << "IndexOf(20): " << list1.IndexOf(20) << "\n";
    std::cout << "IndexOf(99): " << list1.IndexOf(99) << "\n";
    std::cout << "Contains(15): " << (list1.Contains(15) ? "true" : "false") << "\n";
    std::cout << "Contains(99): " << (list1.Contains(99) ? "true" : "false") << "\n";
    std::cout << "LastIndexOf(20): " << list1.LastIndexOf(20) << "\n";
    
    // Test 7: Removal
    std::cout << "\nTest 7: Removal\n";
    std::cout << "Before removal: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    list1.Remove(15);
    std::cout << "After removing 15: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    list1.RemoveAt(0);
    std::cout << "After removing at index 0: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 8: AddRange and InsertRange
    std::cout << "\nTest 8: AddRange and InsertRange\n";
    List<int> range;
    range.Add(100);
    range.Add(200);
    
    list1.AddRange(range);
    std::cout << "After AddRange: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    List<int> insertRange;
    insertRange.Add(50);
    insertRange.Add(60);
    
    list1.InsertRange(1, insertRange);
    std::cout << "After InsertRange at position 1: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 9: RemoveRange and RemoveAll
    std::cout << "\nTest 9: RemoveRange and RemoveAll\n";
    list1.RemoveRange(1, 2);  // Remove 2 elements starting at index 1
    std::cout << "After RemoveRange(1, 2): ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Add some even numbers for RemoveAll test
    list1.Add(2);
    list1.Add(4);
    list1.Add(6);
    std::cout << "Before RemoveAll (evens): ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    int removedCount = list1.RemoveAll(isEven);
    std::cout << "Removed " << removedCount << " even numbers: ";
    for (int i = 0; i < list1.Count(); i++) {
        std::cout << list1[i] << " ";
    }
    std::cout << "\n";
    
    // Test 10: Reverse and Sort
    std::cout << "\nTest 10: Reverse and Sort\n";
    List<int> sortList;
    sortList.Add(5);
    sortList.Add(2);
    sortList.Add(8);
    sortList.Add(1);
    sortList.Add(9);
    
    std::cout << "Original: ";
    for (int i = 0; i < sortList.Count(); i++) {
        std::cout << sortList[i] << " ";
    }
    std::cout << "\n";
    
    sortList.Reverse();
    std::cout << "After Reverse(): ";
    for (int i = 0; i < sortList.Count(); i++) {
        std::cout << sortList[i] << " ";
    }
    std::cout << "\n";
    
    sortList.Sort();
    std::cout << "After Sort(): ";
    for (int i = 0; i < sortList.Count(); i++) {
        std::cout << sortList[i] << " ";
    }
    std::cout << "\n";
    
    sortList.Reverse();
    sortList.Sort(compareInts);
    std::cout << "After Sort with comparer: ";
    for (int i = 0; i < sortList.Count(); i++) {
        std::cout << sortList[i] << " ";
    }
    std::cout << "\n";
    
    // Test 11: GetRange
    std::cout << "\nTest 11: GetRange\n";
    List<int> subList = sortList.GetRange(1, 3);
    std::cout << "GetRange(1, 3): ";
    for (int i = 0; i < subList.Count(); i++) {
        std::cout << subList[i] << " ";
    }
    std::cout << "\n";
    
    // Test 12: ToArray
    std::cout << "\nTest 12: ToArray\n";
    int arraySize;
    int* array = sortList.ToArray(&arraySize);
    std::cout << "ToArray (size " << arraySize << "): ";
    for (int i = 0; i < arraySize; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << "\n";
    
    // Clean up array (caller responsibility)
    // Note: for simple types like int, no destructor call needed
    free(array);
    
    // Test 13: Capacity management
    std::cout << "\nTest 13: Capacity management\n";
    List<int> capacityTest;
    std::cout << "Initial capacity: " << capacityTest.Capacity() << "\n";
    
    for (int i = 0; i < 10; i++) {
        capacityTest.Add(i);
        std::cout << "Count: " << capacityTest.Count() << ", Capacity: " << capacityTest.Capacity() << "\n";
    }
    
    capacityTest.TrimExcess();
    std::cout << "After TrimExcess - Count: " << capacityTest.Count() << ", Capacity: " << capacityTest.Capacity() << "\n";
    
    capacityTest.SetCapacity(20);
    std::cout << "After SetCapacity(20) - Capacity: " << capacityTest.Capacity() << "\n";
    
    // Test 14: Clear
    std::cout << "\nTest 14: Clear\n";
    std::cout << "Before clear - Count: " << capacityTest.Count() << "\n";
    capacityTest.Clear();
    std::cout << "After clear - Count: " << capacityTest.Count() << "\n";
    std::cout << "After clear - Empty: " << (capacityTest.Empty() ? "true" : "false") << "\n";
    
    // Test 15: Working with string class
    std::cout << "\nTest 15: Working with string class\n";
    List<string> stringList;
    stringList.Add("Hello");
    stringList.Add("World");
    stringList.Add("List");
    stringList.Add("Template");
    
    std::cout << "String list: ";
    for (int i = 0; i < stringList.Count(); i++) {
        std::cout << "'" << stringList[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    std::cout << "Contains('World'): " << (stringList.Contains(string("World")) ? "true" : "false") << "\n";
    std::cout << "IndexOf('List'): " << stringList.IndexOf(string("List")) << "\n";
    
    stringList.Remove(string("World"));
    std::cout << "After removing 'World': ";
    for (int i = 0; i < stringList.Count(); i++) {
        std::cout << "'" << stringList[i].c_str() << "' ";
    }
    std::cout << "\n";
    
    // Test 16: Iterator support
    std::cout << "\nTest 16: Iterator support\n";
    List<int> iterList;
    for (int i = 1; i <= 5; i++) {
        iterList.Add(i * 10);
    }
    
    std::cout << "Using iterators: ";
    for (const int* it = iterList.begin(); it != iterList.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    // Test using range-based for loop (C++11)
    std::cout << "Using range-based for: ";
    for (const int& value : iterList) {
        std::cout << value << " ";
    }
    std::cout << "\n";
    
    // Test 17: Constructor that adopts array (perfect for string.Split)
    std::cout << "\nTest 17: Constructor that adopts array (perfect for string.Split)\n";
    string csv2("red,green,blue,yellow");
    std::cout << "CSV: '" << csv2.c_str() << "'\n";
    
    int splitCount;
    string* splitArray = csv2.Split(',', &splitCount);
    
    // Create List by adopting the array from Split
    List<string> colorList(splitArray, splitCount);  // Takes ownership of splitArray
    
    std::cout << "List from adopted array (" << colorList.Count() << " items):\n";
    for (int i = 0; i < colorList.Count(); i++) {
        std::cout << "  [" << i << "]: '" << colorList[i].c_str() << "'\n";
    }
    
    // Test that the list is fully functional
    colorList.Add("purple");
    std::cout << "After adding 'purple': " << colorList.Count() << " items\n";
    std::cout << "Contains('blue'): " << (colorList.Contains(string("blue")) ? "true" : "false") << "\n";
    
    // No need to free splitArray - the List now owns it!
    
    // Test 18: string::Join static methods
    std::cout << "\nTest 18: string::Join static methods\n";
    
    // Test Join with array
    string fruits[] = {"apple", "banana", "cherry", "date"};
    string joined1 = string::Join(", ", fruits, 4);
    std::cout << "Joined array: '" << joined1.c_str() << "'\n";
    
    string joined2 = string::Join(" | ", fruits, 4);
    std::cout << "Joined with pipe: '" << joined2.c_str() << "'\n";
    
    // Test Join with List<string>
    string joined3 = ListStringJoin::Join(", ", colorList);
    std::cout << "Joined color list: '" << joined3.c_str() << "'\n";
    
    // Test edge cases
    string empty = string::Join(", ", fruits, 0);  // Empty array
    std::cout << "Join empty array: '" << empty.c_str() << "' (should be empty)\n";
    
    string single = string::Join(", ", fruits, 1);  // Single element
    std::cout << "Join single element: '" << single.c_str() << "'\n";
    
    // Test round-trip: Split then Join
    string original("red,green,blue");
    int parts_count;
    string* parts = original.Split(',', &parts_count);
    List<string> partsList(parts, parts_count);  // Adopt the array
    string rejoined = ListStringJoin::Join(",", partsList);
    std::cout << "Round-trip test:\n";
    std::cout << "  Original: '" << original.c_str() << "'\n";
    std::cout << "  Rejoined: '" << rejoined.c_str() << "'\n";
    std::cout << "  Equal: " << (original == rejoined ? "true" : "false") << "\n";
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}