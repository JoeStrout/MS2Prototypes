using System;
using System.Diagnostics;

namespace ScriptingVM.Benchmarks
{
    class LevenshteinImproved
    {
        static Value EditDistance(Value s1_val, Value s2_val)
        {
            if (!s1_val.IsString || !s2_val.IsString)
                return Value.Null();
            
            int n = StringOperations.StringLength(s1_val);
            int m = StringOperations.StringLength(s2_val);
            
            // Handle edge cases
            if (n == 0)
                return Value.FromInt(m);
            if (m == 0)
                return Value.FromInt(n);
            
            // Split strings into character lists (matching C implementation)
            var emptyDelim = Value.FromString("");
            var s1chars = StringOperations.StringSplit(s1_val, emptyDelim);
            var s2chars = StringOperations.StringSplit(s2_val, emptyDelim);
            
            // Allocate distance array as a List
            var d_list = ListOperations.MakeList();
            
            // Initialize d array with range(0, m)
            for (int i = 0; i <= m; i++)
            {
                ListOperations.ListAdd(d_list, Value.FromInt(i));
            }
            
            int lastCost = 0;
            int nextCost = 0;
            
            // Main algorithm loop  
            for (int i = 1; i <= n; i++)
            {
                var s1char = ListOperations.ListGet(s1chars, i - 1);
                lastCost = i;
                int jMinus1 = 0;
                
                for (int j = 1; j <= m; j++)
                {
                    var s2char = ListOperations.ListGet(s2chars, jMinus1);
                    
                    // Calculate cost (0 if characters match, 1 if different)
                    int cost = StringOperations.StringEquals(s1char, s2char) ? 0 : 1;
                    
                    // Calculate the three possibilities
                    int a = ListOperations.ListGet(d_list, j).AsInt() + 1;           // deletion
                    int b = lastCost + 1;                                            // insertion
                    int c = cost + ListOperations.ListGet(d_list, jMinus1).AsInt();  // substitution
                    
                    // Find minimum using nested if statements (matching original)
                    if (a < b)
                    {
                        if (c < a)
                            nextCost = c;
                        else
                            nextCost = a;
                    }
                    else
                    {
                        if (c < b)
                            nextCost = c;
                        else
                            nextCost = b;
                    }
                    
                    ListOperations.ListSet(d_list, jMinus1, Value.FromInt(lastCost));
                    lastCost = nextCost;
                    jMinus1 = j;
                }
                ListOperations.ListSet(d_list, m, Value.FromInt(lastCost));
            }
            
            return Value.FromInt(nextCost);
        }
        
        static void RunTest()
        {
            // Test 1: "kitten" -> "sitting" = 3
            var s1 = Value.FromString("kitten");
            var s2 = Value.FromString("sitting");
            var result1 = EditDistance(s1, s2);
            
            // Test 2: "this is a test..." -> "that was a test..." 
            var s3 = Value.FromString("this is a test of a slightly longer string");
            var s4 = Value.FromString("that was a test of a slightly longer string");
            var result2 = EditDistance(s3, s4);
            
            // Test 3: Gettysburg Address variants
            var ga1 = Value.FromString(
"Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.  Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. " +
"But, in a larger sense, we can not dedicate--we can not consecrate--we can not hallow--this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. " +
"It is rather for us to be here dedicated to the great task remaining before us--that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion--that we here highly resolve that these dead shall not have died in vain--that this nation, under God, shall have a new birth of freedom--and that government of the people, by the people, for the people, shall not perish from the earth.");

            var ga2 = Value.FromString(
"Eighty seven years ago our ancestors brought forth in these parts, a new nation, conceived in freedom, and dedicated to the proposition that all people are created equal.  Now we are engaged in a lousy civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are gathered on a famous battlefield of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is super groovy and cool that we should do this. " +
"But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or subtract. The world will little note, nor long remember what we say here (ha ha as if), but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. " +
"It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, with its constitutionally guaranteed separation of church and state, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not disappear from Earth.");
    
            var result3 = EditDistance(ga1, ga2);
            
            // Test 4: Very different strings
            var banana = Value.FromString("banana");
            var result4 = EditDistance(ga1, banana);
            
            // Store results (we'll print them for verification)
            Console.WriteLine("Test results:");
            Console.WriteLine($"\"kitten\" -> \"sitting\": {result1.AsInt()}");
            Console.WriteLine($"Short sentence test: {result2.AsInt()}");
            Console.WriteLine($"Gettysburg variants: {result3.AsInt()}");
            Console.WriteLine($"GA1 -> \"banana\": {result4.AsInt()}");
        }
        
        static void RunBenchmark()
        {
            Console.WriteLine("Running levenshtein benchmark...");
            var stopwatch = Stopwatch.StartNew();
            RunTest();
            stopwatch.Stop();
            Console.WriteLine($"levenshtein time: {stopwatch.ElapsedMilliseconds / 1000.0:F3} seconds");
        }
        
        static void RunCorrectnessTests()
        {
            Console.WriteLine("Correctness verification:");
            
            // Test basic cases
            var empty1 = Value.FromString("");
            var empty2 = Value.FromString("");
            var hello = Value.FromString("hello");
            var world = Value.FromString("world");
            
            Console.WriteLine($"\"\" -> \"\": {EditDistance(empty1, empty2).AsInt()} (expected: 0)");
            Console.WriteLine($"\"hello\" -> \"\": {EditDistance(hello, empty1).AsInt()} (expected: 5)");
            Console.WriteLine($"\"\" -> \"world\": {EditDistance(empty1, world).AsInt()} (expected: 5)");
            Console.WriteLine($"\"hello\" -> \"hello\": {EditDistance(hello, hello).AsInt()} (expected: 0)");
            
            // Known test case
            var kitten = Value.FromString("kitten");
            var sitting = Value.FromString("sitting");
            int result = EditDistance(kitten, sitting).AsInt();
            Console.WriteLine($"\"kitten\" -> \"sitting\": {result} (expected: 3)");
            
            if (result != 3)
            {
                Console.WriteLine("ERROR: Basic test case failed!");
            }
            else
            {
                Console.WriteLine("✓ Basic correctness verified");
            }
        }
        
        static void Main(string[] args)
        {
            Console.WriteLine("C# NaN Boxing Levenshtein Benchmark (Improved)");
            Console.WriteLine("===============================================");
            Console.WriteLine();
            
            Console.WriteLine($"Before tests: {HandlePool.GetCount()} objects in HandlePool");
            
            RunCorrectnessTests();
            Console.WriteLine();
            
            Console.WriteLine($"After correctness tests: {HandlePool.GetCount()} objects in HandlePool");
            
            RunBenchmark();
            
            Console.WriteLine($"After benchmark: {HandlePool.GetCount()} objects in HandlePool");
        }
    }
}