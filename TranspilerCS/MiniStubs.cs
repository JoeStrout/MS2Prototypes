// Stub types for Mini namespace to make compilation work
using System.Collections.Generic;

namespace Mini {
    public class Value {
        public bool Equals(Value other) {
            return false; // stub implementation
        }
    }
    
    public class List<T> {
        private System.Collections.Generic.List<T> _items = new();
        
        public int Count => _items.Count;
        public void Add(T item) => _items.Add(item);
        public T this[int index] => _items[index];
        
        public System.Collections.Generic.IEnumerator<T> GetEnumerator() => _items.GetEnumerator();
    }
}