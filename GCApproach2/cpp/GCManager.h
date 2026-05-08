#pragma once
#include "GCSet.h"
#include "GCItems.h"
#include <vector>
#include <cstdio>

namespace MS2::GCPrototype {

// Central GC coordinator. Owns the five typed GCSets and an explicit root list.
// Mark(Value) dispatches to the right GCSet using the GCSet index baked into
// the Value bits — no switch statement, just array indexing.
// Mirrors the C# GCManager sealed class.
class GCManager {
public:
    // GCSet indices — these constants define the encoding baked into every GC Value.
    static constexpr int STRING_SET = Value::STRING_SET;
    static constexpr int LIST_SET   = Value::LIST_SET;
    static constexpr int MAP_SET    = Value::MAP_SET;
    static constexpr int ERROR_SET  = Value::ERROR_SET;
    static constexpr int HANDLE_SET = Value::HANDLE_SET;

    // Typed accessors for allocation; use these when creating new objects.
    GCSet<GCString> Strings;
    GCSet<GCList>   Lists;
    GCSet<GCMap>    Maps;
    GCSet<GCError>  Errors;
    GCSet<GCHandle> Handles;

    GCManager();

    // Non-copyable, non-movable: _sets holds pointers to our own members.
    GCManager(const GCManager&)            = delete;
    GCManager& operator=(const GCManager&) = delete;
    GCManager(GCManager&&)                 = delete;
    GCManager& operator=(GCManager&&)      = delete;

    // ── Value factories ──────────────────────────────────────────────────────
    Value NewString(const std::string& s);
    Value NewList(int capacity = 8);
    Value NewMap(int capacity = 8);
    Value NewError(Value message, Value inner, Value stack, Value isa);
    Value NewHandle(void* native, std::function<void(void*)> disposeAction = nullptr);

    // ── Retain / Release ─────────────────────────────────────────────────────
    void RetainValue(Value v);
    void ReleaseValue(Value v);

    // ── Root set ──────────────────────────────────────────────────────────────
    void AddRoot(Value v);
    void RemoveRoot(Value v);

    // ── GC cycle ──────────────────────────────────────────────────────────────

    // Mark the given value as live. Called from MarkChildren implementations
    // and from CollectGarbage. Branchless dispatch via _sets array.
    void Mark(Value v) {
        if (!v.IsGCObject()) return;
        _sets[v.GCSetIndex()]->Mark(v.ItemIndex(), *this);
    }

    void CollectGarbage();

    // ── Diagnostics ───────────────────────────────────────────────────────────
    void PrintStats() const;

    // Convenience ref-accessors for reading GC objects from a Value.
    GCString& GetString(Value v) { return Strings.Get(v.ItemIndex()); }
    GCList&   GetList(Value v)   { return Lists.Get(v.ItemIndex()); }
    GCMap&    GetMap(Value v)    { return Maps.Get(v.ItemIndex()); }
    GCError&  GetError(Value v)  { return Errors.Get(v.ItemIndex()); }
    GCHandle& GetHandle(Value v) { return Handles.Get(v.ItemIndex()); }

private:
    IGCSet*            _sets[5];
    std::vector<Value> _roots;
};

} // namespace MS2::GCPrototype
