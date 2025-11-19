// test_CS_Dictionary.cpp - Unit tests for CS_Dictionary class
#include "../cpp/core/CS_Dictionary.h"
#include "../cpp/core/CS_String.h"
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
    Dictionary<int, int> dict1;
    test("Default constructor creates empty dictionary", dict1.Count() == 0);
    test("Default constructor Empty() returns true", dict1.Empty());

    // Constructor with poolNum (should work but ignore poolNum)
    Dictionary<int, int> dict2(5);
    test("Constructor with poolNum creates empty dictionary", dict2.Count() == 0);

    // Copy constructor
    Dictionary<int, int> dict3;
    dict3.Add(1, 100);
    dict3.Add(2, 200);
    Dictionary<int, int> dict4(dict3);
    test("Copy constructor count", dict4.Count() == 2);
    test("Copy constructor values", dict4[1] == 100 && dict4[2] == 200);

    // Assignment operator
    Dictionary<int, int> dict5;
    dict5 = dict3;
    test("Assignment operator count", dict5.Count() == 2);
    test("Assignment operator values", dict5[1] == 100 && dict5[2] == 200);
}

void testAddAndAccess() {
    std::cout << "\n=== Add and Access Tests ===" << std::endl;

    Dictionary<int, int> dict;

    // Add single element
    dict.Add(1, 100);
    test("Add single element count", dict.Count() == 1);
    test("Add single element value via operator[]", dict[1] == 100);

    // Add multiple elements
    dict.Add(2, 200);
    dict.Add(3, 300);
    test("Add multiple elements count", dict.Count() == 3);
    test("Access via operator[] key 2", dict[2] == 200);
    test("Access via operator[] key 3", dict[3] == 300);

    // Update existing key
    dict.Add(2, 250);
    test("Update existing key count unchanged", dict.Count() == 3);
    test("Update existing key new value", dict[2] == 250);

    // Empty check
    test("Non-empty dictionary Empty() returns false", !dict.Empty());
}

void testOperatorBracket() {
    std::cout << "\n=== Operator[] Tests ===" << std::endl;

    Dictionary<int, int> dict;

    // Access non-existing key should add with default value
    int value = dict[1];
    test("Access non-existing key returns default", value == 0);
    test("Access non-existing key adds to dictionary", dict.Count() == 1);

    // Assign via operator[]
    dict[2] = 200;
    test("Assign via operator[] count", dict.Count() == 2);
    test("Assign via operator[] value", dict[2] == 200);

    // Update via operator[]
    dict[2] = 250;
    test("Update via operator[] count unchanged", dict.Count() == 2);
    test("Update via operator[] new value", dict[2] == 250);

    // Const dictionary operator[]
    const Dictionary<int, int> constDict = dict;
    test("Const operator[] returns correct value", constDict[2] == 250);
    test("Const operator[] non-existing returns default", constDict[999] == 0);
}

void testContainsKey() {
    std::cout << "\n=== ContainsKey Tests ===" << std::endl;

    Dictionary<int, int> dict;
    dict.Add(1, 100);
    dict.Add(2, 200);
    dict.Add(3, 300);

    test("ContainsKey existing key 1", dict.ContainsKey(1));
    test("ContainsKey existing key 2", dict.ContainsKey(2));
    test("ContainsKey existing key 3", dict.ContainsKey(3));
    test("ContainsKey non-existing key", !dict.ContainsKey(99));

    // Empty dictionary
    Dictionary<int, int> empty;
    test("ContainsKey on empty dictionary", !empty.ContainsKey(1));
}

void testTryGetValue() {
    std::cout << "\n=== TryGetValue Tests ===" << std::endl;

    Dictionary<int, int> dict;
    dict.Add(1, 100);
    dict.Add(2, 200);

    // Existing key
    int value;
    bool found = dict.TryGetValue(1, &value);
    test("TryGetValue existing key returns true", found);
    test("TryGetValue existing key sets value", value == 100);

    // Non-existing key
    found = dict.TryGetValue(99, &value);
    test("TryGetValue non-existing key returns false", !found);

    // Empty dictionary
    Dictionary<int, int> empty;
    found = empty.TryGetValue(1, &value);
    test("TryGetValue on empty dictionary returns false", !found);
}

void testRemove() {
    std::cout << "\n=== Remove Tests ===" << std::endl;

    Dictionary<int, int> dict;
    dict.Add(1, 100);
    dict.Add(2, 200);
    dict.Add(3, 300);

    // Remove existing key
    bool removed = dict.Remove(2);
    test("Remove returns true on success", removed);
    test("Remove count after removal", dict.Count() == 2);
    test("Remove key no longer exists", !dict.ContainsKey(2));
    test("Remove other keys still exist", dict.ContainsKey(1) && dict.ContainsKey(3));

    // Remove non-existing key
    bool notRemoved = dict.Remove(99);
    test("Remove returns false for non-existing key", !notRemoved);
    test("Remove count unchanged", dict.Count() == 2);

    // Remove all keys
    dict.Remove(1);
    dict.Remove(3);
    test("Remove all keys makes dictionary empty", dict.Empty());
}

