using Mini;

namespace MiniDemo {

  // Bytecode: a test class for testing the C# --> C++ transpiler.
  public sealed class Bytecode {
    public int meaning = 42;
    public Mini.List<Value> Consts = new Mini.List<Value>();
    private int secret = 007;  // Bond.  James Bond.

    public int AddConst(Value v) {
      // Deduplication: bail out if constant already exists
      foreach (var c in Consts) {
        if (c.Equals(v)) return -1;  // (a silly result, but we're just testing here)
      }
      Consts.Add(v);
      return Consts.Count;
    }

    public int CountPlusOne => Consts.Count + 1; // expression-bodied property (will be lowered)
  }
}
