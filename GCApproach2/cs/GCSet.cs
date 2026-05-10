namespace MS2.GCPrototype;

/// <summary>
/// Manages a pool of GC-tracked objects of type T, using a struct-of-arrays layout
/// so that GC-phase metadata (Marked, InUse, RetainCount) occupies its own tight
/// arrays, separate from the item data.
/// </summary>
public sealed class GCSet<T> : IGCSet where T : struct, IGCItem {

    private T[]    _items;
    private bool[] _inUse;
    private bool[] _marked;
    private int[]  _retainCounts;

    private int    _hwm;        // high-water mark: indices 0.._hwm-1 have been used
    private int[]  _free;       // stack of recycled indices
    private int    _freeTop;

    public GCSet(int initialCapacity = 64) {
        _items        = new T[initialCapacity];
        _inUse        = new bool[initialCapacity];
        _marked       = new bool[initialCapacity];
        _retainCounts = new int[initialCapacity];
        _free         = new int[initialCapacity];
        _freeTop      = 0;
        _hwm          = 0;
    }

    // ── Allocation ──────────────────────────────────────────────────────────

    /// <summary>
    /// Reserves a slot and returns its index.
    /// The caller must initialise the item via Get(idx) before use.
    /// </summary>
    public int New() {
        int idx;
        if (_freeTop > 0) {
            idx = _free[--_freeTop];
        } else {
            idx = _hwm++;
            if (idx >= _items.Length) Grow();
        }
        _items[idx]        = default;
        _inUse[idx]        = true;
        _marked[idx]       = false;
        _retainCounts[idx] = 0;
        return idx;
    }

    /// <summary>Returns a ref to the item at idx for in-place initialisation or mutation.</summary>
    public ref T Get(int idx) => ref _items[idx];

    // ── Retain / Release ────────────────────────────────────────────────────
    // Used for long-lived values not in the standard root set.

    // (ToDo: add bounds checking here, throw an error as needed)
    public void Retain(int idx)  => _retainCounts[idx]++;
    public void Release(int idx) => _retainCounts[idx]--;

    // ── IGCSet implementation ────────────────────────────────────────────────

    /// <summary>Clear all mark bits before the Mark phase begins.</summary>
    public void PrepareForGC() {
        Array.Clear(_marked, 0, _hwm);
    }

    /// <summary>Mark the item at idx (and recurse into its children) if not already marked.</summary>
    public void Mark(int idx, GCManager gc) {
        // InUse check is omitted in the hot path; calling Mark on a free slot
        // indicates a bug and should be caught at a higher level in debug builds.
        if (_marked[idx]) return;
        _marked[idx] = true;
        _items[idx].MarkChildren(gc);
    }

    /// <summary>Mark all items whose retain count is > 0 (and their children).</summary>
    public void MarkRetained(GCManager gc) {
        for (int i = 0; i < _hwm; i++)
            if (_inUse[i] && _retainCounts[i] > 0) Mark(i, gc);
    }

    /// <summary>
    /// Free every live, unmarked, unretained item: call OnSweep, clear the slot,
    /// and push the index onto the free stack.
    /// </summary>
    public void Sweep() {
        for (int i = 0; i < _hwm; i++) {
            if (_inUse[i] && !_marked[i] && _retainCounts[i] <= 0) {
                _items[i].OnSweep();
                _items[i]        = default;
                _inUse[i]        = false;
                _retainCounts[i] = 0;
                if (_freeTop >= _free.Length) Array.Resize(ref _free, _free.Length * 2);
                _free[_freeTop++] = i;
            }
        }
    }

    /// <summary>Count of currently live (InUse) slots; O(n), for diagnostics only.</summary>
    public int LiveCount {
        get {
            int n = 0;
            for (int i = 0; i < _hwm; i++) if (_inUse[i]) n++;
            return n;
        }
    }

    // ── Internal ─────────────────────────────────────────────────────────────

    private void Grow() {
        int newLen = _items.Length * 2;
        Array.Resize(ref _items,        newLen);
        Array.Resize(ref _inUse,        newLen);
        Array.Resize(ref _marked,       newLen);
        Array.Resize(ref _retainCounts, newLen);
        Array.Resize(ref _free,         newLen);
    }
}
