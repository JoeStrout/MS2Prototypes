#ifndef MINISCRIPTERRORS_H
#define MINISCRIPTERRORS_H

#include "SimpleString.h"
#include <iostream>

namespace MiniScript {
    // Base exception class
    class MSException {
    public:
        String message;
        MSException(String msg) : message(msg) {}
        void raise() { std::cerr << "Exception: " << message.c_str() << std::endl; }
    };
    
    class RuntimeException : public MSException {
    public:
        RuntimeException(String msg) : MSException(msg) {}
    };
    
    class TypeException : public MSException {
    public:
        TypeException(String msg) : MSException(msg) {}
    };
    
    class IndexException : public MSException {
    public:
        IndexException(String msg) : MSException(msg) {}
    };
    
    class KeyException : public MSException {
    public:
        KeyException(String msg) : MSException(msg) {}
    };
    
    class LimitExceededException : public MSException {
    public:
        LimitExceededException(String msg) : MSException(msg) {}
    };
}

#endif /* MINISCRIPTERRORS_H */