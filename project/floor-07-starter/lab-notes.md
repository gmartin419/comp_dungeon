# Floor 7 Lab Notes — First In, First Served

## 1. The transcript

Demo sequence, pasted from the terminal (`selftest queue`, three `potato`
runs, `provoke Wraith`, `provoke Cave Troll`, `provoke Lich`, `simulate`,
`simulate` again on the now-empty line, `log 8`, `quit`):

```
=== THE CISTERNS ===

What is your name, adventurer? Gary

Welcome, Gary.
Steward Quill sits behind a low ironwood desk in a soaked grey robe.
He does not look up. "Take your number. I will see you in order."
The line behind you stretches further than you can see.

(commands:
   search <name>                 — look up by name in bestiary, inventory, OR event log
   list                          — list the bestiary
   inventory                     — list your inventory
   inspect <n>                   — show the nth item in your inventory
   take <name>                   — add a small item by that name to your inventory
   sort inventory by <key> [asc|desc]
                                 — key is name, weight, or value
   undo                          — reverse the most recent mutating command
   lint <text>                   — check that brackets/parens/braces in <text> balance
   provoke <name>                — enqueue an enemy action onto the line
   simulate                      — drain the enemy line in FIFO order
   potato <name>... <k>          — classical hot-potato survivor (FIFO elimination)
   log [n]                       — show the last n events, newest first
   log --oldest [n]              — show the first n events, oldest first
   clone hero                    — deep-copy the hero, print both logs, let the copy die
   selftest chain                — Chain<T> leak + deep-copy harness
   selftest iterator             — Chain<T>::iterator harness
   selftest stack                — Stack<T> push/pop/top/LIFO harness
   selftest queue                — Queue<T> enqueue/dequeue/front/FIFO harness
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   benchmark log [N]             — race Chain::push_front vs vector insert(begin)
   battle warden                 — face the Warden of the Foundations
   help                          — this screen
   quit                          — leave the dungeon)

> selftest queue
  empty Queue<int>: size==0 && empty(): OK
  enqueue(7): size==1 && front()==7: OK
  enqueue 1,2,3: size==3 && front()==1 (FIFO): OK
  after dequeue(): size==2 && front()==2: OK
  drain to empty: size==0 && empty(): OK
  all phases OK
> potato A B C D E 3
  Survivor: D
> potato Grix Skrit Quill Vael Resh 2
  Survivor: Quill
> potato OnlyOne 7
  Survivor: OnlyOne
> provoke Wraith
  Wraith takes a number and joins the line.
> provoke Cave Troll
  Cave Troll takes a number and joins the line.
> provoke Lich
  Lich takes a number and joins the line.
> simulate
Turn 1: Wraith stirs in the dark.
Turn 2: Cave Troll stirs in the dark.
Turn 3: Lich stirs in the dark.
All enemies have acted; the line is empty.
> simulate
no enemies in the line. Use provoke <name> first.
> log 8
   1.  simulate turn 3: Lich stirs in the dark
   2.  simulate turn 2: Cave Troll stirs in the dark
   3.  simulate turn 1: Wraith stirs in the dark
   4.  provoke Lich
   5.  provoke Cave Troll
   6.  provoke Wraith
   7.  potato — survivor OnlyOne
   8.  potato — survivor Quill
  (newest first; chain length 10)
> quit
Quill stamps your ticket. The line shuffles forward by one.
```

The three enemies came out in exactly the order they went in: Wraith, Cave
Troll, Lich.

## 2. The contrast one-pager — `Stack<T>` vs `Queue<T>`

The five methods of each adapter, copied from `hero/Stack.h` and
`hero/Queue.h` (reformatted to one line each) and set side by side:

```cpp
// Stack<T>  (Floor 6)                         // Queue<T>  (Floor 7)
void push(const T& v) { chain_.push_front(v); }   void enqueue(const T& v) { chain_.push_back(v); }
void pop()            { chain_.pop_front(); }     void dequeue()           { chain_.pop_front(); }
T&   top()            { return chain_.head()->data; }   T&   front()      { return chain_.head()->data; }
size_t size() const   { return chain_.size(); }   size_t size() const     { return chain_.size(); }
bool   empty() const  { return size() == 0; }     bool   empty() const     { return size() == 0; }
```

| Job              | `Stack<T>`                  | `Queue<T>`                  | Same `Chain<T>` call? |
|------------------|-----------------------------|-----------------------------|-----------------------|
| add an item      | `push` → `push_front`       | `enqueue` → `push_back`     | **no** — front vs back |
| remove an item   | `pop` → `pop_front`         | `dequeue` → `pop_front`     | yes                   |
| read next-served | `top` → `head()->data`      | `front` → `head()->data`    | yes                   |
| count            | `size` → `chain_.size()`    | `size` → `chain_.size()`    | yes                   |
| is it empty      | `size() == 0`               | `size() == 0`               | yes                   |

Only one of the five rows differs: which end of `chain_` the new item goes in at.

