using System.Runtime.CompilerServices;

namespace MS2.GCPrototype;

/// <summary>
/// Central GC coordinator.  Owns the five typed GCSets and an explicit root list.
/// Mark(Value) dispatches to the right GCSet using the GCSet index baked into the Value bits —
/// no switch statement, just array indexing.
/// </summary>
public sealed class GCManager {

    // GCSet indices — these constants define the encoding baked into every GC Value.
    public const int STRING_SET = 0;
    public const int LIST_SET   = 1;
    public const int MAP_SET    = 2;
    public const int ERROR_SET  = 3;
    public const int HANDLE_SET = 4;

    // Typed accessors for allocation; use these when creating new objects.
    public readonly GCSet<GCString> Strings = new();
    public readonly GCSet<GCList>   Lists   = new();
    public readonly GCSet<GCMap>    Maps    = new();
    public readonly GCSet<GCError>  Errors  = new();
    public readonly GCSet<GCHandle> Handles = new();

    // Unified array for branchless dispatch in Mark().
    private readonly IGCSet[] _sets;

    private readonly List<Value> _roots = new();

    public GCManager() {
        _sets = new IGCSet[] { Strings, Lists, Maps, Errors, Handles };
    }

    // ── Value factories ─────────────────────────────────────────────────────

    public Value NewString(string s) {
        int idx = Strings.New();
        Strings.Get(idx).Data = s;
        return Value.MakeGC(STRING_SET, idx);
    }

    public Value NewList(int capacity = 8) {
        int idx = Lists.New();
        Lists.Get(idx).Init(capacity);
        return Value.MakeGC(LIST_SET, idx);
    }

    public Value NewMap(int capacity = 8) {
        int idx = Maps.New();
        Maps.Get(idx).Init(capacity);
        return Value.MakeGC(MAP_SET, idx);
    }

    public Value NewError(Value message, Value inner, Value stack, Value isa) {
        int idx = Errors.New();
        ref GCError e = ref Errors.Get(idx);
        e.Message = message;
        e.Inner   = inner;
        e.Stack   = stack;
        e.Isa     = isa;
        return Value.MakeGC(ERROR_SET, idx);
    }

    public Value NewHandle(object native, Action<object>? disposeAction = null) {
        int idx = Handles.New();
        ref GCHandle h = ref Handles.Get(idx);
        h.Native        = native;
        h.DisposeAction = disposeAction;
        return Value.MakeGC(HANDLE_SET, idx);
    }

    // ── Retain / Release ────────────────────────────────────────────────────

    public void Retain(Value v) {
        if (v.IsGCObject) _sets[v.GCSetIndex].Mark(v.ItemIndex, this);
    }

    // (Proper Retain/Release just bumps the count; Mark is only called during GC.)
    public void RetainValue(Value v) {
        if (!v.IsGCObject) return;
        switch (v.GCSetIndex) {
            case STRING_SET: Strings.Retain(v.ItemIndex); break;
            case LIST_SET:   Lists.Retain(v.ItemIndex);   break;
            case MAP_SET:    Maps.Retain(v.ItemIndex);    break;
            case ERROR_SET:  Errors.Retain(v.ItemIndex);  break;
            case HANDLE_SET: Handles.Retain(v.ItemIndex); break;
        }
    }

    public void ReleaseValue(Value v) {
        if (!v.IsGCObject) return;
        switch (v.GCSetIndex) {
            case STRING_SET: Strings.Release(v.ItemIndex); break;
            case LIST_SET:   Lists.Release(v.ItemIndex);   break;
            case MAP_SET:    Maps.Release(v.ItemIndex);    break;
            case ERROR_SET:  Errors.Release(v.ItemIndex);  break;
            case HANDLE_SET: Handles.Release(v.ItemIndex); break;
        }
    }

    // ── Root set ────────────────────────────────────────────────────────────

    public void AddRoot(Value v)    => _roots.Add(v);
    public void RemoveRoot(Value v) => _roots.Remove(v);

    // ── GC cycle ────────────────────────────────────────────────────────────

    /// <summary>
    /// Mark the given value as live.  Called from IGCItem.MarkChildren implementations
    /// and from CollectGarbage.  Branchless dispatch via _sets array.
    /// </summary>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void Mark(Value v) {
        if (!v.IsGCObject) return;
        _sets[v.GCSetIndex].Mark(v.ItemIndex, this);
    }

    /// <summary>Run a full mark-sweep cycle.</summary>
    public void CollectGarbage() {
        // 1. Clear all mark bits.
        foreach (var set in _sets) set.PrepareForGC();

        // 2. Mark from explicit roots.
        foreach (var root in _roots) Mark(root);

        // 3. Mark retained items (and their children).
        foreach (var set in _sets) set.MarkRetained(this);

        // 4. Sweep: free everything that is still unmarked.
        foreach (var set in _sets) set.Sweep();
    }

    // ── Diagnostics ─────────────────────────────────────────────────────────

    public void PrintStats() {
        Console.WriteLine($"  Strings: {Strings.LiveCount} live");
        Console.WriteLine($"  Lists:   {Lists.LiveCount} live");
        Console.WriteLine($"  Maps:    {Maps.LiveCount} live");
        Console.WriteLine($"  Errors:  {Errors.LiveCount} live");
        Console.WriteLine($"  Handles: {Handles.LiveCount} live");
    }

    // Convenience accessors for reading GC objects from a Value.
    public ref GCString GetString(Value v) => ref Strings.Get(v.ItemIndex);
    public ref GCList   GetList(Value v)   => ref Lists.Get(v.ItemIndex);
    public ref GCMap    GetMap(Value v)    => ref Maps.Get(v.ItemIndex);
    public ref GCError  GetError(Value v)  => ref Errors.Get(v.ItemIndex);
    public ref GCHandle GetHandle(Value v) => ref Handles.Get(v.ItemIndex);
}
