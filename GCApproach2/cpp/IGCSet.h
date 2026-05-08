#pragma once

namespace MS2::GCPrototype {

class GCManager; // forward declaration

// Non-generic abstract base enabling GCManager to hold an IGCSet*[5] array
// and dispatch Mark/Sweep without a switch statement.
// Mirrors the C# IGCSet interface.
class IGCSet {
public:
    virtual void PrepareForGC()             = 0;
    virtual void Mark(int idx, GCManager&)  = 0;
    virtual void MarkRetained(GCManager&)   = 0;
    virtual void Sweep()                    = 0;
    virtual int  LiveCount() const          = 0;
    virtual ~IGCSet() = default;
};

} // namespace MS2::GCPrototype
