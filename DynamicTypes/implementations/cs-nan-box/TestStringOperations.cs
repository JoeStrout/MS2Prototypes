using System;
using System.Diagnostics;

namespace ScriptingVM.Tests
{
    class TestStringOperations
    {
        static void TestStringSplit()
        {
            Console.WriteLine("Testing string split operations...");
            
            // Test character split
            var str = Value.FromString("abc");
            var emptyDelim = Value.FromString("");
            var charList = StringOperations.StringSplit(str, emptyDelim);
            
            Debug.Assert(charList.IsList);
            Debug.Assert(ListOperations.ListCount(charList) == 3);
            
            var item0 = ListOperations.ListGet(charList, 0);
            var item1 = ListOperations.ListGet(charList, 1);
            var item2 = ListOperations.ListGet(charList, 2);
            
            Debug.Assert(item0.IsString && item0.ToString() == "a");
            Debug.Assert(item1.IsString && item1.ToString() == "b");
            Debug.Assert(item2.IsString && item2.ToString() == "c");
            
            // Test word split
            var sentence = Value.FromString("hello world test");
            var space = Value.FromString(" ");
            var wordList = StringOperations.StringSplit(sentence, space);
            
            Debug.Assert(wordList.IsList);
            Debug.Assert(ListOperations.ListCount(wordList) == 3);
            
            var word0 = ListOperations.ListGet(wordList, 0);
            var word1 = ListOperations.ListGet(wordList, 1);
            var word2 = ListOperations.ListGet(wordList, 2);
            
            Debug.Assert(word0.ToString() == "hello");
            Debug.Assert(word1.ToString() == "world");
            Debug.Assert(word2.ToString() == "test");
            
            Console.WriteLine("✓ String split tests passed");
        }
        
        static void TestStringReplace()
        {
            Console.WriteLine("Testing string replace operations...");
            
            // Single replacement
            var str = Value.FromString("hello-world");
            var from = Value.FromString("-");
            var to = Value.FromString(" ");
            var result = StringOperations.StringReplace(str, from, to);
            
            Debug.Assert(result.IsString);
            Debug.Assert(result.ToString() == "hello world");
            
            // Multiple replacements
            var multiStr = Value.FromString("thirty-seven and sixty-four");
            var multiResult = StringOperations.StringReplace(multiStr, from, to);
            Debug.Assert(multiResult.ToString() == "thirty seven and sixty four");
            
            // No replacement case
            var noMatch = Value.FromString("hello world");
            var noMatchFrom = Value.FromString("foo");
            var noMatchResult = StringOperations.StringReplace(noMatch, noMatchFrom, to);
            Debug.Assert(Value.Equal(noMatchResult, noMatch)); // Should return original
            
            Console.WriteLine("✓ String replace tests passed");
        }
        
        static void TestStringIndexOf()
        {
            Console.WriteLine("Testing string indexOf operations...");
            
            var haystack = Value.FromString("hello world");
            var needle1 = Value.FromString("world");
            var needle2 = Value.FromString("foo");
            var needle3 = Value.FromString("o");
            
            var idx1 = StringOperations.StringIndexOf(haystack, needle1);
            var idx2 = StringOperations.StringIndexOf(haystack, needle2);
            var idx3 = StringOperations.StringIndexOf(haystack, needle3);
            
            Debug.Assert(idx1.AsInt() == 6);  // "world" at position 6
            Debug.Assert(idx2.AsInt() == -1); // "foo" not found
            Debug.Assert(idx3.AsInt() == 4);  // First "o" at position 4
            
            Console.WriteLine("✓ String indexOf tests passed");
        }
        
        static void TestStringConcat()
        {
            Console.WriteLine("Testing string concatenation...");
            
            var str1 = Value.FromString("hello");
            var str2 = Value.FromString(" world");
            var result = StringOperations.StringConcat(str1, str2);
            
            Debug.Assert(result.IsString);
            Debug.Assert(result.ToString() == "hello world");
            Debug.Assert(StringOperations.StringLength(result) == 11);
            
            Console.WriteLine("✓ String concat tests passed");
        }
        
        static void TestListOperations()
        {
            Console.WriteLine("Testing list operations...");
            
            var list = ListOperations.MakeList();
            Debug.Assert(list.IsList);
            Debug.Assert(ListOperations.ListCount(list) == 0);
            
            // Add items
            ListOperations.ListAdd(list, Value.FromString("first"));
            ListOperations.ListAdd(list, Value.FromString("second"));
            ListOperations.ListAdd(list, Value.FromString("third"));
            
            Debug.Assert(ListOperations.ListCount(list) == 3);
            
            // Get items
            var item0 = ListOperations.ListGet(list, 0);
            var item1 = ListOperations.ListGet(list, 1);
            var item2 = ListOperations.ListGet(list, 2);
            
            Debug.Assert(item0.ToString() == "first");
            Debug.Assert(item1.ToString() == "second");
            Debug.Assert(item2.ToString() == "third");
            
            // Test indexOf
            var searchItem = Value.FromString("second");
            int idx = ListOperations.ListIndexOf(list, searchItem);
            Debug.Assert(idx == 1);
            
            var notFound = Value.FromString("nothere");
            int notFoundIdx = ListOperations.ListIndexOf(list, notFound);
            Debug.Assert(notFoundIdx == -1);
            
            // Test set
            ListOperations.ListSet(list, 1, Value.FromString("modified"));
            var modifiedItem = ListOperations.ListGet(list, 1);
            Debug.Assert(modifiedItem.ToString() == "modified");
            
            Console.WriteLine("✓ List operation tests passed");
        }
        
        static void TestStringEquality()
        {
            Console.WriteLine("Testing string equality...");
            
            var str1 = Value.FromString("hello");
            var str2 = Value.FromString("hello");
            var str3 = Value.FromString("world");
            
            Debug.Assert(StringOperations.StringEquals(str1, str2));
            Debug.Assert(!StringOperations.StringEquals(str1, str3));
            
            Console.WriteLine("✓ String equality tests passed");
        }
        
        static void Main(string[] args)
        {
            Console.WriteLine("C# NaN Boxing String & List Operation Tests");
            Console.WriteLine("==========================================");
            Console.WriteLine();
            
            TestStringSplit();
            TestStringReplace();
            TestStringIndexOf();
            TestStringConcat();
            TestListOperations();
            TestStringEquality();
            
            Console.WriteLine();
            Console.WriteLine($"HandlePool usage: {HandlePool.GetCount()} objects");
            Console.WriteLine("🎉 All string and list operation tests passed!");
        }
    }
}