# Floor 6 Lab Notes — The Last Word First

## 1. The transcript

Demo sequence, pasted from the terminal:

```
> selftest stack
  empty Stack<int>: size==0 && empty(): OK
  push(7): size==1 && top()==7: OK
  push 1,2,3: size==3 && top()==3 (LIFO): OK
  after pop(): size==2 && top()==2: OK
  drain to empty: size==0 && empty(): OK
  all phases OK
> lint ({a} (b) [c])
  balanced
> lint ({)}
  not balanced
> lint hello (world
  not balanced
> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
> take Lantern
  Taken: Lantern  (wt 0.1, val 1)
> sort inventory by weight
   1.  Iron key          (wt 0.1, val 0)
   2.  Loaf of bread     (wt 0.1, val 1)
   3.  Lantern           (wt 0.1, val 1)
   4.  Healing potion    (wt 0.5, val 12)
   5.  Cloak of shadows  (wt 1.5, val 80)
   6.  Rusty sword       (wt 4.0, val 5)
> undo
Undid: sort inventory by weight
> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
   6.  Lantern           (wt 0.1, val 1)
> undo
Undid: take Lantern
> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
> undo
Nothing to undo.
> quit
The cantor falls silent. The last echo decays. The hall is quiet.
```

(Hero name entered at the prompt: Gary.)

## 2. The ADT-vs-DS one-pager

TODO: In two short paragraphs, define *abstract data type* and *data structure*
in your own words, then state which one `Stack` is and which one `Chain` is.
End with one sentence on why `Stack<T>` as an adapter over `Chain<T>`
reinforces the distinction.

A: An abstract data type is basically a list of things you’re allowed to do and what’s supposed to happen when you do them. 
It doesn’t say how any of it is stored. A data structure is the actual thing in memory that makes it work, like nodes and pointers, or an array

So Stack is the ADT, because all it promises is push, pop, top, and empty with last-in-first-out behavior. Chain is the data structure, because it’s the real linked nodes doing the work. Stack<T> sitting on top of Chain<T> as an adapter makes the difference obvious, since the stack just passes its calls down to the chain and nobody using the stack knows or cares what’s underneath

## 3. The cheap swap

`Bag<T>` has no `pop_back()` or `back()`, so for this experiment I temporarily
added three one-line wrappers to `Bag.h` (`pop_back`, and const / non-const
`back`), then changed `Stack.h` to use `Bag<T>` with `push_back`, `pop_back`,
and `back()` in place of `push_front`, `pop_front`, and `head()->data`.
Built, then ran `selftest stack` and `benchmark log 100000`:

```
> selftest stack
  empty Stack<int>: size==0 && empty(): OK
  push(7): size==1 && top()==7: OK
  push 1,2,3: size==3 && top()==3 (LIFO): OK
  after pop(): size==2 && top()==2: OK
  drain to empty: size==0 && empty(): OK
  all phases OK
> benchmark log 100000
  N= 100000   Chain::push_front =     5.69 ms   Bag::insert(begin) =  5665.74 ms
```

Every phase passes with the `Bag`-backed stack. `benchmark log` printed the same
shape of result as before (a ~1000x gap); it does not use `Stack` at all.
Both files were restored with `git checkout` afterwards and rebuilt.

TODO: Does anything change, and why or why not?

A: Nothing visible changed, selftest stack passed every phase with Bag the same as with Chain. The benchmark log result is the same shape as before because it never touches stack, it just compares Chain::push_front to Bag::insert(begin). I think the stack stayed fast because it only ever works at one end, so it never hit the slow front-insert that Bag has. If it had used Bag::insert(begin) it would still be right but a lot slower

## 4. An empty stack call

Scratch program (outside the repo), calling `top()` on an empty `Stack<int>`
with no `empty()` check:

```cpp
Stack<int> s;
std::cout << "about to call top() on an empty stack...\n" << std::flush;
std::cout << "top() = " << s.top() << "\n";
std::cout << "survived\n";
```

Built with `/O2`, run result:

```
about to call top() on an empty stack...
```

