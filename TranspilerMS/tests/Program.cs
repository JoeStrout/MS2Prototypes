using System;

class Program {

    public static void Main(string[] args) {
        string text = "The quick brown fox jumps over the lazy dog";
        
        IOHelper.Print("Splitting...");
        List<string> words = text.Split(' ').ToList();

        IOHelper.Print("Reversing...");
        words.Reverse();
        
        IOHelper.Print("Joining...");
        string reversed = string.Join(" ", words);
        
        IOHelper.Print(reversed);
        IOHelper.Print("All done!");
    }
    
}