void testClear() {
    std::cout << "\n=== Clear Tests ===" << std::endl;

    Dictionary<int, int> dict;
    dict.Add(1, 100);
    dict.Add(2, 200);
    dict.Add(3, 300);

    dict.Clear();
    test("Clear empties dictionary", dict.Count() == 0);
    test("Clear makes dictionary empty", dict.Empty());
    test("Clear removes keys", !dict.ContainsKey(1));

    // Can add after clear
    dict.Add(10, 1000);
    test("Add after clear", dict.Count() == 1 && dict[10] == 1000);
}

void testResize() {
    std::cout << "\n=== Resize Tests ===" << std::endl;

    Dictionary<int, int> dict;

    // Add many elements to trigger resize
    for (int i = 0; i < 100; i++) {
        dict.Add(i, i * 10);
    }

    test("Resize count after many adds", dict.Count() == 100);

    // Verify all elements are still accessible
    bool allPresent = true;
    for (int i = 0; i < 100; i++) {
        if (!dict.ContainsKey(i) || dict[i] != i * 10) {
            allPresent = false;
            break;
        }
    }
    test("Resize all elements accessible", allPresent);
}

void testCollisions() {
    std::cout << "\n=== Collision Tests ===" << std::endl;

    Dictionary<int, int> dict;

    // Add elements that might collide
    for (int i = 0; i < 20; i++) {
        dict.Add(i, i * 100);
    }

    test("Collisions count correct", dict.Count() == 20);

    // Verify all elements
    bool allCorrect = true;
    for (int i = 0; i < 20; i++) {
        if (dict[i] != i * 100) {
            allCorrect = false;
            break;
        }
    }
    test("Collisions all values correct", allCorrect);
}

void testKeyIteration() {
    std::cout << "\n=== Key Iteration Tests ===" << std::endl;

    Dictionary<int, int> dict;
    dict.Add(1, 100);
    dict.Add(2, 200);
    dict.Add(3, 300);

    // Iterate through keys
    int keyCount = 0;
    int keySum = 0;
    for (int key : dict.GetKeys()) {
        keyCount++;
        keySum += key;
    }

    test("Key iteration count", keyCount == 3);
    test("Key iteration sum", keySum == 6);  // 1 + 2 + 3

    // Empty dictionary iteration
    Dictionary<int, int> empty;
    int emptyCount = 0;
    for (int key : empty.GetKeys()) {
        (void)key;  // Suppress unused warning
        emptyCount++;
    }
    test("Empty dictionary iteration", emptyCount == 0);
}

void testStringKeys() {
    std::cout << "\n=== String Key Tests ===" << std::endl;

    Dictionary<String, int> dict;
    dict.Add(String("one"), 1);
    dict.Add(String("two"), 2);
    dict.Add(String("three"), 3);

    test("String key count", dict.Count() == 3);
    test("String key access", dict[String("two")] == 2);
    test("String key ContainsKey", dict.ContainsKey(String("three")));
    test("String key non-existing", !dict.ContainsKey(String("four")));

    // Remove with string key
    dict.Remove(String("two"));
    test("String key remove", dict.Count() == 2);
    test("String key after remove", !dict.ContainsKey(String("two")));
}

void testStringValues() {
    std::cout << "\n=== String Value Tests ===" << std::endl;

    Dictionary<int, String> dict;
    dict.Add(1, String("one"));
    dict.Add(2, String("two"));
    dict.Add(3, String("three"));

    test("String value count", dict.Count() == 3);
    test("String value access", dict[2] == String("two"));

    // Update string value
    dict[2] = String("TWO");
    test("String value update", dict[2] == String("TWO"));
}

void testIsValid() {
    std::cout << "\n=== IsValid Tests ===" << std::endl;

    Dictionary<int, int> dict1;
    test("Empty dictionary isValid", dict1.isValid());

    dict1.Add(1, 100);
    test("Non-empty dictionary isValid", dict1.isValid());
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "   CS_Dictionary Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    testConstructors();
    testAddAndAccess();
    testOperatorBracket();
    testContainsKey();
    testTryGetValue();
    testRemove();
    testClear();
    testResize();
    testCollisions();
    testKeyIteration();
    testStringKeys();
    testStringValues();
    testIsValid();

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