TODO (your own words): two short paragraphs on what changed between the
adapters. Name the specific `Chain<T>` methods each one calls, and say which
end of the chain each operation touches.

A: stack<T> and Queue<T> are both thin wrappers around the same Chain<T>, and four of their five methods call exactly the same Chain<T> functions. Both remove with chain_.pop_front(), both read the next item with chain_.head()->data, and both get their size from chain_.size(), with empty() written as size() == 0. So removing and reading happen at the front of the chain for both adapters

The only thing that changed is where a new item goes in. Stack::push calls chain_.push_front(v), so the newest item lands at the front. Queue::enqueue calls chain_.push_back(v), so the newest item goes on the back while dequeue and front still work at the front. That one change from the front to the back of the chain is what turns last-in-first-out into first-in-first-out.

TODO: finish this sentence — *"A stack and a queue are different because…"*

A: A stack and a queue are different because a stack serves the item that was added most recently, while a queue serves the item that has waited longest

## 3. The cheap swap, redux — `Queue<T>` over `Bag<T>`

All of this was done in a scratch copy of the floor 7 folder, so your real
`hero/Queue.h` and `hero/Bag.h` were never touched.

In the scratch `Queue.h` I swapped `Chain<T> chain_` for `Bag<T> bag_`
(`push_back` → `bag_.push_back`, `front` → `bag_[0]`, `size` → `bag_.size()`)
and pointed `dequeue` at `bag_.pop_front()`. It does not compile:

```
hero\Queue.h(76): error C2039: 'pop_front': is not a member of 'dungeon::Bag<T>'
```

`Bag<T>` has no `pop_front`, and no `erase` or `remove` of any kind. The
closest honest equivalent is to erase element 0 of the vector it wraps, which
shifts every remaining element down one slot. I added exactly that to the
scratch `Bag.h`:

```cpp
void pop_front() { if (!data_.empty()) data_.erase(data_.begin()); }   // O(n)
```

`selftest queue` with the Bag-backed `Queue<T>` (in the full game build):

```
> selftest queue
  empty Queue<int>: size==0 && empty(): OK
  enqueue(7): size==1 && front()==7: OK
  enqueue 1,2,3: size==3 && front()==1 (FIFO): OK
  after dequeue(): size==2 && front()==2: OK
  drain to empty: size==0 && empty(): OK
  all phases OK
```

It still passes, so the ADT contract holds. Then the timing run: a small
`main` that enqueues N `std::string` names ("name0", "name1", …) and then
dequeues them all, built with MSVC `/O2` against each version of `Queue.h`.
One run per cell, so treat the numbers as approximate:

| N       | `Chain` enqueue all | `Chain` dequeue all | `Bag` enqueue all | `Bag` dequeue all |
|--------:|--------------------:|--------------------:|------------------:|------------------:|
|  10,000 |           1.02 ms   |            0.27 ms  |          0.81 ms  |           90.2 ms |
|  20,000 |           1.72 ms   |            0.53 ms  |          1.49 ms  |          327.0 ms |
|  40,000 |           4.04 ms   |            1.51 ms  |          4.44 ms  |        1,358.6 ms |
| 100,000 |          10.67 ms   |            3.61 ms  |          8.62 ms  |       10,167 ms   |

At N = 100,000 the whole run took about 14.3 ms with `Chain` and about
10,176 ms (just over 10 seconds) with `Bag` — roughly 700 times slower, and
all of the difference is in `dequeue`. Enqueueing cost about the same either
way. Going from N = 10,000 to 20,000 to 40,000, the `Bag` dequeue time grew
about 3.6x and then 4.2x per doubling (roughly quadrupling), while the
`Chain` dequeue time stayed under 4 ms even at 100,000.

Restore: nothing to restore — the project's own `Queue.h` still uses `Chain<T>`.

TODO: What did you pay for using `Bag<T>` instead of `Chain<T>`, and *why*?
(Say which operation got slow and what it has to do on every call.)

A: I paid in speed on dequeue. Bag<T> has no pop_front, so the closest I could do was data_.erase(data_.begin()) on the vector inside it. Every call has to shift all of the remaining elements down one slot, so each dequeue is O(n), while Chain::pop_front only moves the head pointer and is O(1). At N = 100,000 dequeuing everything took about 10,167 ms with Bag versus 3.61 ms with Chain, roughly 700 times slower overall, and the Bag dequeue time roughly quadrupled each time N doubled, which is what n calls that each cost O(n) should look like. Enqueueing cost about the same either way, so the contract was identical but the cost was not

## 4. An empty queue call

A small `main` that calls `front()` on an empty `Queue<int>` with no
`empty()` check:

```cpp
Queue<int> q;
std::cout << "about to call front() on an empty queue...\n" << std::flush;
std::cout << "front = " << q.front() << "\n";
std::cout << "main() finished normally\n";
```

Output (same result in a Release `/O2` build and a Debug `/Od /Zi` build):

