#pragma once
#include "IGCSet.h"
#include <vector>
#include <algorithm>
#include <cstdint>

namespace MS2::GCPrototype {

// GCSet<T> manages a pool of GC-tracked objects using a struct-of-arrays layout.
// T must provide: void MarkChildren(GCManager&), void OnSweep()
// Mirrors the C# GCSet<T> sealed class.
template<typename T>
class GCSet final : public IGCSet {
private:
    std::vector<T>        _items;
    std::vector<uint8_t>  _inUse;
    std::vector<uint8_t>  _marked;
    std::vector<int>      _retainCounts;
    std::vector<int>      _free;          // stack of recycled indices
    int _hwm = 0;                         // high-water mark

public:
    explicit GCSet(int initialCapacity = 64) {
        _items.resize(initialCapacity);
        _inUse.resize(initialCapacity, 0);
        _marked.resize(initialCapacity, 0);
        _retainCounts.resize(initialCapacity, 0);
        _free.reserve(initialCapacity);
    }

    // ── Allocation ──────────────────────────────────────────────────────────

    // Reserves a slot and returns its index.
    // The caller must initialise the item via Get(idx) before use.
    int New() {
        int idx;
        if (!_free.empty()) {
            idx = _free.back();
            _free.pop_back();
        } else {
            idx = _hwm++;
            if (idx >= (int)_items.size()) Grow();
        }
        _items[idx]        = T{};
        _inUse[idx]        = 1;
        _marked[idx]       = 0;
        _retainCounts[idx] = 0;
        return idx;
    }

    // Returns a reference to the item at idx for in-place initialisation or mutation.
    T& Get(int idx) { return _items[idx]; }

    // ── Retain / Release ────────────────────────────────────────────────────

    void Retain(int idx)  { _retainCounts[idx]++; }
    void Release(int idx) { _retainCounts[idx]--; }

    // ── IGCSet implementation ────────────────────────────────────────────────

    void PrepareForGC() override {
        std::fill(_marked.begin(), _marked.begin() + _hwm, 0);
    }

    void Mark(int idx, GCManager& gc) override {
        if (_marked[idx]) return;
        _marked[idx] = 1;
        _items[idx].MarkChildren(gc);
    }

    void MarkRetained(GCManager& gc) override {
        for (int i = 0; i < _hwm; i++)
            if (_inUse[i] && _retainCounts[i] > 0) Mark(i, gc);
    }

    void Sweep() override {
        for (int i = 0; i < _hwm; i++) {
            if (_inUse[i] && !_marked[i] && _retainCounts[i] <= 0) {
                _items[i].OnSweep();
                _items[i]        = T{};
                _inUse[i]        = 0;
                _retainCounts[i] = 0;
                _free.push_back(i);
            }
        }
    }

    int LiveCount() const override {
        int n = 0;
        for (int i = 0; i < _hwm; i++) if (_inUse[i]) n++;
        return n;
    }

private:
    void Grow() {
        int newLen = (int)_items.size() * 2;
        _items.resize(newLen);
        _inUse.resize(newLen, 0);
        _marked.resize(newLen, 0);
        _retainCounts.resize(newLen, 0);
    }
};

} // namespace MS2::GCPrototype
