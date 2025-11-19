// test_CS_List.cpp - Unit tests for CS_List class
#include "../cpp/core/CS_List.h"
#include <iostream>
#include <cassert>

int testCount = 0;
int passCount = 0;

void test(const char* name, bool condition) {
    testCount++;
    if (condition) {
        passCount++;
        std::cout << "✓ " << name << std::endl;
    } else {
        std::cout << "✗ " << name << " FAILED" << std::endl;
    }
}

void testConstructors() {
    std::cout << "\n=== Constructor Tests ===" << std::endl;

    // Default constructor
    List<int> list1;
    test("Default constructor creates empty list", list1.Count() == 0);
    test("Default constructor Empty() returns true", list1.Empty());

    // Constructor with poolNum (should work but ignore poolNum)
    List<int> list2(5);
    test("Constructor with poolNum creates empty list", list2.Count() == 0);

    // Initializer list constructor
    List<int> list3 = {1, 2, 3, 4, 5};
    test("Initializer list constructor count", list3.Count() == 5);
    test("Initializer list values", list3[0] == 1 && list3[4] == 5);

    // Copy constructor
    List<int> list4(list3);
    test("Copy constructor count", list4.Count() == 5);
    test("Copy constructor values", list4[0] == 1 && list4[4] == 5);

    // Assignment operator
    List<int> list5;
    list5 = list3;
    test("Assignment operator count", list5.Count() == 5);
    test("Assignment operator values", list5[0] == 1 && list5[4] == 5);
}

void testAddAndAccess() {
    std::cout << "\n=== Add and Access Tests ===" << std::endl;

    List<int> list;

    // Add elements
    list.Add(10);
    test("Add single element count", list.Count() == 1);
    test("Add single element value", list[0] == 10);

    list.Add(20);
    list.Add(30);
    test("Add multiple elements count", list.Count() == 3);
    test("Access via operator[]", list[1] == 20);

    // At method
    test("At method valid index", list.At(2) == 30);
    test("At method invalid index returns default", list.At(10) == 0);
    test("At method negative index returns default", list.At(-1) == 0);

    // Empty check
    test("Non-empty list Empty() returns false", !list.Empty());
}

void testAddRange() {
    std::cout << "\n=== AddRange Tests ===" << std::endl;

    List<int> list1 = {1, 2, 3};
    List<int> list2 = {4, 5, 6};

    list1.AddRange(list2);
    test("AddRange count", list1.Count() == 6);
    test("AddRange preserves first list", list1[0] == 1 && list1[2] == 3);
    test("AddRange appends second list", list1[3] == 4 && list1[5] == 6);

    // AddRange empty list
    List<int> list3 = {7, 8};
    List<int> empty;
    list3.AddRange(empty);
    test("AddRange with empty list", list3.Count() == 2);
}

void testContainsAndIndexOf() {
    std::cout << "\n=== Contains and IndexOf Tests ===" << std::endl;

    List<int> list = {10, 20, 30, 20, 40};

    // Contains
    test("Contains existing element", list.Contains(30));
    test("Contains non-existing element", !list.Contains(99));

    // IndexOf
    test("IndexOf first occurrence", list.IndexOf(20) == 1);
    test("IndexOf non-existing", list.IndexOf(99) == -1);

    // LastIndexOf
    test("LastIndexOf last occurrence", list.LastIndexOf(20) == 3);
    test("LastIndexOf non-existing", list.LastIndexOf(99) == -1);
    test("LastIndexOf single occurrence", list.LastIndexOf(40) == 4);
}

void testInsert() {
    std::cout << "\n=== Insert Tests ===" << std::endl;

    List<int> list = {1, 2, 4, 5};

    // Insert in middle
    list.Insert(2, 3);
    test("Insert in middle count", list.Count() == 5);
    test("Insert in middle value", list[2] == 3);
    test("Insert in middle shifts elements", list[3] == 4);

    // Insert at beginning
    list.Insert(0, 0);
    test("Insert at beginning", list[0] == 0 && list.Count() == 6);

    // Insert at end
    list.Insert(6, 6);
    test("Insert at end", list[6] == 6 && list.Count() == 7);
}

void testRemove() {
    std::cout << "\n=== Remove Tests ===" << std::endl;

    List<int> list = {10, 20, 30, 20, 40};

    // Remove first occurrence
    bool removed = list.Remove(20);
    test("Remove returns true on success", removed);
    test("Remove count after removal", list.Count() == 4);
    test("Remove removes first occurrence", list[1] == 30);
    test("Remove keeps second occurrence", list.Contains(20));

    // Remove non-existing
    bool notRemoved = list.Remove(99);
    test("Remove returns false for non-existing", !notRemoved);
    test("Remove count unchanged", list.Count() == 4);
}

void testRemoveAt() {
    std::cout << "\n=== RemoveAt Tests ===" << std::endl;

    List<int> list = {1, 2, 3, 4, 5};

    list.RemoveAt(2);
    test("RemoveAt count", list.Count() == 4);
    test("RemoveAt shifts elements", list[2] == 4);

    // RemoveAt invalid index (should do nothing)
    list.RemoveAt(10);
    test("RemoveAt invalid index count unchanged", list.Count() == 4);

    list.RemoveAt(-1);
    test("RemoveAt negative index count unchanged", list.Count() == 4);
}

