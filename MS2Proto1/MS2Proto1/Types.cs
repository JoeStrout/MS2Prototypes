using System;

namespace Types {

    public enum ValueType {
        Null,
        Number,
        String
    }

    public class Value {
        public ValueType type;
        public object? value;

        public Value(ValueType type, object? value) {
            this.type = type;
            this.value = value;
        }
        public Value(double value) {
            this.type = ValueType.Number;
            this.value = value;
        }
        public Value(string value) {
            this.type = ValueType.String;
            this.value = value;
        }
        public static readonly Value Null = new Value(ValueType.Null, null);
        public static readonly Value True = new Value(1);
        public static readonly Value False = new Value(0);
        public static readonly Value Zero = new Value(0);
        public static readonly Value One = new Value(1);

        public override string ToString() {
            switch (type) {
                case ValueType.Null:
                    return "null";
                case ValueType.Number:
                    return ((double)value).ToString();
                case ValueType.String:
                    return "\"" + ((string)value).Replace("\"", "\\\"") + "\"";
                default:
                    return "unknown value type: " + type;
            }
        }

        public static Value operator +(Value a, Value b) {
            if (a.type == ValueType.Null || b.type == ValueType.Null) return Null;
            if (a.type == ValueType.Number) {
                return new Value((double)a.value + (double)b.value);
            } else if (a.type == ValueType.String) {
                return new Value((string)a.value + (string)b.value);
            }
            throw new Exception("Not implemented");
        }

        public static Value operator -(Value a, Value b) {
            if (a.type == ValueType.Null || b.type == ValueType.Null) return Null;
            if (a.type == ValueType.Number) {
                return new Value((double)a.value - (double)b.value);
            } else if (a.type == ValueType.String) {
                string astr = (string)a.value;
                string bstr = (string)b.value;
                if (astr.EndsWith(bstr)) astr = astr.Substring(0, astr.Length - bstr.Length);
                return new Value(astr);
            }
            throw new Exception("Not implemented");
        }

        public static bool operator ==(Value a, Value b) {
            if (a.type != b.type) return false;
            if (a.type == ValueType.Number) {
                double diff = (double)a.value - (double)b.value;
                return diff < 1E-16;
            } else if (a.type == ValueType.String) {
                return (string)a.value == (string)b.value;
            } else if (a.type == ValueType.Null) {
                return true;    // (since we already know they're the same type)
            }
            return false;
        }

        public static bool operator !=(Value a, Value b) {
            return !(a == b);
        }

        public static bool operator <(Value a, Value b) {
            if (a.type != b.type) return false;
            if (a.type == ValueType.Number) {
                return (double)a.value < (double)b.value;
            } else if (a.type == ValueType.String) {
                return string.Compare((string)a.value, (string)b.value, StringComparison.InvariantCulture) < 0;
            }
            return false;
        }

        public static bool operator >(Value a, Value b) {
            if (a.type != b.type) return false;
            if (a.type == ValueType.Number) {
                return (double)a.value > (double)b.value;
            } else if (a.type == ValueType.String) {
                return string.Compare((string)a.value, (string)b.value, StringComparison.InvariantCulture) > 0;
            }
            return false;
        }


    }
}