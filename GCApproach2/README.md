_08 May 2026_

MiniScript 2 is pretty much fully functional now, but it has a problem: the C# version uses a "HandlePool" approach to big objects (lists/maps) that effectively disables the C# garbage collector.  Those objects are going to leak, at least until the whole HandlePool is disposed of (presumably when the program exits).

That's not acceptable.  But it comes from the unalterable fact that we can't NaN-box a C# reference.  So, as long as our Values are NaN boxes, we don't play well with the C# GC system.

But maybe we can turn this weakness into a strength.  If we take control of our own garbage collection for MiniScript values, then we can provide stronger guarantees than C# does as to when an object will be disposed of.  That could be handy particularly for "handle" types, by which I mean, any native object that might need a Dispose call to free resources when it's no longer needed.

The key observation now is that our GC does not need to handle arbitrary memory allocations.  It only needs to handle five types of well-defined, small structs:

1. Lists
2. Maps
3. Strings
4. Errors
5. Handles (native object wrappers)

...and all of those can allocate/deallocate their *data* using malloc/free (in C) or new/dispose (C++), or C#'s native memory management, or custom functions in the case of handles.

In fact I'm tempted to claim that we will not do our garbage collection at allocation time, but only at well-defined boundary times.  That would eliminate the need to GC_PROTECT every local Value -- only long-lived Values would be of concern.  And for those, I'm tempted to use a Retain/Release system rather than actually adding to the root set.

So, for each of the five types above, I propose we have a "GCSet" container class with the following functionality:

- New() prepares a new object of its type, and returns an index number.
- Get(idx) returns the object with that index.
- Retain(idx) increases the retain count of the object with that index.
- Release(idx) decreases the retain count of the object with that index.  (These are used only for long-lived values not part of the standard root set.)
- PrepareForGC() does whatever internal preparations are needed before marking values as in-use.
- Mark(idx) marks the given object as in use, and if it's a list or map, recurses in and marks all the objects it contains.  (We'll need some additional flag on these to avoid recursing through an object more than once, in case of reference cycles.)
- Sweep() frees any objects not marked at this point, returning them to a free list or recycle pool or whatever, after calling their Dispose method (to free allocated data or do whatever a Handle object wants done at this point).

A central GC manager would know about the four GCSets, and include a Mark(Value) method that calls through to the Mark method on the appropriate GCSet (doing nothing for numbers, tiny strings, null, and funcRefs).  It would also manage the root set, and orchestrate the mark/sweep process, which would be triggered explicitly by the user via some intrinsic, or implicitly in `yield` and `wait` when reasonable criteria are met.

Value would still be a NaN-boxed type, but for one of the five types above, the payload would be just an index into the corresponding GCSet.  Indexes would be guaranteed to fit (unlike pointers, which fit on *most* current systems but not all).
