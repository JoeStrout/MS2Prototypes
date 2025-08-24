#ifndef MINISCRIPTTAC_H
#define MINISCRIPTTAC_H

namespace MiniScript {
    // Forward declarations
    class Value;
    class String;
    enum class LocalOnlyMode : unsigned char;
    
    // Stub classes for Three Address Code (TAC) compilation
    class TACLine {
    public:
        // Stub implementation
    };
    
    class Machine {
    public:
        // Stub implementation for virtual machine
        String FindShortName(Value v) { (void)v; return String("?"); }
        
        // Type instances (stub)
        Value numberType;
        Value stringType;
        Value listType;
        Value mapType;
        Value functionType;
    };

    class Context {
    public:
        // Stub implementation for execution context
        Machine* vm;
        Context() : vm(nullptr) {}
        
        Value GetTemp(int tempNum) { (void)tempNum; return Value(); }
        Value GetVar(String ident, LocalOnlyMode localOnly) { (void)ident; (void)localOnly; return Value(); }
    };
    
    // Stub intrinsics
    namespace Intrinsics {
        Value MapType() { return Value(); }
        Value ListType() { return Value(); }
        Value StringType() { return Value(); }
        Value NumberType() { return Value(); }
        Value FunctionType() { return Value(); }
    }
}

#endif /* MINISCRIPTTAC_H */