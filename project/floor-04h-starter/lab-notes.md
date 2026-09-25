# Floor 4½ Lab Notes — Both Directions, Twice the Iron

## 1. The transcript

Demo sequence, pasted from the terminal (`search Goblin`, `inventory`,
`inspect 99`, `log --oldest 5`, `clone hero`, `log 3`, `selftest chain`,
`quit`):

```
What is your name, adventurer? Aric

Welcome back, Aric.
Mavren has cuffed a second ring onto every link of your chain.
You can walk it both ways now — and copy it without it exploding.
Try `log --oldest` once you have prev wired, and `clone hero` once you have deep copy.

> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
> inspect 99
No such item. (index 98 out of bounds for size 5)
> log --oldest 5
   1.  began session as "Aric"
   2.  search Goblin — found in bestiary
   3.  inventory — listed 5 items
   4.  error: index 98 out of bounds for size 5
  (oldest first; chain length 4)
> clone hero
  -- original log (newest first) --
   1.  error: index 98 out of bounds for size 5
   2.  inventory — listed 5 items
   3.  search Goblin — found in bestiary
   4.  began session as "Aric"
  (newest first; chain length 4)
  -- cloned log (newest first) --
   1.  error: index 98 out of bounds for size 5
   2.  inventory — listed 5 items
   3.  search Goblin — found in bestiary
   4.  began session as "Aric"
  (newest first; chain length 4)
  (clone is being destroyed now)
  (clone destroyed; original event log still has 4 entries — try `log 3`)
> log 3
   1.  clone hero — copy lived and died
   2.  error: index 98 out of bounds for size 5
   3.  inventory — listed 5 items
  (newest first; chain length 5)
> selftest chain
  Phase 1 (single chain)
    allocations:  1000   deallocations:  1000   leaked:     0   OK
  Phase 2 (deep copy)
    original after copy died — forward walk:  1000   backward walk:  1000
    copy before death        — forward walk:  1000   backward walk:  1000
    allocations:  2000   deallocations:  2000   leaked:     0   OK
> quit
The forge cools. Two chains dissolve, each by its own hand.
```

## 2. Break a `prev` pointer on purpose

In `push_back`, my node is built with its `prev` passed straight into the
constructor, so "omitting `new_node->prev = tail_;`" means changing:

```cpp
Node* n = new Node(value, tail_, nullptr);
```

to:

```cpp
Node* n = new Node(value, nullptr, nullptr);   // prev never set
```

Rebuilt, reran the demo. **`log --oldest 5` printed exactly the same thing as
the clean run:**

```
> log --oldest 5
   1.  began session as "Aric"
   2.  search Goblin — found in bestiary
   3.  inventory — listed 5 items
   4.  error: index 98 out of bounds for size 5
  (oldest first; chain length 4)
```

That's because the hero's event log is only ever built with `push_front`,
which still wires `prev` correctly. The broken `push_back` never touches it.
The damage shows up anywhere a chain is built with `push_back`, which includes
every deep copy (the copy ctor calls `push_back`) and `selftest chain`:

```
> selftest chain
  Phase 1 (single chain)
    allocations:  1000   deallocations:  1000   leaked:     0   OK
  Phase 2 (deep copy)
    original after copy died — forward walk:  1000   backward walk:     1
    copy before death        — forward walk:  1000   backward walk:     1
    allocations:  2000   deallocations:  2000   leaked:     0   FAIL — backward walk wrong (prev / tail_?);
```

Forward walk still finds all 1000 nodes; the backward walk stops after
**1** node, for both the original and the copy.

Restored the line and rebuilt; `selftest chain` is back to `OK` on both phases.

TODO: In one sentence, which step of the backward walk reads the bad pointer?

A:

## 3. Try the shallow copy

Replaced the copy ctor body with:

```cpp
Chain(const Chain& other) {
    head_ = other.head_; tail_ = other.tail_; size_ = other.size_;
}
```

Rebuilt, ran `selftest chain`. The program crashed with no output after the
prompt. Windows exit code **`0xC0000005`** (access violation).

To see exactly where, I rebuilt with MSVC AddressSanitizer
(`/fsanitize=address`) and ran it again:

```
==23136==ERROR: AddressSanitizer: heap-use-after-free on address 0x1231000abfc0
READ of size 8 at 0x1231000abfc0 thread T0
    #0 in dungeon::`anonymous namespace'::walkForward<int>  hero\ChainTests.cpp:44
    #1 in dungeon::runChainSelfTest(void)                    hero\ChainTests.cpp:110
    #2 in main                                               main.cpp:243

