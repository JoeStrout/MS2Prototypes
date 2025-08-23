This folder contains prototypes focused on the dynamic type system.

The basic premise here is: our VM's main job is to manipulate values: adding them, splitting them apart, looking them up in lists or maps, etc.  If the values themselves are slow, the best VM in the world can't make them fast.

So here, we are going to focus on the implementation of those values, including the memory manager and garbage collection.  We'll benchmark various approaches by writing code in C, C++, or C# that use those dynamic types directly -- no VM at all.  This will give us an upper bound on how fast a VM based on that type system could be.

The folder structure is as follows:

- benchmarks: reference implementations, in MiniScript
- implementations: subfolder for each language/approach
