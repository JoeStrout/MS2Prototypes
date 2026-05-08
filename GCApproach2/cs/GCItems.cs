namespace MS2.GCPrototype;

// ── GCString ────────────────────────────────────────────────────────────────

public struct GCString : IGCItem {
    public string? Data;

    public void MarkChildren(GCManager gc) { }   // strings contain no child Values

    public void OnSweep() { Data = null; }
}

// ── GCList ──────────────────────────────────────────────────────────────────

public struct GCList : IGCItem {
    internal Value[]? Items;
    public int Count;
    public bool Frozen;

    public void Init(int capacity = 8) {
        Items = new Value[Math.Max(capacity, 4)];
        Count = 0;
        Frozen = false;
    }

    public void Push(Value v) {
        if (Items == null) Init();
        if (Count == Items!.Length) Array.Resize(ref Items, Items.Length * 2);
        Items[Count++] = v;
    }

    public Value Get(int i) => (uint)i < (uint)Count ? Items![i] : Value.Null;
    public void  Set(int i, Value v) { if ((uint)i < (uint)Count) Items![i] = v; }

    public void MarkChildren(GCManager gc) {
        if (Items == null) return;
        for (int i = 0; i < Count; i++) gc.Mark(Items[i]);
    }

    public void OnSweep() { Items = null; Count = 0; Frozen = false; }
}

// ── GCMap ───────────────────────────────────────────────────────────────────
// Open-addressing hash table (linear probing) with Value keys and values.
// Tombstones are represented by a reserved sentinel in the hash array.

public struct GCMap : IGCItem {
    private Value[]? _keys;
    private Value[]? _vals;
    private int[]?   _hashes;   // stored hash, or EMPTY / TOMBSTONE
    private int      _count;
    private int      _cap;
    public  bool     Frozen;

    private const int EMPTY     = 0;
    private const int TOMBSTONE = -1;

    public int Count => _count;

    public void Init(int capacity = 8) {
        _cap    = NextPow2(Math.Max(capacity, 8));
        _keys   = new Value[_cap];
        _vals   = new Value[_cap];
        _hashes = new int[_cap];   // all zero = EMPTY
        _count  = 0;
        Frozen  = false;
    }

    public bool TryGet(Value key, out Value value) {
        if (_hashes == null) { value = Value.Null; return false; }
        int h = Hash(key);
        int idx = h & (_cap - 1);
        for (int probe = 0; probe < _cap; probe++) {
            int stored = _hashes[idx];
            if (stored == EMPTY) { value = Value.Null; return false; }
            if (stored != TOMBSTONE && stored == h && Value.Identical(_keys![idx], key)) {
                value = _vals![idx];
                return true;
            }
            idx = (idx + 1) & (_cap - 1);
        }
        value = Value.Null;
        return false;
    }

    public void Set(Value key, Value value) {
        if (_hashes == null) Init();
        if (_count * 2 >= _cap) Resize(_cap * 2);
        int h = Hash(key);
        int idx = h & (_cap - 1);
        int firstTomb = -1;
        for (int probe = 0; probe < _cap; probe++) {
            int stored = _hashes![idx];
            if (stored == EMPTY) {
                int ins = firstTomb >= 0 ? firstTomb : idx;
                _keys![ins] = key; _vals![ins] = value; _hashes[ins] = h;
                _count++;
                return;
            }
            if (stored == TOMBSTONE) {
                if (firstTomb < 0) firstTomb = idx;
            } else if (stored == h && Value.Identical(_keys![idx], key)) {
                _vals![idx] = value;   // update existing
                return;
            }
            idx = (idx + 1) & (_cap - 1);
        }
        // Should not reach here if load factor is kept below 0.5
        throw new InvalidOperationException("GCMap: table full");
    }

    public bool Remove(Value key) {
        if (_hashes == null) return false;
        int h = Hash(key);
        int idx = h & (_cap - 1);
        for (int probe = 0; probe < _cap; probe++) {
            int stored = _hashes[idx];
            if (stored == EMPTY) return false;
            if (stored != TOMBSTONE && stored == h && Value.Identical(_keys![idx], key)) {
                _hashes[idx] = TOMBSTONE;
                _keys![idx]  = Value.Null;
                _vals![idx]  = Value.Null;
                _count--;
                return true;
            }
            idx = (idx + 1) & (_cap - 1);
        }
        return false;
    }

    // Iterate over live entries.  Use a for loop: for (int i = NextEntry(-1); i >= 0; i = NextEntry(i))
    public int NextEntry(int after) {
        if (_hashes == null) return -1;
        for (int i = after + 1; i < _cap; i++)
            if (_hashes[i] != EMPTY && _hashes[i] != TOMBSTONE) return i;
        return -1;
    }
    public Value KeyAt(int i)   => _keys![i];
    public Value ValueAt(int i) => _vals![i];

    public void MarkChildren(GCManager gc) {
        if (_hashes == null) return;
        for (int i = 0; i < _cap; i++) {
            if (_hashes[i] != EMPTY && _hashes[i] != TOMBSTONE) {
                gc.Mark(_keys![i]);
                gc.Mark(_vals![i]);
            }
        }
    }

    public void OnSweep() { _keys = null; _vals = null; _hashes = null; _count = 0; _cap = 0; Frozen = false; }

    private void Resize(int newCap) {
        var oldKeys   = _keys!;
        var oldVals   = _vals!;
        var oldHashes = _hashes!;
        int oldCap    = _cap;
        _cap    = newCap;
        _keys   = new Value[newCap];
        _vals   = new Value[newCap];
        _hashes = new int[newCap];
        _count  = 0;
        for (int i = 0; i < oldCap; i++) {
            if (oldHashes[i] != EMPTY && oldHashes[i] != TOMBSTONE)
                Set(oldKeys[i], oldVals[i]);
        }
    }

    private static int Hash(Value v) {
        int h = (int)(v.Bits ^ (v.Bits >> 32));
        // Ensure result is never EMPTY (0) or TOMBSTONE (-1) to avoid sentinel collision.
        if (h == EMPTY)     h = 1;
        if (h == TOMBSTONE) h = 2;
        return h;
    }

    private static int NextPow2(int n) {
        int p = 1;
        while (p < n) p <<= 1;
        return p;
    }
}

// ── GCError ─────────────────────────────────────────────────────────────────

public struct GCError : IGCItem {
    public Value Message;
    public Value Inner;
    public Value Stack;
    public Value Isa;

    public void MarkChildren(GCManager gc) {
        gc.Mark(Message);
        gc.Mark(Inner);
        gc.Mark(Stack);
        gc.Mark(Isa);
    }

    public void OnSweep() {
        Message = Value.Null;
        Inner   = Value.Null;
        Stack   = Value.Null;
        Isa     = Value.Null;
    }
}

// ── GCHandle ─────────────────────────────────────────────────────────────────
// Wraps any native object.  DisposeAction is called on sweep if non-null.

public struct GCHandle : IGCItem {
    public object?         Native;
    public Action<object>? DisposeAction;

    public void MarkChildren(GCManager gc) { }   // handles are GC leaves

    public void OnSweep() {
        if (DisposeAction != null && Native != null) DisposeAction(Native);
        Native        = null;
        DisposeAction = null;
    }
}
