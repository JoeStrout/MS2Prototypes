//*** BEGIN CS_ONLY ***
// (This entire file is only for C#; the C++ code uses value.h/.c instead.)

using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace MiniScript {
    // NOTE: Align the TAG MASK constants below with your C value.h.
    // The layout mirrors a NaN-box: 64-bit payload that is either:
    // - a real double (no matching tag), OR
    // - an encoded immediate (int, tiny string), OR
    // - a tagged 32-bit handle to heap-managed objects (string/list/map).
    //
    // Keep Value at 8 bytes, blittable, and aggressively inlined.

    [StructLayout(LayoutKind.Explicit, Size = 8)]
    public readonly struct Value {
        // Overlaid views enable single-move bit casts on CoreCLR.
        [FieldOffset(0)] private readonly ulong _u;
        [FieldOffset(0)] private readonly double _d;

        private Value(ulong u) { _d = 0; _u = u; }

        // ==== TAGS & MASKS (EDIT TO MATCH YOUR C EXACTLY) =====================
        // High 16 bits used to tag NaN-ish payloads.
        private const ulong NANISH_MASK  = 0xFFFF_0000_0000_0000UL; // choose to match C
        private const ulong INTEGER_MASK = 0x7FFC_0000_0000_0000UL; // C int tag
        private const ulong STRING_MASK  = 0xFFFE_0000_0000_0000UL; // heap string tag
        private const ulong LIST_MASK    = 0xFFFD_0000_0000_0000UL; // list tag
        private const ulong MAP_MASK     = 0xFFFB_0000_0000_0000UL; // map tag
        private const ulong TINY_MASK    = 0xFFFF_0000_0000_0000UL; // tiny string tag (shared with NANish)
        private const ulong NULL_VALUE   = 0x7FFE_0000_0000_0000UL; // null singleton

        // ==== CONSTRUCTORS ====================================================
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value Null() => new(NULL_VALUE);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromInt(int i) => new(INTEGER_MASK | (uint)i);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromDouble(double d) {
            // Unity/Mono/IL2CPP-safe: use BitConverter, no Unsafe dependency.
            ulong bits = (ulong)BitConverter.DoubleToInt64Bits(d);
            return new(bits);
        }

        // Tiny ASCII string: stores length (low 8 bits) + up to 5 bytes data in bits 8..48.
        // High bits carry TINY_MASK so the tag check is a single AND/compare.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromTinyAscii(ReadOnlySpan<byte> s) {
            int len = s.Length;
            if ((uint)len > 5u) throw new ArgumentOutOfRangeException(nameof(s), "Tiny string max 5 bytes");
            ulong u = TINY_MASK | (ulong)((uint)len & 0xFFU);
            for (int i = 0; i < len; i++)
                u |= (ulong)((byte)s[i]) << (8 * (i + 1));
            return new(u);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromString(string s) {
            if (s.Length <= 5 && IsAllAscii(s)) {
                Span<byte> tmp = stackalloc byte[5];
                for (int i = 0; i < s.Length; i++) tmp[i] = (byte)s[i];
                return FromTinyAscii(tmp[..s.Length]);
            }
            int h = HandlePool.Add(s);
            return FromHandle(STRING_MASK, h);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromList(object list) { // typically List<Value> or a custom IList
            int h = HandlePool.Add(list);
            return FromHandle(LIST_MASK, h);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromMap(object map) {// typically Dictionary<Value,Value> or custom
            int h = HandlePool.Add(map);
            return FromHandle(MAP_MASK, h);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value FromHandle(ulong tagMask, int handle)
            => new(tagMask | (uint)handle);

        // ==== TYPE PREDICATES =================================================
        public bool IsNull   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => _u == NULL_VALUE; }
        public bool IsInt    { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == INTEGER_MASK; }
        public bool IsTiny   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & TINY_MASK) == TINY_MASK && (_u & NANISH_MASK) != STRING_MASK && (_u & NANISH_MASK) != LIST_MASK && (_u & NANISH_MASK) != MAP_MASK && (_u & NANISH_MASK) != INTEGER_MASK && _u != NULL_VALUE; }
        public bool IsHeapString { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == STRING_MASK && !IsTiny; }
        public bool IsString { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => IsTiny || IsHeapString; }
        public bool IsList   { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == LIST_MASK; }
        public bool IsMap    { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => (_u & NANISH_MASK) == MAP_MASK; }
        public bool IsDouble { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => !IsNull && !IsInt && !IsString && !IsList && !IsMap; }

        // ==== ACCESSORS =======================================================
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int AsInt() => (int)_u;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public double AsDouble() {
            long bits = (long)_u;
            return BitConverter.Int64BitsToDouble(bits);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int Handle() => (int)_u;

        // Tiny decode helpers
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int TinyLen() => (int)(_u & 0xFF);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void TinyCopyTo(Span<byte> dst) {
            int len = TinyLen();
            for (int i = 0; i < len; i++) dst[i] = (byte)((_u >> (8 * (i + 1))) & 0xFF);
        }

        public override string ToString()  {
            if (IsNull) return "null";
            if (IsInt) return AsInt().ToString();
            if (IsDouble) return AsDouble().ToString();
            if (IsString) {
                if (IsTiny) {
                    Span<byte> b = stackalloc byte[5];
                    TinyCopyTo(b);
                    return System.Text.Encoding.ASCII.GetString(b[..TinyLen()]);
                }
                return HandlePool.Get(Handle()) as string ?? "<str?>";
            }
            if (IsList) return "<list>";
            if (IsMap) return "<map>";
            return "<value>";
        }

        public ulong Bits => _u; // sometimes useful for hashing/equality

        // ==== ARITHMETIC & COMPARISON (subset; extend as needed) ==============
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value Add(Value a, Value b) {
            if (a.IsInt && b.IsInt) {
                long r = (long)a.AsInt() + b.AsInt();
                if ((uint)r == r) return FromInt((int)r);
                return FromDouble((double)r);
            }
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return FromDouble(da + db);
            }
            // Handle string concatenation
            if (a.IsString && b.IsString) {
                return StringOperations.StringConcat(a, b);
            }
            // string concat, list append, etc. can be added here.
            return Null();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value Multiply(Value a, Value b) {
            if (a.IsInt && b.IsInt) {
                long r = (long)a.AsInt() * b.AsInt();
                if ((uint)r == r) return FromInt((int)r);
                return FromDouble((double)r);
            }
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return FromDouble(da * db);
            }
            // TODO: String support not added yet!
            // string concat, list append, etc. can be added here.
            return Null();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value Divide(Value a, Value b) {
            if (a.IsInt && b.IsInt) {
                long r = (long)a.AsInt() / b.AsInt();
                if ((uint)r == r) return FromInt((int)r);
                return FromDouble((double)r);
            }
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return FromDouble(da / db);
            }
            // TODO: String support not added yet!
            // string concat, list append, etc. can be added here.
            return Null();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value Sub(Value a, Value b) {
            if (a.IsInt && b.IsInt) {
                long r = (long)a.AsInt() - b.AsInt();
                if (r >= int.MinValue && r <= int.MaxValue) return FromInt((int)r);
                return FromDouble((double)r);
            }
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return FromDouble(da - db);
            }
            return Null();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool LessThan(Value a, Value b) {
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return da < db;
            }
            // Handle string comparison
            if (a.IsString && b.IsString) {
                return StringOperations.StringCompare(a, b) < 0;
            }
            return false;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool Equal(Value a, Value b)  {
            // Fast path: identical bits
            if (a._u == b._u) return true;

            // If both numeric, compare numerically (handles int/double mix)
            if ((a.IsInt || a.IsDouble) && (b.IsInt || b.IsDouble)) {
                double da = a.IsInt ? a.AsInt() : a.AsDouble();
                double db = b.IsInt ? b.AsInt() : b.AsDouble();
                return da == db; // Note: NaN == NaN is false, matching IEEE
            }

            // Both tiny strings => byte compare via bits when lengths equal
            if (a.IsTiny && b.IsTiny) return a._u == b._u;

            // Heap strings via handle indirection
            if (a.IsString && b.IsString) {
                string sa = a.IsTiny ? a.ToString() : HandlePool.Get(a.Handle()) as string;
                string sb = b.IsTiny ? b.ToString() : HandlePool.Get(b.Handle()) as string;
                return string.Equals(sa, sb, StringComparison.Ordinal);
            }

            // Null only equals Null
            if (a.IsNull || b.IsNull) return a.IsNull && b.IsNull;

            return false;
        }

        // ==== HELPERS =========================================================
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static bool IsAllAscii(string s) {
            foreach (char c in s) if (c > 0x7F) return false;
            return true;
        }
    }

    // Global helper functions to match C++ value.h interface
    public static class ValueHelpers {
        // Core value creation functions (matching value.h)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value make_null() => Value.Null();
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value make_int(int i) => Value.FromInt(i);
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value make_double(double d) => Value.FromDouble(d);
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value make_string(string str) => Value.FromString(str);
        
        // Core value extraction functions (matching value.h)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int as_int(Value v) => v.AsInt();
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static double as_double(Value v) => v.AsDouble();
        
        // Core type checking functions (matching value.h)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool is_null(Value v) => v.IsNull;
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool is_int(Value v) => v.IsInt;
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool is_double(Value v) => v.IsDouble;
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool is_string(Value v) => v.IsString;
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool is_number(Value v) => v.IsInt || v.IsDouble;
        
        // Arithmetic operations (matching value.h)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value value_add(Value a, Value b) => Value.Add(a, b);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value value_mult(Value a, Value b) => Value.Multiply(a, b);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value value_div(Value a, Value b) => Value.Divide(a, b);
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Value value_sub(Value a, Value b) => Value.Sub(a, b);
        
        // Comparison operations (matching value.h)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool value_lt(Value a, Value b) => Value.LessThan(a, b);
    }

    // A minimal, fast handle table. Stores actual C# objects referenced by Value.
    // All heap-backed Value variants carry a 32-bit index into this pool.
    internal static class HandlePool {
        private static object[] _objs = new object[1024];
        private static int _count;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int Add(object o) {
            int idx = _count;
            if ((uint)idx >= (uint)_objs.Length)
                Array.Resize(ref _objs, _objs.Length << 1);
            _objs[idx] = o;
            _count = idx + 1;
            return idx;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static object Get(int h) => (uint)h < (uint)_count ? _objs[h] : null;
        
        public static int GetCount() => _count;
    }
    
    // List implementation for Value lists
    public class ValueList {
        private List<Value> _items = new List<Value>();
        
        public int Count => _items.Count;
        
        public void Add(Value item) => _items.Add(item);
        
        public Value Get(int index) {
            if (index < 0 || index >= _items.Count) return Value.Null();
            return _items[index];
        }
        
        public void Set(int index, Value value) {
            if (index >= 0 && index < _items.Count)
                _items[index] = value;
        }
        
        public int IndexOf(Value item) {
            for (int i = 0; i < _items.Count; i++) {
                if (Value.Equal(_items[i], item)) return i;
            }
            return -1;
        }
    }
    
    // String operations
    public static class StringOperations {
        public static Value StringSplit(Value str, Value delimiter) {
            if (!str.IsString || !delimiter.IsString) return Value.Null();
            
            string s = GetStringValue(str);
            string delim = GetStringValue(delimiter);
            
            string[] parts;
            if (delim == "") {
                // Split into characters
                parts = new string[s.Length];
                for (int i = 0; i < s.Length; i++)
                    parts[i] = s[i].ToString();
            } else {
                parts = s.Split(new string[] { delim }, StringSplitOptions.None);
            }
            
            var list = new ValueList();
            foreach (string part in parts) {
                list.Add(Value.FromString(part)); // Include all parts, even empty ones
            }
            
            return Value.FromList(list);
        }
        
        public static Value StringReplace(Value str, Value from, Value to) {
            if (!str.IsString || !from.IsString || !to.IsString) return Value.Null();
            
            string s = GetStringValue(str);
            string fromStr = GetStringValue(from);
            string toStr = GetStringValue(to);
            
            if (fromStr == "") {
                return str; // Can't replace empty string
            }
            if (!s.Contains(fromStr)) {
                return str; // Return original if no match
            }
            string result = s.Replace(fromStr, toStr);
            return Value.FromString(result);
        }
        
        public static Value StringIndexOf(Value str, Value needle) {
            if (!str.IsString || !needle.IsString) return Value.FromInt(-1);
            
            string s = GetStringValue(str);
            string needleStr = GetStringValue(needle);
            
            int index = s.IndexOf(needleStr);
            return Value.FromInt(index);
        }
        
        public static Value StringConcat(Value str1, Value str2) {
            if (!str1.IsString || !str2.IsString) return Value.Null();
            
            string s1 = GetStringValue(str1);
            string s2 = GetStringValue(str2);
            
            return Value.FromString(s1 + s2);
        }
        
        public static int StringLength(Value str) {
            if (!str.IsString) return 0;
            
            return GetStringValue(str).Length;
        }
        
        public static bool StringEquals(Value str1, Value str2) {
            return Value.Equal(str1, str2);
        }

        public static int StringCompare(Value str1, Value str2) {
            if (!str1.IsString || !str2.IsString) return 0;
            
            string sa = str1.IsTiny ? str1.ToString() : HandlePool.Get(str1.Handle()) as string;
            string sb = str2.IsTiny ? str2.ToString() : HandlePool.Get(str2.Handle()) as string;
            
            if (sa == null || sb == null) return 0;
            return string.Compare(sa, sb, StringComparison.Ordinal);
        }
        
        private static string GetStringValue(Value val) {
            if (val.IsTiny) return val.ToString();
            if (val.IsHeapString) return HandlePool.Get(val.Handle()) as string ?? "";
            return "";
        }
    }
    
    // List operations
    public static class ListOperations {
        public static Value ListGet(Value list, int index) {
            if (!list.IsList) return Value.Null();
            
            var valueList = HandlePool.Get(list.Handle()) as ValueList;
            return valueList?.Get(index) ?? Value.Null();
        }
        
        public static void ListSet(Value list, int index, Value value) {
            if (!list.IsList) return;
            
            var valueList = HandlePool.Get(list.Handle()) as ValueList;
            valueList?.Set(index, value);
        }
        
        public static void ListAdd(Value list, Value item) {
            if (!list.IsList)  return;
            
            var valueList = HandlePool.Get(list.Handle()) as ValueList;
            valueList?.Add(item);
        }
        
        public static int ListCount(Value list) {
            if (!list.IsList) return 0;
            
            var valueList = HandlePool.Get(list.Handle()) as ValueList;
            return valueList?.Count ?? 0;
        }
        
        public static int ListIndexOf(Value list, Value item) {
            if (!list.IsList) return -1;
            
            var valueList = HandlePool.Get(list.Handle()) as ValueList;
            return valueList?.IndexOf(item) ?? -1;
        }
        
        public static Value MakeList(int initialCapacity = 0) {
            var list = new ValueList();
            return Value.FromList(list);
        }
    }
}
