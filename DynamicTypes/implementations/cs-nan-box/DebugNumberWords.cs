using System;
using System.Diagnostics;

namespace ScriptingVM.Benchmarks
{
    class DebugNumberWords
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Debug NumberWords");
            Console.WriteLine("================");
            
            // Test string split behavior
            var space = Value.FromString(" ");
            var singlesStr = Value.FromString(" one two three four five six seven eight nine ");
            var singles = StringOperations.StringSplit(singlesStr, space);
            
            Console.WriteLine($"Singles list count: {ListOperations.ListCount(singles)}");
            for (int i = 0; i < ListOperations.ListCount(singles); i++)
            {
                var item = ListOperations.ListGet(singles, i);
                Console.WriteLine($"singles[{i}] = '{item}' (IsString: {item.IsString}, IsNull: {item.IsNull})");
            }
            
            Console.WriteLine("\nTesting number 9:");
            // Should get index 9 from singles list
            var nineItem = ListOperations.ListGet(singles, 9);
            Console.WriteLine($"singles[9] = '{nineItem}' (IsString: {nineItem.IsString})");
            
            Console.WriteLine("\nTesting simple NumberToText for 9:");
            var result = SimpleNumberToText(9, singles);
            Console.WriteLine($"NumberToText(9) = '{result}'");
        }
        
        static Value SimpleNumberToText(long n, Value singles)
        {
            if (n == 0)
                return Value.FromString("zero");
            
            long a = Math.Abs(n);
            var r = Value.FromString("");
            
            long h = a % 100;
            if (h > 0 && h < 10)
            {
                var singlesItem = ListOperations.ListGet(singles, (int)h);
                Console.WriteLine($"Getting singles[{h}] = '{singlesItem}'");
                var spaceVal = Value.FromString(" ");
                var temp = StringOperations.StringConcat(singlesItem, spaceVal);
                r = StringOperations.StringConcat(temp, r);
            }
            
            string rStr = r.ToString().Trim();
            return Value.FromString(rStr);
        }
    }
}