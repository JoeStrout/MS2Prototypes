using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace ScriptingVM.Benchmarks
{
    class NumberWords
    {
        // Global word arrays - equivalent to MiniScript's split arrays
        static string[] singles = { "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
        static string[] teens = { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
        static string[] tys = { "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety" };
        static string[] ions = { "thousand", "million", "billion" };
        
        static Value NumberToText(long n)
        {
            if (n == 0)
                return Value.FromString("zero");
            
            long a = Math.Abs(n);
            StringBuilder r = new StringBuilder();
            
            // Process each scale (units, thousands, millions, billions)
            for (int ionIndex = 0; ionIndex < ions.Length; ionIndex++)
            {
                string u = ions[ionIndex];
                
                long h = a % 100;
                if (h > 0 && h < 10)
                {
                    r.Insert(0, singles[h] + " ");
                }
                if (h > 9 && h < 20)
                {
                    r.Insert(0, teens[h - 10] + " ");
                }
                if (h > 19 && h < 100)
                {
                    string hyphen = (h % 10 > 0) ? "-" : "";
                    r.Insert(0, tys[h / 10] + hyphen + singles[h % 10] + " ");
                }
                
                h = (a % 1000) / 100;
                if (h > 0)
                {
                    r.Insert(0, singles[h] + " hundred ");
                }
                
                a = a / 1000;
                if (a == 0) break;
                if (a % 1000 > 0)
                {
                    r.Insert(0, u + " ");
                }
            }
            
            if (n < 0)
            {
                r.Insert(0, "negative ");
            }
            
            // Trim the result
            string result = r.ToString().Trim();
            return Value.FromString(result);
        }
        
        static long TextToNumber(Value s_val)
        {
            if (!s_val.IsString)
                return 0;
                
            string s = s_val.ToString();
            
            if (s == "zero")
                return 0;
            
            // Replace hyphens with spaces and split
            string cleanS = s.Replace("-", " ");
            string[] words = cleanS.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            
            long result = 0;
            long ionVal = 0;
            bool negative = false;
            
            for (int i = 0; i < words.Length; i++)
            {
                string word = words[i];
                
                if (word == "negative")
                {
                    negative = true;
                    continue;
                }
                
                // Check for scale words
                int idx = Array.IndexOf(ions, word);
                if (idx != -1)
                {
                    long[] multipliers = { 1000, 1000000, 1000000000 };
                    result += ionVal * multipliers[idx];
                    ionVal = 0;
                    continue;
                }
                
                long wordVal = 0;
                
                // Check singles
                idx = Array.IndexOf(singles, word);
                if (idx != -1)
                {
                    wordVal = idx;
                }
                else
                {
                    // Check tys
                    idx = Array.IndexOf(tys, word);
                    if (idx != -1)
                    {
                        wordVal = idx * 10;
                    }
                    else
                    {
                        // Check teens
                        idx = Array.IndexOf(teens, word);
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
                if (i < words.Length - 1 && words[i + 1] == "hundred")
                {
                    wordVal *= 100;
                    i++; // Skip "hundred"
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
            Console.WriteLine("C# NaN Boxing NumberWords Benchmark");
            Console.WriteLine("====================================");
            Console.WriteLine();
            
            Console.WriteLine($"Before tests: {HandlePool.GetCount()} objects in HandlePool");
            
            RunCorrectnessTests();
            Console.WriteLine($"After correctness tests: {HandlePool.GetCount()} objects in HandlePool");
            Console.WriteLine();
            
            RunBenchmark(10000);
            
            Console.WriteLine($"After benchmark: {HandlePool.GetCount()} objects in HandlePool");
        }
    }
}