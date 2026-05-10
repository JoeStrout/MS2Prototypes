namespace MS2.GCPrototype;

/// <summary>
/// Non-generic interface over GCSet&lt;T&gt;, enabling the GCManager to hold
/// an IGCSet[5] array and dispatch Mark/Sweep without a switch statement.
/// </summary>
public interface IGCSet {
    void PrepareForGC();
    void Mark(int idx, GCManager gc);
    void MarkRetained(GCManager gc);
    void Sweep();
    int LiveCount { get; }
}
