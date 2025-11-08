The current prototype has demonstrated that we can transpile C# code (that follows certain [CS_CODING_STANDARDS.md](coding standards)) into C++ code, though not without considerable care and difficulty regarding memory management.

Given the importance of the web as a platform, it's natural to consider whether we should also transpile to JavaScript or TypeScript.  This document digs into that idea.

## Current Transpiler Architecture

  Our transpiler is relatively simple and elegant:
  - String-based pattern matching (not AST-based)
  - Context tracking (START → NAMESPACE → CLASS → METHOD)
  - Line-by-line processing with regex-like patterns
  - Template-based output using pattern matching and .fill()
  - Special directives for platform-specific code (// CPP:, CS_ONLY, etc.)

## Feasibility: JavaScript/TypeScript Transpiler

  Overall difficulty: Moderate - probably easier than C++ in some ways, harder in others.

## What Would Be Easier with JS/TS

  1. Memory management: No need to worry about pointers, references, or manual memory. Everything is garbage collected like C#.
  2. String transformations are simpler:
    - new keyword: Can keep it (no removal needed)
    - null → stays null (not nullptr)
    - Properties vs methods: Less critical since JS doesn't distinguish
  3. No header/implementation split: Just one .js or .ts file per class (simpler output structure)
  4. Type erasure in production: For plain JS, types disappear completely
  5. Similar syntax for common operations:
  // C# → JavaScript is quite natural
  for (int i = 0; i < count; i++) { }  // → for (let i = 0; i < count; i++) { }
  if (condition) { }                    // → same
  Class.StaticMethod()                  // → same (or Class.staticMethod())

## Significant Challenges

  1. Type system mismatch:
  // C#
  Int32 x = 5;          // Explicit type
  Boolean flag = true;

  // JavaScript
  let x = 5;            // No type annotation
  let flag = true;

  // TypeScript
  let x: number = 5;    // Different type syntax
  let flag: boolean = true;  // lowercase type names
  2. No method overloading: JavaScript doesn't support multiple methods with same name but different signatures
  // C# - we might have:
  public CallInfo(Int32 pc, Int32 base, Int32 funcIdx) { }
  public CallInfo(Int32 pc, Int32 base, Int32 funcIdx, Int32 copyToReg) { }

  // JS - would need workarounds:
  constructor(pc, base, funcIdx, copyToReg = -1) { }  // default parameters
  3. Collection types need mapping:
  // C#
  List<Value> stack;        → // JS/TS
  stack.Add(item);          → stack.push(item);
  stack.Count               → stack.length (property, not method!)
  stack[i]                  → stack[i] (same)
  3. Our transpiler would need to map:
    - List<T> → Array (T[] in TypeScript)
    - .Add() → .push()
    - .Count → .length
    - .Clear() → .length = 0 or = []
  4. Enum syntax differences:
  // C#
  public enum Opcode : Byte { NOOP = 0, LOAD = 1 }

  // TypeScript
  enum Opcode { NOOP = 0, LOAD = 1 }  // No base type specification

  // JavaScript (no native enums)
  const Opcode = Object.freeze({ NOOP: 0, LOAD: 1 });
  5. Access modifiers: Our VM.cs:295-298 has:
  private List<Value> stack;
  public Int32 BaseIndex { get; private set; }
    - TypeScript: Has public/private but they're compile-time only
    - JavaScript: Uses #private syntax (different from C#) or convention (_private)
  6. Property vs method call ambiguity (actually worse than C++):
  // In C#, we know from context:
  Int32 count = stack.Count;           // Property
  String name = funcDef.Name;           // Property

  // JavaScript needs to pick one:
  let count = stack.length;             // Property (no parens)
  let name = funcDef.name;              // Property
  // OR
  let count = stack.getCount();         // Method (with parens)
  6. Our transpiler would need heuristics or metadata to decide.
  7. No out parameters:
  // C#: VM.cs:576-577
  if (!map_try_get(container, index, out result)) { }

  // JavaScript - would need to return tuple/object:
  const [success, result] = map_try_get(container, index);
  // or
  const result = map_try_get(container, index);  // returns null on failure
  8. Span and performance features: VM.cs:344
  Span<Value> localStack = CollectionsMarshal.AsSpan(stack).Slice(baseIndex);
  8. No equivalent in JavaScript - would need to simulate with array slicing or indexing
  9. Namespace translation:
  namespace MiniScript { }  →  // Could become:
  export namespace MiniScript { }  // TypeScript
  // or module pattern in plain JS

## Specific Difficult Patterns from Our Code

  Looking at VM.cs, here are tricky parts:

  1. Generic constraints and ref parameters (VM.cs:214):
  private Int32 ProcessArguments(..., ref List<UInt32> code)
  JavaScript doesn't have ref - objects are already passed by reference, but we can't reassign the reference itself.

  2. Static readonly fields (VM.cs:1244-1247):
  private static readonly Value FuncNamePrint = make_string("print");
  Would become:
  static FuncNamePrint = make_string("print");  // TypeScript
  // or
  VM.FuncNamePrint = make_string("print");      // JavaScript

  3. Switch statements on enums: Works similarly in JS/TS, so this is fine.

  4. Special comments like // CPP:: We'd need equivalent like // JS: or // TS:

## Recommended Approach

  1. Target TypeScript - it's closer to C# with its type system
  2. Extend our coding standards to make transpilation easier:
    - Avoid method overloading (use different names instead)
    - Minimize use of out parameters
    - Document which properties are really properties vs computed values
  3. Add type mapping rules:
  Int32 → number
  Boolean → boolean
  String → string
  List<T> → T[]
  4. Handle collection methods with search/replace:
  .Add( → .push(
  .Count → .length
  .Count() → .length
  5. Use our existing pattern-matching approach - it would work fine
  6. Single-file output instead of .h/.cpp split

## Difficulty Comparison

  | Aspect             | C++              | JavaScript     | TypeScript           |
  |--------------------|------------------|----------------|----------------------|
  | Type system        | Similar to C#    | Very different | Moderately different |
  | Memory management  | Hard (manual)    | Easy (GC)      | Easy (GC)            |
  | Header/impl split  | Hard             | N/A            | N/A                  |
  | Collections        | Need custom impl | Native arrays  | Native + types       |
  | Properties         | Tricky           | No distinction | Compile-time types   |
  | Method overloading | Supported        | Not supported  | Not supported        |
  | Overall difficulty | Hard             | Medium         | Medium-Easy          |

## Bottom Line

  Creating a C#-to-TypeScript transpiler using our approach would be feasible and probably easier than C++ in terms of runtime semantics, but we'd face different challenges:

  - C++ challenge: Memory management, pointers, header files
  - JS/TS challenge: Type system differences, no overloading, collection API mapping, out parameters

  The good news: Our string-based pattern-matching approach would work fine. We'd just need different transformation rules and a different set of limitations in our C# coding standards.

## But Is It Worth It?

We could instead simply compile the C/C++ code with Emscripten, and this is likely to be significantly better performance, as seen in similar projects:

  1. Lua (similar VM architecture to yours):
    - Native C: baseline
    - Emscripten Wasm: ~80-90% of native
    - Hand-optimized JS: ~20-40% of native
    - Fengari (Lua in plain JS): ~30% of native
  2. Python (bytecode interpreter):
    - CPython: baseline
    - Pyodide (Emscripten): ~70% of native
    - Brython (transpiled): ~15-30% of native
  3. Ruby (YARV bytecode VM):
    - CRuby: baseline
    - ruby.wasm: ~60-80% of native
    - Opal (transpiled JS): ~10-20% of native

Wasm is consistently 2-5x faster than transpiled JS for VM implementations.  Wasm also makes it considerably easier to do things like link in Raylib.  So... on the whole, a JS/TS implementation is probably not worth it, unless we have some need to tightly integrate with browser APIs.




