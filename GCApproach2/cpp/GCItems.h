#pragma once
#include "Value.h"
#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>

namespace MS2::GCPrototype {

class GCManager; // forward declaration; MarkChildren bodies that call gc.Mark() live in GCItems.cpp

// Each struct below is the C++ equivalent of its C# counterpart.
// Required interface (duck-typed via GCSet<T> template):
//   void MarkChildren(GCManager& gc)
//   void OnSweep()

// ── GCString ────────────────────────────────────────────────────────────────

struct GCString {
    std::string Data;

    void MarkChildren(GCManager&) {}      // strings contain no child Values
    void OnSweep() { Data.clear(); }
};

// ── GCList ──────────────────────────────────────────────────────────────────

struct GCList {
    std::vector<Value> Items;
    bool Frozen = false;

    int Count() const { return (int)Items.size(); }

    void Init(int capacity = 8) {
        Items.clear();
        Items.reserve(std::max(capacity, 4));
        Frozen = false;
    }

    void Push(Value v) { Items.push_back(v); }

    Value Get(int i) const {
        if (i < 0) i += (int)Items.size();
        return (i >= 0 && i < (int)Items.size()) ? Items[i] : Value::Null;
    }
    void Set(int i, Value v) {
        if (i >= 0 && i < (int)Items.size()) Items[i] = v;
    }

    void MarkChildren(GCManager& gc); // defined in GCItems.cpp
    void OnSweep() { Items.clear(); Frozen = false; }
};

// ── GCMap ───────────────────────────────────────────────────────────────────
// Open-addressing hash table (linear probing) with Value keys and values.

struct GCMap {
    bool Frozen = false;

    int Count() const { return _count; }

    void Init(int capacity = 8);
    bool TryGet(Value key, Value& value) const;
    void Set(Value key, Value value);
    bool Remove(Value key);

    // Iterate live entries: for (int i = NextEntry(-1); i >= 0; i = NextEntry(i))
    int   NextEntry(int after) const;
    Value KeyAt(int i)   const { return _keys[i]; }
    Value ValueAt(int i) const { return _vals[i]; }

    void MarkChildren(GCManager& gc); // defined in GCItems.cpp
    void OnSweep() {
        _keys.clear(); _vals.clear(); _hashes.clear();
        _count = 0; _cap = 0; Frozen = false;
    }

private:
    std::vector<Value> _keys;
    std::vector<Value> _vals;
    std::vector<int>   _hashes;   // EMPTY=0, TOMBSTONE=-1, else cached hash
    int _count = 0;
    int _cap   = 0;

    static constexpr int EMPTY     = 0;
    static constexpr int TOMBSTONE = -1;

    void Resize(int newCap);

    static int Hash(Value v) {
        uint64_t bits = v.Bits();
        int h = (int)(bits ^ (bits >> 32));
        if (h == EMPTY)     h = 1;
        if (h == TOMBSTONE) h = 2;
        return h;
    }
    static int NextPow2(int n) { int p = 1; while (p < n) p <<= 1; return p; }
};

// ── GCError ─────────────────────────────────────────────────────────────────

struct GCError {
    Value Message;
    Value Inner;
    Value Stack;
    Value Isa;

    void MarkChildren(GCManager& gc); // defined in GCItems.cpp
    void OnSweep() {
        Message = Value::Null; Inner = Value::Null;
        Stack   = Value::Null; Isa   = Value::Null;
    }
};

// ── GCHandle ─────────────────────────────────────────────────────────────────
// Wraps any native object. DisposeAction is called on sweep if set.

struct GCHandle {
    void*                      Native        = nullptr;
    std::function<void(void*)> DisposeAction;

    void MarkChildren(GCManager&) {}     // handles are GC leaves
    void OnSweep() {
        if (DisposeAction && Native) DisposeAction(Native);
        Native = nullptr;
        DisposeAction = nullptr;
    }
};

} // namespace MS2::GCPrototype
