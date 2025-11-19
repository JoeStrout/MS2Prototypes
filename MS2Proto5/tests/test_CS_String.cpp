// test_CS_String.cpp - Unit tests for CS_String class
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
    String s1;
    test("Default constructor creates empty string", s1.Length() == 0);

    // C-string constructor
    String s2("hello");
    test("C-string constructor", s2.Length() == 5);
    test("C-string content correct", strcmp(s2.c_str(), "hello") == 0);

    // Single char constructor
    String s3('x');
    test("Char constructor", s3.Length() == 1);
    test("Char content correct", strcmp(s3.c_str(), "x") == 0);

    // Copy constructor
    String s4(s2);
    test("Copy constructor", s4.Length() == 5);
    test("Copy content correct", strcmp(s4.c_str(), "hello") == 0);

    // Assignment operator
    String s5;
    s5 = s2;
    test("Assignment operator", s5.Length() == 5);
    test("Assignment content correct", strcmp(s5.c_str(), "hello") == 0);
}

void testConcatenation() {
    std::cout << "\n=== Concatenation Tests ===" << std::endl;

    String s1("hello");
    String s2(" world");

    // operator+
    String s3 = s1 + s2;
    test("String concatenation operator+", strcmp(s3.c_str(), "hello world") == 0);
    test("operator+ result length", s3.Length() == 11);

    // operator+=
    String s4("foo");
    s4 += String("bar");
    test("String concatenation operator+=", strcmp(s4.c_str(), "foobar") == 0);

    // Char concatenation
    String s5("test");
    s5 += '!';
    test("Char concatenation", strcmp(s5.c_str(), "test!") == 0);

    // Empty string concatenation
    String empty;
    String s6 = s1 + empty;
    test("Concat with empty string", strcmp(s6.c_str(), "hello") == 0);

    String s7 = empty + s1;
    test("Empty string concat", strcmp(s7.c_str(), "hello") == 0);
}

void testComparison() {
    std::cout << "\n=== Comparison Tests ===" << std::endl;

    String s1("apple");
    String s2("apple");
    String s3("banana");
    String empty1;
    String empty2;

    // Equality
    test("String equality (equal)", s1 == s2);
    test("String equality (not equal)", !(s1 == s3));
    test("Empty strings equal", empty1 == empty2);

    // Inequality
    test("String inequality", s1 != s3);
    test("String inequality (equal)", !(s1 != s2));

    // C-string comparison
    test("C-string equality", s1 == "apple");
    test("C-string inequality", s1 != "banana");

    // Relational operators
    test("Less than", s1 < s3);
    test("Greater than", s3 > s1);
    test("Less than or equal (less)", s1 <= s3);
    test("Less than or equal (equal)", s1 <= s2);
    test("Greater than or equal (greater)", s3 >= s1);
    test("Greater than or equal (equal)", s1 >= s2);
}

void testSubstring() {
    std::cout << "\n=== Substring Tests ===" << std::endl;

    String s("hello world");

    String sub1 = s.Substring(0, 5);
    test("Substring from start", strcmp(sub1.c_str(), "hello") == 0);

    String sub2 = s.Substring(6, 5);
    test("Substring from middle", strcmp(sub2.c_str(), "world") == 0);

    String sub3 = s.Substring(6);
    test("Substring to end", strcmp(sub3.c_str(), "world") == 0);

    String sub4 = s.Substring(0);
    test("Substring entire string", strcmp(sub4.c_str(), "hello world") == 0);
}

void testIndexOf() {
    std::cout << "\n=== IndexOf Tests ===" << std::endl;

    String s("hello world");

    test("IndexOf found", s.IndexOf('o') == 4);
    test("IndexOf not found", s.IndexOf('x') == -1);
    test("IndexOf string found", s.IndexOf(String("world")) == 6);
    test("IndexOf string not found", s.IndexOf(String("xyz")) == -1);
    test("IndexOf empty string", s.IndexOf(String("")) == 0);
}

