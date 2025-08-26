using System;
using System.Diagnostics;

namespace ScriptingVM.Benchmarks
{
    class NumberWordsImproved
    {
        // Global word arrays - equivalent to MiniScript's split arrays
        static Value singles;
        static Value teens;
        static Value tys;
        static Value ions;
        
        static void InitializeWordArrays()
        {
            var space = Value.FromString(" ");
            
            // singles = " one two three four five six seven eight nine ".split(" ")
            var singlesStr = Value.FromString(" one two three four five six seven eight nine ");
            singles = StringOperations.StringSplit(singlesStr, space);
            
            // teens = "ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ".split(" ")
            var teensStr = Value.FromString("ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ");
            teens = StringOperations.StringSplit(teensStr, space);
            
            // tys = "  twenty thirty forty fifty sixty seventy eighty ninety".split(" ")
            var tysStr = Value.FromString("  twenty thirty forty fifty sixty seventy eighty ninety");
            tys = StringOperations.StringSplit(tysStr, space);
            
            // ions = "thousand million billion".split(" ")
            var ionsStr = Value.FromString("thousand million billion");
            ions = StringOperations.StringSplit(ionsStr, space);
        }
        
        static Value NumberToText(long n)
        {
            if (n == 0)
                return Value.FromString("zero");
            
            long a = Math.Abs(n);
            var r = Value.FromString("");  // result
            
            // Process each scale (units, thousands, millions, billions)
            for (int ionIndex = 0; ionIndex < ListOperations.ListCount(ions); ionIndex++)
            {
                var u = ListOperations.ListGet(ions, ionIndex);
                
                long h = a % 100;
                if (h > 0 && h < 10)
                {
                    var singlesItem = ListOperations.ListGet(singles, (int)h);
                    var spaceVal = Value.FromString(" ");
                    var temp = StringOperations.StringConcat(singlesItem, spaceVal);
                    r = StringOperations.StringConcat(temp, r);
                }
                if (h > 9 && h < 20)
                {
                    var teensItem = ListOperations.ListGet(teens, (int)(h - 10));
                    var spaceVal = Value.FromString(" ");
                    var temp = StringOperations.StringConcat(teensItem, spaceVal);
                    r = StringOperations.StringConcat(temp, r);
                }
                if (h > 19 && h < 100)
                {
                    var tysItem = ListOperations.ListGet(tys, (int)(h / 10));
                    var hyphen = (h % 10 > 0) ? Value.FromString("-") : Value.FromString("");
                    var singlesItem = ListOperations.ListGet(singles, (int)(h % 10));
                    var spaceVal = Value.FromString(" ");
                    
                    var temp1 = StringOperations.StringConcat(tysItem, hyphen);
                    var temp2 = StringOperations.StringConcat(temp1, singlesItem);
                    var temp3 = StringOperations.StringConcat(temp2, spaceVal);
                    r = StringOperations.StringConcat(temp3, r);
                }
                
                h = (a % 1000) / 100;
                if (h > 0)
                {
                    var singlesItem = ListOperations.ListGet(singles, (int)h);
                    var hundredSpace = Value.FromString(" hundred ");
                    var temp = StringOperations.StringConcat(singlesItem, hundredSpace);
                    r = StringOperations.StringConcat(temp, r);
                }
                
                a = a / 1000;
                if (a == 0) break;
                if (a % 1000 > 0)
                {
                    var spaceVal = Value.FromString(" ");
                    var temp = StringOperations.StringConcat(u, spaceVal);
                    r = StringOperations.StringConcat(temp, r);
                }
            }
            
            if (n < 0)
            {
                var negative = Value.FromString("negative ");
                r = StringOperations.StringConcat(negative, r);
            }
            
            // Trim the result (simple version - remove leading/trailing spaces)
            string rStr = r.ToString().Trim();
            return Value.FromString(rStr);
        }
        
