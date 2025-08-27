using System;


    void Program::Main(string[] args) {
        string text = "The quick brown fox jumps over the lazy dog";
        
        IOHelper.print("Splitting...");
        List<string> words = text.Split(' ').ToList();

        IOHelper.print("Reversing...");
        words.Reverse();
        
        IOHelper.print("Joining...");
        string reversed = string.Join(" ", words);
        
        IOHelper.print(reversed);
        IOHelper.print("All done!");
    }
    

int main() {
	Program::Main();
}
