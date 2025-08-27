// mini_runtime.hpp - Basic stub runtime for Mini C# to C++ transpiler
#pragma once

#include <vector>
#include <string>
#include <cstdint>

// Basic type aliases to avoid STL
using int32_t = int;
using int64_t = long long;
using MiniString = std::string;

namespace Mini {
    // Forward declarations
    class Value;
    
    // Basic Value class stub
    class Value {
    public:
        Value() = default;
        Value(const Value& other) = default;
        Value& operator=(const Value& other) = default;
        
        bool Equals(const Value& other) const {
            // Stub implementation - always false for now
            return false;
        }
    };
    
    // Generic List class that avoids STL in the interface
    template<typename T>
    class List {
    private:
        std::vector<T> _items;
        
    public:
        List() = default;
        
        void Add(const T& item) {
            _items.push_back(item);
        }
        
        int Count() const {
            return static_cast<int>(_items.size());
        }
        
        T& operator[](int index) {
            return _items[index];
        }
        
        const T& operator[](int index) const {
            return _items[index];
        }
        
        // Iterator support for range-based for loops
        typename std::vector<T>::iterator begin() { return _items.begin(); }
        typename std::vector<T>::iterator end() { return _items.end(); }
        typename std::vector<T>::const_iterator begin() const { return _items.begin(); }
        typename std::vector<T>::const_iterator end() const { return _items.end(); }
    };
}