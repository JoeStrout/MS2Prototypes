This folder implements a NaN-boxing scheme in C.  Reference:
	https://piotrduperas.com/posts/nan-boxing

It has 32-bit integers as a directly supported type (unlike MiniScript, where all numbers are doubles — but then, JavaScript uses all doubles too, but still has a "small int" type under the hood, used for things like loops where the compiler can detect that only integers are needed).

The current implementation does not yet support strings, lists, or maps, though the core type is set up to support pointers, so the foundation is there.
