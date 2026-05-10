using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace MS2.GCPrototype;

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
//                     bits 47-8  = up to 5 ASCII chars (C0 in bits 15-8, etc.)
//                     bits  7-0  = length (0-5)

[StructLayout(LayoutKind.Explicit, Size = 8)]
public readonly struct Value {
    [FieldOffset(0)] private readonly ulong _u;
    [FieldOffset(0)] private readonly double _d;

    private Value(ulong u) { _d = 0; _u = u; }

    // ── Tags ────────────────────────────────────────────────────────────────
    private const ulong NANISH_MASK = 0xFFFF_0000_0000_0000UL;
    private const ulong NULL_TAG    = 0xFFF1_0000_0000_0000UL;
    public  const ulong INT_TAG     = 0xFFFA_0000_0000_0000UL;
    private const ulong FUNCREF_TAG = 0xFFFB_0000_0000_0000UL;
    public  const ulong GC_TAG      = 0xFFFE_0000_0000_0000UL;
    public  const ulong TINYSTR_TAG = 0xFFFF_0000_0000_0000UL;

    // Mask covering the top 16-bit tag plus the 3-bit GCSet field (bits 34-32).
    // Used for single-comparison GC type checks (e.g. IsList).
    private const ulong GC_TYPE_MASK = NANISH_MASK | 0x0000_0007_0000_0000UL;

    // ── Singletons ──────────────────────────────────────────────────────────
    public static readonly Value Null        = new(NULL_TAG);
    public static readonly Value Zero        = new(INT_TAG);
    public static readonly Value One         = new(INT_TAG | 1u);
    public static readonly Value EmptyString = MakeTinyString("");

    // ── Factories ───────────────────────────────────────────────────────────
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Value MakeInt(int i) => new(INT_TAG | (uint)i);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Value MakeDouble(double d) {
        ulong bits = (ulong)BitConverter.DoubleToInt64Bits(d);
        return new(bits);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Value MakeGC(int gcSet, int itemIdx) =>
        new(GC_TAG | ((ulong)gcSet << 32) | (uint)itemIdx);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Value MakeFuncRef(int handlePoolIdx) =>
        new(FUNCREF_TAG | (uint)handlePoolIdx);

    public static Value MakeTinyString(string s) {
        if (s.Length > 5) throw new ArgumentException("Tiny string max 5 chars");
        ulong u = TINYSTR_TAG | (ulong)(byte)s.Length;
        for (int i = 0; i < s.Length; i++)
            u |= (ulong)(byte)s[i] << (8 * (i + 1));
        return new(u);
    }

    // ── Type predicates ─────────────────────────────────────────────────────
    public bool IsNull     { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => _u == NULL_TAG; }
    public bool IsInt      { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == INT_TAG; }
    public bool IsDouble   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) < NULL_TAG; }
    public bool IsNumber   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => IsInt || IsDouble; }
    public bool IsFuncRef  { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == FUNCREF_TAG; }
    public bool IsTinyStr  { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == TINYSTR_TAG; }
    public bool IsGCObject { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == GC_TAG; }

    // Per-type GC checks: single AND + compare using GC_TYPE_MASK.
    public bool IsString { [MethodImpl(MethodImplOptions.AggressiveInlining)] get =>
        IsTinyStr || (_u & GC_TYPE_MASK) == (GC_TAG | ((ulong)GCManager.STRING_SET << 32)); }
    public bool IsList   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get =>
        (_u & GC_TYPE_MASK) == (GC_TAG | ((ulong)GCManager.LIST_SET << 32)); }
    public bool IsMap    { [MethodImpl(MethodImplOptions.AggressiveInlining)] get =>
        (_u & GC_TYPE_MASK) == (GC_TAG | ((ulong)GCManager.MAP_SET << 32)); }
    public bool IsError  { [MethodImpl(MethodImplOptions.AggressiveInlining)] get =>
        (_u & GC_TYPE_MASK) == (GC_TAG | ((ulong)GCManager.ERROR_SET << 32)); }
    public bool IsHandle { [MethodImpl(MethodImplOptions.AggressiveInlining)] get =>
        (_u & GC_TYPE_MASK) == (GC_TAG | ((ulong)GCManager.HANDLE_SET << 32)); }

    // ── Accessors ───────────────────────────────────────────────────────────
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public int AsInt() => (int)_u;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public double AsDouble() => BitConverter.Int64BitsToDouble((long)_u);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public double AsNumber() => IsInt ? (double)(int)_u : AsDouble();

    /// <summary>GCSet index (0-4); only meaningful when IsGCObject.</summary>
    public int GCSetIndex { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (int)((_u >> 32) & 0x7); }

    /// <summary>Item index within its GCSet; only meaningful when IsGCObject.</summary>
    public int ItemIndex  { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (int)_u; }

    /// <summary>HandlePool index; only meaningful when IsFuncRef.</summary>
    public int HandleIndex { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (int)_u; }

    // Tiny-string helpers
    public int TinyLen() => (int)(_u & 0xFF);
    public string TinyString() {
        int len = TinyLen();
        var chars = new char[len];
        for (int i = 0; i < len; i++) chars[i] = (char)((_u >> (8 * (i + 1))) & 0xFF);
        return new string(chars);
    }

    // ── Equality / hashing ──────────────────────────────────────────────────
    public ulong Bits => _u;

    public static bool Identical(Value a, Value b) => a._u == b._u;

    public override bool Equals(object? obj) => obj is Value v && _u == v._u;
    public override int GetHashCode() => (int)(_u ^ (_u >> 32));

    // ── Display ─────────────────────────────────────────────────────────────
    public override string ToString() {
        if (IsNull)   return "null";
        if (IsInt)    return AsInt().ToString();
        if (IsDouble) return AsDouble().ToString();
        if (IsTinyStr) return TinyString();
        if (IsGCObject) return $"<gc:{GCSetIndex}[{ItemIndex}]>";
        if (IsFuncRef) return $"<funcref:{HandleIndex}>";
        return "<value>";
    }
}
