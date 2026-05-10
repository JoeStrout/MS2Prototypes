#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <stdexcept>

namespace MS2::GCPrototype {

// 64-bit NaN-boxed dynamic value.
//
// Bit layout:
//   valid double  : top 13 bits not all 1s (standard IEEE 754)
//   null          : 0xFFF1_0000_0000_0000
//   int32         : 0xFFFA_0000_IIII_IIII  (lower 32 bits = signed int)
//   funcref       : 0xFFFB_0000_IIII_IIII  (HandlePool index, not GC-managed)
//   GC object     : 0xFFFE_0000_000G_IIII_IIII
//                     bits 34-32 = GCSet index (0-4, see GCManager constants)
//                     bits 31-0  = item index within that GCSet
//                     bits 47-35 = reserved/unused
//   tiny string   : 0xFFFF_C4C3_C2C1_C0LL
//                     bits 47-8  = up to 5 ASCII chars
//                     bits  7-0  = length (0-5)

struct Value {
    // ── Tags ────────────────────────────────────────────────────────────────
    static constexpr uint64_t NANISH_MASK  = 0xFFFF'0000'0000'0000ULL;
    static constexpr uint64_t NULL_TAG     = 0xFFF1'0000'0000'0000ULL;
    static constexpr uint64_t INT_TAG      = 0xFFFA'0000'0000'0000ULL;
    static constexpr uint64_t FUNCREF_TAG  = 0xFFFB'0000'0000'0000ULL;
    static constexpr uint64_t GC_TAG       = 0xFFFE'0000'0000'0000ULL;
    static constexpr uint64_t TINYSTR_TAG  = 0xFFFF'0000'0000'0000ULL;
    // Mask covering top-16 tag bits plus the 3-bit GCSet field (bits 34-32).
    static constexpr uint64_t GC_TYPE_MASK = NANISH_MASK | 0x0000'0007'0000'0000ULL;

    // GCSet indices — must match GCManager's constants
    static constexpr int STRING_SET = 0;
    static constexpr int LIST_SET   = 1;
    static constexpr int MAP_SET    = 2;
    static constexpr int ERROR_SET  = 3;
    static constexpr int HANDLE_SET = 4;

private:
    uint64_t _u;

    constexpr explicit Value(uint64_t u) : _u(u) {}

public:
    constexpr Value() : _u(NULL_TAG) {}

    // ── Singletons (defined after struct closes, where Value is complete) ───
    static const Value Null;
    static const Value Zero;
    static const Value One;

    // ── Factories ───────────────────────────────────────────────────────────
    static Value MakeInt(int32_t i) { return Value(INT_TAG | (uint32_t)i); }

    static Value MakeDouble(double d) {
        uint64_t bits;
        memcpy(&bits, &d, sizeof bits);
        return Value(bits);
    }

    static Value MakeGC(int gcSet, int itemIdx) {
        return Value(GC_TAG | ((uint64_t)gcSet << 32) | (uint32_t)itemIdx);
    }

    static Value MakeFuncRef(int handleIdx) {
        return Value(FUNCREF_TAG | (uint32_t)handleIdx);
    }

    static Value MakeTinyString(const char* s) {
        size_t len = strlen(s);
        if (len > 5) throw std::invalid_argument("Tiny string max 5 chars");
        uint64_t u = TINYSTR_TAG | (uint8_t)len;
        for (size_t i = 0; i < len; i++)
            u |= (uint64_t)(uint8_t)s[i] << (8 * (i + 1));
        return Value(u);
    }

    // ── Type predicates ─────────────────────────────────────────────────────
    bool IsNull()     const { return _u == NULL_TAG; }
    bool IsInt()      const { return (_u & NANISH_MASK) == INT_TAG; }
    bool IsDouble()   const { return (_u & NANISH_MASK) < NULL_TAG; }
    bool IsNumber()   const { return IsInt() || IsDouble(); }
    bool IsFuncRef()  const { return (_u & NANISH_MASK) == FUNCREF_TAG; }
    bool IsTinyStr()  const { return (_u & NANISH_MASK) == TINYSTR_TAG; }
    bool IsGCObject() const { return (_u & NANISH_MASK) == GC_TAG; }

    bool IsString() const {
        return IsTinyStr() || (_u & GC_TYPE_MASK) == (GC_TAG | ((uint64_t)STRING_SET << 32));
    }
    bool IsList()   const { return (_u & GC_TYPE_MASK) == (GC_TAG | ((uint64_t)LIST_SET   << 32)); }
    bool IsMap()    const { return (_u & GC_TYPE_MASK) == (GC_TAG | ((uint64_t)MAP_SET    << 32)); }
    bool IsError()  const { return (_u & GC_TYPE_MASK) == (GC_TAG | ((uint64_t)ERROR_SET  << 32)); }
    bool IsHandle() const { return (_u & GC_TYPE_MASK) == (GC_TAG | ((uint64_t)HANDLE_SET << 32)); }

    // ── Accessors ───────────────────────────────────────────────────────────
    int32_t AsInt()     const { return (int32_t)_u; }
    double  AsDouble()  const { double d; memcpy(&d, &_u, sizeof d); return d; }
    double  AsNumber()  const { return IsInt() ? (double)(int32_t)_u : AsDouble(); }
    int GCSetIndex()    const { return (int)((_u >> 32) & 7); }
    int ItemIndex()     const { return (int)(uint32_t)_u; }
    int HandleIndex()   const { return (int)(uint32_t)_u; }

    // Tiny-string helpers
    int TinyLen() const { return (int)(_u & 0xFF); }
    std::string TinyToString() const {
        int len = TinyLen();
        std::string s(len, '\0');
        for (int i = 0; i < len; i++) s[i] = (char)((_u >> (8*(i+1))) & 0xFF);
        return s;
    }

    // ── Equality / hashing ──────────────────────────────────────────────────
    uint64_t Bits() const { return _u; }

    static bool Identical(Value a, Value b) { return a._u == b._u; }
    bool operator==(const Value& o) const { return _u == o._u; }
    bool operator!=(const Value& o) const { return _u != o._u; }

    // ── Display ─────────────────────────────────────────────────────────────
    std::string ToString() const {
        if (IsNull())    return "null";
        if (IsInt())     return std::to_string(AsInt());
        if (IsDouble())  { char buf[32]; snprintf(buf, sizeof buf, "%g", AsDouble()); return buf; }
        if (IsTinyStr()) return TinyToString();
        if (IsGCObject()) { char buf[32]; snprintf(buf, sizeof buf, "<gc:%d[%d]>", GCSetIndex(), ItemIndex()); return buf; }
        if (IsFuncRef())  { char buf[32]; snprintf(buf, sizeof buf, "<funcref:%d>", HandleIndex()); return buf; }
        return "<value>";
    }
};

// Singletons defined here because Value must be complete for constexpr init.
// The out-of-class definition has access to the private constructor.
inline constexpr Value Value::Null = Value(Value::NULL_TAG);
inline constexpr Value Value::Zero = Value(Value::INT_TAG);
inline constexpr Value Value::One  = Value(Value::INT_TAG | 1u);

} // namespace MS2::GCPrototype