0x1231000abfc0 is located 16 bytes inside of 24-byte region
freed by thread T0 here:
    #0 in operator delete(void *, unsigned __int64)
    #1 in dungeon::Chain<int>::Node::`scalar deleting dtor'
    #2 in dungeon::Chain<int>::clear(void)                   hero\Chain.h:293
    #3 in dungeon::Chain<int>::~Chain<int>(void)             hero\Chain.h:93
    #4 in dungeon::runChainSelfTest(void)                    hero\ChainTests.cpp:108
previously allocated by thread T0 here:
    #1 in dungeon::Chain<int>::push_back(int const &)        hero\Chain.h:228
    #2 in dungeon::runChainSelfTest(void)                    hero\ChainTests.cpp:97

SUMMARY: AddressSanitizer: heap-use-after-free hero\ChainTests.cpp:44 in walkForward<int>
```

(Line numbers are for `Chain.h` with the shallow copy in it.)

What happened, step by step:

1. `Chain<int> copy(original);` copies the three fields, so `copy` and
   `original` share the same 1000 nodes.
2. At `ChainTests.cpp:108` (the `}` that ends the copy's scope), **the
   copy's destructor**, `~Chain()` at `Chain.h:93`, calls `clear()`, which
   runs `delete p;` at `Chain.h:293` on every shared node.
3. `original.head_` is now dangling. At `ChainTests.cpp:110` the test walks
   `original`, and that read of freed memory is what actually crashed it.
4. If the walk had survived, **the second `delete` on the same node** would
   come from **the original's destructor**: `~Chain()` (`Chain.h:93`), then
   `clear()`, then `delete p;` (`Chain.h:293`), when `original` leaves scope
   at `ChainTests.cpp:112`. The first node it would delete twice is the old
   head.

Restored the deep copy and rebuilt; `selftest chain` is back to
`2000 / 2000 / 0 OK`.

## 4. Copy-and-swap vs. the explicit form

**Copy-and-swap** (what's in my `Chain.h`):

```cpp
Chain& operator=(const Chain& other) {
    Chain tmp(other); // deep copy other into local tmp
    swap(tmp); // trade out guts, exchanges head, tail size with tmp
    return *this;
}
```

**Explicit clear-and-rebuild:**

```cpp
Chain& operator=(const Chain& other) {
    if (this == &other) return *this;
    clear();
    for (const Node* p = other.head_; p; p = p->next) {
        push_back(p->data);
    }
    return *this;
}
```

I tested both with the same small program under AddressSanitizer. It assigns a
1000-node chain over a 5-node chain, then does a self-assignment, then calls
`pop_front` + `pop_back`:

```
[copy-and-swap]
  b = a        -> b fwd 1000 bwd 1000, a fwd 1000
  b = b (self) -> b fwd 1000 bwd 1000
  pop_front + pop_back -> b size 998 fwd 998 bwd 998 front 1 back 998
  allocations 3005  deallocations 3005  leaked 0
[explicit]
  b = a        -> b fwd 1000 bwd 1000, a fwd 1000
  b = b (self) -> b fwd 1000 bwd 1000
  pop_front + pop_back -> b size 998 fwd 998 bwd 998 front 1 back 998
  allocations 2005  deallocations 2005  leaked 0
```

Both are correct, with no leaks and no sanitizer errors. The one visible
difference: copy-and-swap handles `b = b` safely *without* a self-check, but it
pays for it with 1000 extra allocations, because it copies before it swaps.

TODO: In two sentences, which one are you more confident you can write
correctly under exam pressure, and why?

A:

## 5. Why no O(1) `pop_back` on a singly-linked list with `tail_`?

TODO: Answer in two sentences. (Hint: after you delete the tail, what has to
point to `nullptr`, and what would it take to find it?)

A:

## 6. Reflection — the Rule of Three

TODO: State the Rule of Three in one sentence. Then: when would you reach for
the **Rule of Zero** instead, and why does `Chain<T>` not qualify?

A:

## AI acknowledgment

Claude Opus 5.5 (Anthropic, 2026) was used to fix bugs in my `Chain.h`
(`push_front` never set `head_` and only counted the first node; `push_back`
only set `tail_`/`size_` on an empty chain; `pop_back` had a missing `==` and a
`new_tail->next;` with no assignment) and the `printLog`/`printLogOldest`
footer formatting in `Hero.cpp`. It was also used to build the project, run the
demo transcript, perform the two "break it" exercises (the `prev` pointer and
the shallow copy, including the AddressSanitizer run), test both `operator=`
forms, capture the real output each time, and revert each change. The analysis
answers (Q2's one-sentence explanation, Q4's preference, Q5, and Q6) are my own.