```
about to call front() on an empty queue...
```

Then the program died. The process exit code was `-1073741819`, which is
`0xC0000005` — `STATUS_ACCESS_VIOLATION`. The `front = ...` line and the
`main() finished normally` line never printed. That is a hard crash, not
garbage and not silent UB: `chain_.head()` is `nullptr` on an empty chain, and
`front()` reads `->data` through it.

With the guard added:

```cpp
if (!q.empty()) {
    std::cout << "front = " << q.front() << "\n";
} else {
    std::cout << "the queue is empty - nothing to read\n";
}
```

```
the queue is empty - nothing to read
main() finished normally
```

Exit code 0.

TODO: In one sentence — why does the queue ADT *deliberately not* throw on
`front()` of an empty queue, and what does that tell you about ADT design
philosophy?

A: The queue ADT deliberately doesn't throw on front() of an empty queue because calling it on an empty queue is a precondition violation that the caller is expected to prevent by checking empty() first, and that tells me an ADT is defined by a contract of sorts where the caller is responsible for using it correctly

## 5. Queue from two stacks

Scratch file `StackQueue.h` (not part of the project): same five-method
surface as `Queue<T>`, built from two `Stack<T>` objects. `enqueue` pushes
onto `in_`. When `front()` or `dequeue()` needs the oldest element and `out_`
is empty, everything is popped off `in_` and pushed onto `out_`, which
reverses it so `out_.top()` is the oldest.

```cpp
void enqueue(const T& value) { in_.push(value); }
void dequeue()               { shift(); out_.pop(); }
T&   front()                 { shift(); return out_.top(); }
std::size_t size() const     { return in_.size() + out_.size(); }
bool        empty() const    { return size() == 0; }

void shift() const {                    // only moves when out_ is empty
    if (out_.empty())
        while (!in_.empty()) { out_.push(in_.top()); in_.pop(); }
}
```

I ran the real `QueueTests.cpp` harness with every `Queue<int>` replaced by
`StackQueue<int>` (the only other changes: the include and the function name):

```
> selftest queue   (harness instantiated as StackQueue<int>)
  empty StackQueue<int>: size==0 && empty(): OK
  enqueue(7): size==1 && front()==7: OK
  enqueue 1,2,3: size==3 && front()==1 (FIFO): OK
  after dequeue(): size==2 && front()==2: OK
  drain to empty: size==0 && empty(): OK
  all phases OK
```

To see the transfer burst, I enqueued 100,000 strings and then timed the
first four `dequeue()` calls on each version (MSVC `/O2`, one run):

```
Queue<std::string> (Chain), 100000 items enqueued
  dequeue #1: 1 us
  dequeue #2: 0.2 us
  dequeue #3: 0.1 us
  dequeue #4: 0.2 us
StackQueue<std::string> (two stacks), 100000 items enqueued
  dequeue #1: 8093 us
  dequeue #2: 0.1 us
  dequeue #3: 0.1 us
  dequeue #4: 0 us
```

The first `dequeue` on the two-stack version took about 8 ms because it moved
all 100,000 items from `in_` to `out_`; every `dequeue` after that was
instant.

TODO: In two sentences — which version is amortized O(1) and which has
occasional O(n) transfer bursts, and why does the *ADT* not care which is
which?

A: The two-stack StackQueue is amortized O(1), because each item is moved from in_ to out_ at most once, but it has occasional O(n) bursts (the first dequeue moved all 100,000 items and took about 8 ms), while the Chain-backed Queue is O(1) on every call. The ADT doesn't care because it only promises behavior (items come out in the order they went in through enqueue, dequeue, front, size, and empty), not how fast each call is, which is why both versions passed the same selftest queue

## 6. Reflection

TODO: One paragraph. You wrote the same kind of thing twice in two weeks: a
five-method adapter that gives a contract on top of a `Chain<T>`. What feels
different the second time? What would you build *next* with this pattern if
we kept going? (Heads up: a deque is one more end-pairing decision away.)

A: The second time was a lot less about figuring out how to make it work and more about seeing the pattern. I already knew Chain<T>, so I could look at the five methods and see that almost all of them were the same call and that the only real decision was which end of the chain the new item goes in. The tests carried over almost unchanged too, which made it clear that the contract is the part that matters and the container underneath is replaceable. If we kept going I would build a deque next, since it allows adds and removes at both ends. That would need a pop_back on Chain<T>, and I would have to check whether that is cheap or whether the chain needs another pointer to make it O(1), since the Bag experiment showed that the choice of underlying container really does show up in the timings.



## AI acknowledgment

Claude Sonnet 5.5 (Anthropic, 2026) was used to run the demo transcript and
to perform the experiment exercises (the `Bag<T>` swap and timing, the
empty-queue `front()` crash, and the two-stack `StackQueue<T>`), capturing the
real compiler and runtime output. All experiments were run in scratch copies
so the project's own code was not modified. The analysis (all TODO sections
above) is my own.
