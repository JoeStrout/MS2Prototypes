namespace MS2.GCPrototype;

/// <summary>
/// Implemented by each of the five GC-managed struct types.
/// GCSet&lt;T&gt; calls these during the mark and sweep phases.
/// </summary>
public interface IGCItem {
    /// <summary>Called during the Mark phase; must call gc.Mark() on every child Value.</summary>
    void MarkChildren(GCManager gc);
    /// <summary>Called just before a slot is recycled; release any external resources.</summary>
    void OnSweep();
}