The program died there with exit code `-1073741819` (`0xC0000005`, a Windows
access violation). It never printed `top() = ...` or `survived`. `top()` is
`chain_.head()->data`, and `head()` is `nullptr` on an empty chain.

TODO: Add the guard in your own scratch copy. In one sentence: what does the
stack ADT *not* promise that a queue, a list, or a map also doesn't promise,
and what should callers always do?

guard: T& top() {
    if (empty())
        throw std::out_of_range("top() called on an empty stack");
    return chain_.head()->data;
}

Ran the guarded scratch copy (`top()` wrapped in `try`/`catch (std::out_of_range)`),
built with `/O2`:

```
about to call top() on an empty stack...
top() = caught: top() called on an empty stack
survived
```

The access violation is gone: the program reports the error and keeps running.
(`top() = ` prints first because the `<<` chain evaluates left to right before
`top()` throws.)

A: The stack ADT doesn’t promise any answer for the top element when there are no elements, so callers should always check empty() before calling top() or pop(). The crash happened because head() was nullptr and ->data dereferenced it, which is the access violation

## 5. One recursive function, one stack

TODO: Rewrite `binarySearchRecursive` (or another recursive function)
iteratively using an explicit `Stack<Range>` in a scratch file. Run a small
test, paste both versions side by side, and in two sentences say what the
explicit-stack version *shows* that the recursive one *hides*.

Scratch file (outside the repo, using the real `Stack<T>` from `hero/Stack.h`):

```cpp
// Recursive                                  // Iterative, explicit Stack<Range>
int binarySearchRecursive(                    struct Range { int lo, hi; };
    const std::vector<int>& v, int target,
    int lo, int hi) {                         int binarySearchIterative(const std::vector<int>& v, int target) {
  if (lo > hi) return -1;                       Stack<Range> pending;
  int mid = lo + (hi - lo) / 2;                 pending.push({0, (int)v.size() - 1});
  if (v[mid] == target) return mid;             while (!pending.empty()) {
  if (v[mid] < target)                            Range r = pending.top();
    return binarySearchRecursive(                 pending.pop();
        v, target, mid + 1, hi);                  if (r.lo > r.hi) continue;
  return binarySearchRecursive(                   int mid = r.lo + (r.hi - r.lo) / 2;
      v, target, lo, mid - 1);                    if (v[mid] == target) return mid;
}                                                 if (v[mid] < target) pending.push({mid + 1, r.hi});
                                                  else                 pending.push({r.lo, mid - 1});
                                                }
                                                return -1;
                                              }
```

Test, `v = {1, 3, 5, 7, 9, 11}`, built with `/O2`:

```cpp
std::cout << binarySearchRecursive(v, 7, 0, 5) << " " << binarySearchIterative(v, 7) << "
";
std::cout << binarySearchRecursive(v, 4, 0, 5) << " " << binarySearchIterative(v, 4) << "
";
```

```
3 3
-1 -1
```

Both versions agree (found at index 3; 4 is absent, so -1).

A: The explicit-stack version shows the pending work that recursion keeps hidden. Each `Range` on the stack is exactly what a call frame holds, and you can see it pushed, popped, and the stack run empty when the search ends. The recursive version hides all of that inside the call stack, which the compiler and the CPU manage for you out of sight.

## 6. Reflection

TODO: One paragraph. You wrote four one-line methods this week. What did you
spend the rest of class time on, and why was that the right ratio? Compare to
Floor 4.

A: This week we wrote four one-line methods, and most of class went to testing around them. The self-test, the Bag swap, and crashing the empty stack on purpose. I think that was the right ratio, because the code was easy but the idea wasn’t. A stack only gives LIFO, and the idea was seeing what it doesn’t give, like what top() does when it’s empty. Floor 4 was the opposite. It had a destructor, a copy constructor, and operator=. If a copy just copies the pointers, two objects both delete the same nodes, which is a double-free. Stack was easy because Chain already handled that underneath

## AI acknowledgment

Claude Sonnet 5.5 (Anthropic, 2026) was used to build the project, run the demo
transcript, and carry out the cheap-swap and empty-stack experiments (making
the temporary changes, capturing the real output, and reverting them) in a
properly configured build environment. The code and all analysis (every TODO
above) are my own.