void testReplace() {
    std::cout << "\n=== Replace Tests ===" << std::endl;

    String s("hello world");

    String r1 = s.Replace(String("world"), String("universe"));
    test("Replace substring", strcmp(r1.c_str(), "hello universe") == 0);

    String r2 = s.Replace(String("o"), String("0"));
    test("Replace all occurrences", strcmp(r2.c_str(), "hell0 w0rld") == 0);

    String r3 = s.Replace(String("xyz"), String("abc"));
    test("Replace non-existent", strcmp(r3.c_str(), "hello world") == 0);
}

void testSplit() {
    std::cout << "\n=== Split Tests ===" << std::endl;

    String s("apple,banana,cherry");
    std::vector<String> parts = s.Split(',');

    test("Split count", parts.size() == 3);
    test("Split part 1", strcmp(parts[0].c_str(), "apple") == 0);
    test("Split part 2", strcmp(parts[1].c_str(), "banana") == 0);
    test("Split part 3", strcmp(parts[2].c_str(), "cherry") == 0);

    // Split on non-existent char
    String s2("hello");
    std::vector<String> parts2 = s2.Split(',');
    test("Split no delimiter", parts2.size() == 1);
    test("Split no delimiter content", strcmp(parts2[0].c_str(), "hello") == 0);
}

void testTrimAndCase() {
    std::cout << "\n=== Trim and Case Tests ===" << std::endl;

    // ToUpper / ToLower
    String s1("Hello World");
    String upper = s1.ToUpper();
    String lower = s1.ToLower();
    test("ToUpper", strcmp(upper.c_str(), "HELLO WORLD") == 0);
    test("ToLower", strcmp(lower.c_str(), "hello world") == 0);

    // Trim
    String s2("  hello  ");
    String trimmed = s2.Trim();
    test("Trim", strcmp(trimmed.c_str(), "hello") == 0);

    String s3("  hello");
    String trimStart = s3.TrimStart();
    test("TrimStart", strcmp(trimStart.c_str(), "hello") == 0);

    String s4("hello  ");
    String trimEnd = s4.TrimEnd();
    test("TrimEnd", strcmp(trimEnd.c_str(), "hello") == 0);
}

void testStartsWithEndsWith() {
    std::cout << "\n=== StartsWith/EndsWith Tests ===" << std::endl;

    String s("hello world");

    test("StartsWith true", s.StartsWith(String("hello")));
    test("StartsWith false", !s.StartsWith(String("world")));
    test("StartsWith empty", s.StartsWith(String("")));

    test("EndsWith true", s.EndsWith(String("world")));
    test("EndsWith false", !s.EndsWith(String("hello")));
    test("EndsWith empty", s.EndsWith(String("")));
}

void testContains() {
    std::cout << "\n=== Contains Test ===" << std::endl;

    String s("hello world");

    test("Contains true", s.Contains(String("lo wo")));
    test("Contains false", !s.Contains(String("xyz")));
    test("Contains empty", s.Contains(String("")));
}

void testEmptyAndLength() {
    std::cout << "\n=== Empty and Length Tests ===" << std::endl;

    String empty;
    String notEmpty("hello");

    test("Empty string is empty", empty.Empty());
    test("Non-empty string not empty", !notEmpty.Empty());
    test("Empty string length 0", empty.Length() == 0);
    test("Non-empty string length", notEmpty.Length() == 5);
}

void testUnicode() {
    std::cout << "\n=== Unicode Tests ===" << std::endl;

    // UTF-8 multi-byte characters
    String s("Hello 世界");  // "Hello World" in Chinese

    // Length in characters should differ from byte length
    int byteLen = s.lengthB();
    int charLen = s.lengthC();

    test("Unicode byte length", byteLen > charLen);
    test("Unicode char count", charLen == 8);  // "Hello " = 6 + 2 Chinese chars
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "   CS_String Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    testConstructors();
    testConcatenation();
    testComparison();
    testSubstring();
    testIndexOf();
    testReplace();
    testSplit();
    testTrimAndCase();
    testStartsWithEndsWith();
    testContains();
    testEmptyAndLength();
    testUnicode();

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
