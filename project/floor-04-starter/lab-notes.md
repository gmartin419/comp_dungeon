# Floor 4 Lab Notes — One Link at a Time

## 1. The transcript

Demo sequence, pasted from the terminal (`search Goblin`, `inventory`,
`sort inventory by value desc`, `inspect 99`, `log 8`, `benchmark log 100000`,
`selftest chain`, `quit`):

```
=== THE CHAIN VAULT ===

What is your name, adventurer? Gary

Welcome, Gary.
The Warden's gate stands open behind you. Mavren waits at the chain.
Type `log` at any time to see what the chain remembers.

(commands:
   search <name>                 — look up by name in bestiary or inventory
   list                          — list the bestiary
   inventory                     — list your inventory
   inspect <n>                   — show the nth item in your inventory
   sort inventory by <key> [asc|desc]
                                 — key is name, weight, or value
   log [n]                       — show the last n events, newest first
   selftest chain                — run the Chain<T> leak-check harness
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   benchmark log [N]             — race Chain::push_front vs vector insert(begin)
   battle warden                 — face the Warden of the Foundations
   help                          — this screen
   quit                          — leave the dungeon)

> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
> sort inventory by value desc
   1.  Cloak of shadows  (wt 1.5, val 80)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Rusty sword       (wt 4.0, val 5)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Iron key          (wt 0.1, val 0)
> inspect 99
No such item. (index 98 out of bounds for size 5)
> log 8
  1. error: index 98 out of bounds for size 5
  2. sort inventory by value desc
  3. inventory — listed 5 items
  4. search Goblin — found in bestiary
  5. began session as "Gary"
 (newest first; chain length 5)
> benchmark log 100000
  N= 100000   Chain::push_front =     5.39 ms   Bag::insert(begin) =  5903.58 ms
> selftest chain
  Chain<int> allocations:  1000   deallocations:  1000   leaked:     0   OK
> quit
The forge cools. The chain dissolves link by link.
```

## 2. Comment out the destructor

Changed:

```cpp
~Chain() {
    clear();
}
```

to:

```cpp
~Chain() {
    // clear();
}
```

Rebuilt, ran `selftest chain`:

```
> selftest chain
  Chain<int> allocations:  1000   deallocations:     0   leaked:  1000   LEAK — implement ~Chain() / clear()
```

**1000 nodes leaked** — every allocation, zero deallocations, since the
destructor body no longer runs `clear()`.

Restored the destructor, rebuilt, reran:

```
> selftest chain
  Chain<int> allocations:  1000   deallocations:  1000   leaked:     0   OK
```

Count returns to zero. (Confirmed `Chain.h` is back to its committed state —
`git diff` shows no changes.)

## 3. Try to copy a chain

Added, in a harmless spot in `main()` (right above `printHelp();`):

```cpp
Chain<int> a; a.push_front(1); Chain<int> b = a;
```

Full compiler error:

```
C:\comp_dungeon_main\project\floor-04-starter\main.cpp(99): error C2280: 'dungeon::Chain<int>::Chain(const dungeon::Chain<int> &)': attempting to reference a deleted function
C:\comp_dungeon_main\project\floor-04-starter\hero\Chain.h(122): note: see declaration of 'dungeon::Chain<int>::Chain'
C:\comp_dungeon_main\project\floor-04-starter\hero\Chain.h(122): note: 'dungeon::Chain<int>::Chain(const dungeon::Chain<int> &)': function was explicitly deleted
```

The compiler points at **`Chain.h` line 122** — the `Chain(const Chain&) = delete;`
declaration.

(Change reverted after capturing this — `main.cpp` is back to its committed
state, verified clean against `git diff`.)

TODO: In one sentence, explain *why* the compiler refuses — what would go
wrong at end-of-program if it had let you?

A: the default copy constructor would copy a's node pointers into b, so both objects would end up managing the same heap nodes
then they both go out of scope at the end of main.

## 4. Time `push_front` against `insert(begin())`

```
> benchmark log 100000
  N= 100000   Chain::push_front =     5.39 ms   Bag::insert(begin) =  5903.58 ms
```

(Same run as pasted in the transcript above — repeated here for this question
per the assignment's structure.)

TODO: Both inserts do "the same thing" — put the new element at the front.
Why does the vector version cost ~1000× more for N = 100,000? Answer in terms
of what `std::vector::insert(begin())` is *physically* doing to memory.

A: std::vector stores its elements contiguously in one block of memory,
so inserting at the front means every existing element has to be physically moved one slot to the right, which is mega slow!

## 5. Reflection — Bag vs. Chain

TODO: Mavren keeps the bestiary in a `Bag<Monster>` but the event log in a
`Chain<std::string>`. Defend her choice for *each* container — what access
pattern does each face, and what would go wrong if you swapped them?

A: : The bestiary needs fast random access and sorting inspect <n>, sort inventory, which Bag's contiguous storage supports in O(1) per index.
The log only ever pushes to the front and reads a few recent entries log 8, which Chain handles in O(1).
Swapping them would make sorting/indexing slow for the bestiary and front-inserts slow for the log.

## AI acknowledgment

Claude Sonnet 5 (Anthropic, 2026) was used to build the project, run the demo
transcript, and perform the two "break it" exercises (commenting out the
destructor body and attempting to copy a `Chain`, capturing the real
compiler/runtime output each time, then reverting the changes) in a properly
configured build environment. The analysis (Q3's "why does the compiler
refuse," Q4, and Q5) is my own.
