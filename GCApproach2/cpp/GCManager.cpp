#include "GCManager.h"

namespace MS2::GCPrototype {

GCManager::GCManager() {
    _sets[STRING_SET] = &Strings;
    _sets[LIST_SET]   = &Lists;
    _sets[MAP_SET]    = &Maps;
    _sets[ERROR_SET]  = &Errors;
    _sets[HANDLE_SET] = &Handles;
}

// ── Value factories ──────────────────────────────────────────────────────────

Value GCManager::NewString(const std::string& s) {
    int idx = Strings.New();
    Strings.Get(idx).Data = s;
    return Value::MakeGC(STRING_SET, idx);
}

Value GCManager::NewList(int capacity) {
    int idx = Lists.New();
    Lists.Get(idx).Init(capacity);
    return Value::MakeGC(LIST_SET, idx);
}

Value GCManager::NewMap(int capacity) {
    int idx = Maps.New();
    Maps.Get(idx).Init(capacity);
    return Value::MakeGC(MAP_SET, idx);
}

Value GCManager::NewError(Value message, Value inner, Value stack, Value isa) {
    int idx = Errors.New();
    GCError& e = Errors.Get(idx);
    e.Message = message;
    e.Inner   = inner;
    e.Stack   = stack;
    e.Isa     = isa;
    return Value::MakeGC(ERROR_SET, idx);
}

Value GCManager::NewHandle(void* native, std::function<void(void*)> disposeAction) {
    int idx = Handles.New();
    GCHandle& h = Handles.Get(idx);
    h.Native        = native;
    h.DisposeAction = disposeAction;
    return Value::MakeGC(HANDLE_SET, idx);
}

// ── Retain / Release ─────────────────────────────────────────────────────────

void GCManager::RetainValue(Value v) {
    if (!v.IsGCObject()) return;
    switch (v.GCSetIndex()) {
        case STRING_SET: Strings.Retain(v.ItemIndex()); break;
        case LIST_SET:   Lists.Retain(v.ItemIndex());   break;
        case MAP_SET:    Maps.Retain(v.ItemIndex());    break;
        case ERROR_SET:  Errors.Retain(v.ItemIndex());  break;
        case HANDLE_SET: Handles.Retain(v.ItemIndex()); break;
    }
}

void GCManager::ReleaseValue(Value v) {
    if (!v.IsGCObject()) return;
    switch (v.GCSetIndex()) {
        case STRING_SET: Strings.Release(v.ItemIndex()); break;
        case LIST_SET:   Lists.Release(v.ItemIndex());   break;
        case MAP_SET:    Maps.Release(v.ItemIndex());    break;
        case ERROR_SET:  Errors.Release(v.ItemIndex());  break;
        case HANDLE_SET: Handles.Release(v.ItemIndex()); break;
    }
}

// ── Root set ──────────────────────────────────────────────────────────────────

void GCManager::AddRoot(Value v) { _roots.push_back(v); }

void GCManager::RemoveRoot(Value v) {
    auto it = std::find(_roots.begin(), _roots.end(), v);
    if (it != _roots.end()) _roots.erase(it);
}

// ── GC cycle ──────────────────────────────────────────────────────────────────

void GCManager::CollectGarbage() {
    // 1. Clear all mark bits.
    for (IGCSet* s : _sets) s->PrepareForGC();

    // 2. Mark from explicit roots.
    for (Value root : _roots) Mark(root);

    // 3. Mark retained items (and their children).
    for (IGCSet* s : _sets) s->MarkRetained(*this);

    // 4. Sweep: free everything still unmarked.
    for (IGCSet* s : _sets) s->Sweep();
}

// ── Diagnostics ───────────────────────────────────────────────────────────────

void GCManager::PrintStats() const {
    printf("  Strings: %d live\n", Strings.LiveCount());
    printf("  Lists:   %d live\n", Lists.LiveCount());
    printf("  Maps:    %d live\n", Maps.LiveCount());
    printf("  Errors:  %d live\n", Errors.LiveCount());
    printf("  Handles: %d live\n", Handles.LiveCount());
}

} // namespace MS2::GCPrototype
