# Proposed observable changes in MiniScript 2.0

## Dot syntax on numeric literals

MiniScript 1.0 does not allow things like `4.foo` or `3.14.foo`, even when `foo` is defined as an extension method on the `number` map.  You can call such methods on variables (e.g. `x.foo`), but not on numeric literals.  But there's no strong reason to disallow it, and users are occasionally surprised when it doesn't work.  So, in 2.0, let's allow it.

## Frozen Maps and Lists

MiniScript has two mutable data types: maps and lists.  Their mutability can cause problems (1) when an object is passed around that's intended to be used as a value -- for example, a 2D vector -- but it gets mutated, causing unexpected changes elsewhere that it is used; and (2) when used as a map key; after mutation, the behavior is undefined and the key is likely to no longer work, resulting in an essentially broken map.

Solution:

Maps and lists will have an internal _frozen_ flag.  Some new intrinsics will interact with this flag:

- **freeze(x)** will recursively set the frozen flag on `x` and its contents, then return `x`.
- **frozen(x)** will return true if `x` is frozen, false if not.
- **frozenCopy(x)** will return `x` if `x` is already frozen; otherwise it will return a copy of it with the frozen bit set (and do the same recursively for its contents).

And, **using any list or map as a map key actually uses a frozenCopy**.

Any attempt to mutate a frozen list/map will result in a runtime `Attempt to modify a frozen list` (or `map`) error.

## Function Expressions

`function`...`end function` will comprise an _expression_, not a statement.  This just cleans up various odd corners of the syntax.  The effect of this expression is still to create a funcRef, with code that is compiled (just once) for whatever's between the keywords, and `outer` (if needed) assigned to the locals of the function evaluating this expression.



