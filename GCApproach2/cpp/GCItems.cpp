#include "GCManager.h"   // brings in GCItems.h + full GCManager definition

namespace MS2::GCPrototype {

// ── GCList ──────────────────────────────────────────────────────────────────

void GCList::MarkChildren(GCManager& gc) {
    for (Value v : Items) gc.Mark(v);
}

// ── GCMap ───────────────────────────────────────────────────────────────────

void GCMap::Init(int capacity) {
    _cap = NextPow2(std::max(capacity, 8));
    _keys.assign(_cap, Value::Null);
    _vals.assign(_cap, Value::Null);
    _hashes.assign(_cap, EMPTY);
    _count = 0;
    Frozen = false;
}

bool GCMap::TryGet(Value key, Value& value) const {
    if (_hashes.empty()) { value = Value::Null; return false; }
    int h   = Hash(key);
    int idx = h & (_cap - 1);
    for (int probe = 0; probe < _cap; probe++) {
        int stored = _hashes[idx];
        if (stored == EMPTY) { value = Value::Null; return false; }
        if (stored != TOMBSTONE && stored == h && Value::Identical(_keys[idx], key)) {
            value = _vals[idx];
            return true;
        }
        idx = (idx + 1) & (_cap - 1);
    }
    value = Value::Null;
    return false;
}

void GCMap::Set(Value key, Value value) {
    if (_hashes.empty()) Init();
    if (_count * 2 >= _cap) Resize(_cap * 2);
    int h        = Hash(key);
    int idx      = h & (_cap - 1);
    int firstTomb = -1;
    for (int probe = 0; probe < _cap; probe++) {
        int stored = _hashes[idx];
        if (stored == EMPTY) {
            int ins = firstTomb >= 0 ? firstTomb : idx;
            _keys[ins] = key; _vals[ins] = value; _hashes[ins] = h;
            _count++;
            return;
        }
        if (stored == TOMBSTONE) {
            if (firstTomb < 0) firstTomb = idx;
        } else if (stored == h && Value::Identical(_keys[idx], key)) {
            _vals[idx] = value;   // update existing
            return;
        }
        idx = (idx + 1) & (_cap - 1);
    }
    throw std::runtime_error("GCMap: table full");
}

bool GCMap::Remove(Value key) {
    if (_hashes.empty()) return false;
    int h   = Hash(key);
    int idx = h & (_cap - 1);
    for (int probe = 0; probe < _cap; probe++) {
        int stored = _hashes[idx];
        if (stored == EMPTY) return false;
        if (stored != TOMBSTONE && stored == h && Value::Identical(_keys[idx], key)) {
            _hashes[idx] = TOMBSTONE;
            _keys[idx]   = Value::Null;
            _vals[idx]   = Value::Null;
            _count--;
            return true;
        }
        idx = (idx + 1) & (_cap - 1);
    }
    return false;
}

int GCMap::NextEntry(int after) const {
    for (int i = after + 1; i < _cap; i++)
        if (_hashes[i] != EMPTY && _hashes[i] != TOMBSTONE) return i;
    return -1;
}

void GCMap::Resize(int newCap) {
    std::vector<Value> oldKeys   = std::move(_keys);
    std::vector<Value> oldVals   = std::move(_vals);
    std::vector<int>   oldHashes = std::move(_hashes);
    int oldCap = _cap;
    _cap = newCap;
    _keys.assign(newCap, Value::Null);
    _vals.assign(newCap, Value::Null);
    _hashes.assign(newCap, EMPTY);
    _count = 0;
    for (int i = 0; i < oldCap; i++)
        if (oldHashes[i] != EMPTY && oldHashes[i] != TOMBSTONE)
            Set(oldKeys[i], oldVals[i]);
}

void GCMap::MarkChildren(GCManager& gc) {
    for (int i = NextEntry(-1); i >= 0; i = NextEntry(i)) {
        gc.Mark(_keys[i]);
        gc.Mark(_vals[i]);
    }
}

// ── GCError ─────────────────────────────────────────────────────────────────

void GCError::MarkChildren(GCManager& gc) {
    gc.Mark(Message);
    gc.Mark(Inner);
    gc.Mark(Stack);
    gc.Mark(Isa);
}

} // namespace MS2::GCPrototype