void testRemoveRange() {
    std::cout << "\n=== RemoveRange Tests ===" << std::endl;

    List<int> list = {1, 2, 3, 4, 5, 6, 7};

    list.RemoveRange(2, 3);  // Remove elements 3, 4, 5
    test("RemoveRange count", list.Count() == 4);
    test("RemoveRange removes correct elements", list[2] == 6);

    // RemoveRange at end
    List<int> list2 = {1, 2, 3, 4, 5};
    list2.RemoveRange(3, 10);  // Should remove to end
    test("RemoveRange beyond end", list2.Count() == 3);
}

void testClear() {
    std::cout << "\n=== Clear Tests ===" << std::endl;

    List<int> list = {1, 2, 3, 4, 5};

    list.Clear();
    test("Clear empties list", list.Count() == 0);
    test("Clear makes list empty", list.Empty());

    // Can add after clear
    list.Add(10);
    test("Add after clear", list.Count() == 1 && list[0] == 10);
}

void testReverse() {
    std::cout << "\n=== Reverse Tests ===" << std::endl;

    List<int> list = {1, 2, 3, 4, 5};

    list.Reverse();
    test("Reverse count unchanged", list.Count() == 5);
    test("Reverse first element", list[0] == 5);
    test("Reverse last element", list[4] == 1);
    test("Reverse middle element", list[2] == 3);

    // Reverse empty list
    List<int> empty;
    empty.Reverse();
    test("Reverse empty list", empty.Count() == 0);

    // Reverse single element
    List<int> single = {42};
    single.Reverse();
    test("Reverse single element", single[0] == 42);
}

void testSort() {
    std::cout << "\n=== Sort Tests ===" << std::endl;

    List<int> list = {5, 2, 8, 1, 9, 3};

    list.Sort();
    test("Sort count unchanged", list.Count() == 6);
    test("Sort ascending order", list[0] == 1 && list[5] == 9);
    test("Sort all elements sorted",
         list[0] < list[1] && list[1] < list[2] &&
         list[2] < list[3] && list[3] < list[4] &&
         list[4] < list[5]);

    // Sort already sorted
    list.Sort();
    test("Sort already sorted list", list[0] == 1 && list[5] == 9);

    // Sort empty
    List<int> empty;
    empty.Sort();
    test("Sort empty list", empty.Count() == 0);
}

void testCapacity() {
    std::cout << "\n=== Capacity Tests ===" << std::endl;

    List<int> list;

    test("Empty list capacity >= 0", list.Capacity() >= 0);

    // Add elements and check capacity grows
    for (int i = 0; i < 100; i++) {
        list.Add(i);
    }

    test("Capacity after many adds", list.Capacity() >= 100);
    test("Count correct after many adds", list.Count() == 100);
}

void testIterators() {
    std::cout << "\n=== Iterator Tests ===" << std::endl;

    List<int> list = {1, 2, 3, 4, 5};

    // Range-based for loop
    int sum = 0;
    for (int val : list) {
        sum += val;
    }
    test("Range-based for loop", sum == 15);

    // Manual iteration with begin/end
    int count = 0;
    for (int* it = list.begin(); it != list.end(); ++it) {
        count++;
    }
    test("Manual iterator count", count == 5);

    // Empty list iterators
    List<int> empty;
    test("Empty list begin == end", empty.begin() == empty.end());
}

void testToArrayAndAsArray() {
    std::cout << "\n=== ToArray/AsArray Tests ===" << std::endl;

    List<int> list = {10, 20, 30};

    // ToArrayCopy
    int* copy = list.ToArrayCopy();
    test("ToArrayCopy not null", copy != nullptr);
    test("ToArrayCopy values", copy[0] == 10 && copy[2] == 30);
    free(copy);

    // AsArray
    int* array = list.AsArray();
    test("AsArray not null", array != nullptr);
    test("AsArray values", array[0] == 10 && array[2] == 30);

    // Modify via AsArray
    array[1] = 99;
    test("AsArray modifies original", list[1] == 99);

    // Empty list
    List<int> empty;
    test("ToArrayCopy on empty list", empty.ToArrayCopy() == nullptr);
}

void testBoolConversion() {
    std::cout << "\n=== Bool Conversion Tests ===" << std::endl;

    List<int> empty;
    List<int> notEmpty = {1};

    test("Empty list converts to false", !empty);
    test("Non-empty list converts to true", (bool)notEmpty);
}

void testStringList() {
    std::cout << "\n=== String List Tests ===" << std::endl;

    // Test with a complex type (if String is available)
    List<const char*> strList;
    strList.Add("hello");
    strList.Add("world");

    test("String list count", strList.Count() == 2);
    test("String list values",
         strcmp(strList[0], "hello") == 0 &&
         strcmp(strList[1], "world") == 0);

    test("String list contains", strList.Contains("hello"));
    test("String list IndexOf", strList.IndexOf("world") == 1);
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "   CS_List Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    testConstructors();
    testAddAndAccess();
    testAddRange();
    testContainsAndIndexOf();
    testInsert();
    testRemove();
    testRemoveAt();
    testRemoveRange();
    testClear();
    testReverse();
    testSort();
    testCapacity();
    testIterators();
    testToArrayAndAsArray();
    testBoolConversion();
    testStringList();

    std::cout << "\n==================================" << std::endl;
    std::cout << "Results: " << passCount << "/" << testCount << " tests passed" << std::endl;

    if (passCount == testCount) {
        std::cout << "✓ All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ " << (testCount - passCount) << " test(s) failed" << std::endl;
        return 1;
    }
}
