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
        public static readonly Value True = new Value(ValueType.Number, 1);
        public static readonly Value False = new Value(ValueType.Number, 0);
        public static readonly Value Zero = new Value(ValueType.Number, 0);
        public static readonly Value One = new Value(ValueType.Number, 1);

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
    }
}