        static long TextToNumber(Value s_val)
        {
            if (!s_val.IsString)
                return 0;
            
            var zeroStr = Value.FromString("zero");
            if (StringOperations.StringEquals(s_val, zeroStr))
                return 0;
            
            // Replace hyphens with spaces and split
            var hyphen = Value.FromString("-");
            var spaceVal = Value.FromString(" ");
            var cleanS = StringOperations.StringReplace(s_val, hyphen, spaceVal);
            var words = StringOperations.StringSplit(cleanS, spaceVal);
            
            long result = 0;
            long ionVal = 0;
            bool negative = false;
            int wordCount = ListOperations.ListCount(words);
            
            for (int i = 0; i < wordCount; i++)
            {
                var word = ListOperations.ListGet(words, i);
                
                var negativeStr = Value.FromString("negative");
                if (StringOperations.StringEquals(word, negativeStr))
                {
                    negative = true;
                    continue;
                }
                
                // Check for scale words (thousand, million, billion)
                int idx = ListOperations.ListIndexOf(ions, word);
                if (idx != -1)
                {
                    long[] multipliers = { 1000, 1000000, 1000000000 };
                    result += ionVal * multipliers[idx];
                    ionVal = 0;
                    continue;
                }
                
                long wordVal = 0;
                
                // Check singles
                idx = ListOperations.ListIndexOf(singles, word);
                if (idx != -1)
                {
                    wordVal = idx;
                }
                else
                {
                    // Check tys (twenties, thirties, etc.)
                    idx = ListOperations.ListIndexOf(tys, word);
                    if (idx != -1)
                    {
                        wordVal = idx * 10;
                    }
                    else
                    {
                        // Check teens
                        idx = ListOperations.ListIndexOf(teens, word);
                        if (idx != -1)
                        {
                            wordVal = idx + 10;
                        }
                        else
                        {
                            Console.WriteLine($"Unexpected word: {word}");
                            return 0;
                        }
                    }
                }
                
                // Handle "hundred"
                if (i < wordCount - 1)
                {
                    var nextWord = ListOperations.ListGet(words, i + 1);
                    var hundredStr = Value.FromString("hundred");
                    if (StringOperations.StringEquals(nextWord, hundredStr))
                    {
                        wordVal *= 100;
                        i++; // Skip "hundred"
                    }
                }
                
                ionVal += wordVal;
            }
            
            result += ionVal;
            if (negative) result = -result;
            
            return result;
        }
        
        static void RunBenchmark(long n)
        {
            var stopwatch = Stopwatch.StartNew();
            
            for (long i = 0; i < n; i++)
            {
                var s = NumberToText(i);
                long i2 = TextToNumber(s);
                if (i2 != i)
                {
                    Console.WriteLine($"Oops! Failed on {i}:");
                    Console.WriteLine($"'{s}' --> {i2}");
                    break;
                }
            }
            
            stopwatch.Stop();
            Console.WriteLine($"numberWords({n}) time: {stopwatch.ElapsedMilliseconds / 1000.0:F3} seconds");
        }
        
        static void RunCorrectnessTests()
        {
            Console.WriteLine("Correctness checks:");
            
            long[] testNumbers = { -1234, 0, 7, 42, 4325, 1000004, 214837564 };
            
            foreach (long n in testNumbers)
            {
                var words = NumberToText(n);
                long backToNum = TextToNumber(words);
                
                Console.Write($"{n}: {words} -> {backToNum}");
                if (backToNum != n)
                {
                    Console.WriteLine(" ERROR --^");
                    return;
                }
                Console.WriteLine();
            }
        }
        
        static void Main(string[] args)
        {
            Console.WriteLine("C# NaN Boxing NumberWords Benchmark (Improved)");
            Console.WriteLine("===============================================");
            Console.WriteLine();
            
            Console.WriteLine($"Before initialization: {HandlePool.GetCount()} objects in HandlePool");
            InitializeWordArrays();
            Console.WriteLine($"After initialization: {HandlePool.GetCount()} objects in HandlePool");
            
            RunCorrectnessTests();
            Console.WriteLine($"After correctness tests: {HandlePool.GetCount()} objects in HandlePool");
            Console.WriteLine();
            
            RunBenchmark(10000);
            
            Console.WriteLine($"After benchmark: {HandlePool.GetCount()} objects in HandlePool");
        }
    }
}