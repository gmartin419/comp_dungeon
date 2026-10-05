# Floor 5 Lab Notes — One Lens, Three Shelves

## 1. The transcript

Demo sequence, pasted from the terminal (`search Goblin`, `search Iron key`,
`search began`, `log`, `log --oldest 3`, `selftest iterator`, `quit`):

```
=== THE EYE OF SCRYING ===

What is your name, adventurer? Gary

Welcome back, Gary.
Sister Vael waits at the centre of the round chamber, lens at her sternum.
She will lend you a lens — once you have built one — that walks any container.

(commands:
   search <name>                 — look up by name in bestiary, inventory, OR event log
   list                          — list the bestiary
   inventory                     — list your inventory
   inspect <n>                   — show the nth item in your inventory
   sort inventory by <key> [asc|desc]
                                 — key is name, weight, or value
   log [n]                       — show the last n events, newest first
   log --oldest [n]              — show the first n events, oldest first
   clone hero                    — deep-copy the hero, print both logs, let the copy die
   selftest chain                — Chain<T> leak + deep-copy harness
   selftest iterator             — Chain<T>::iterator + const_iterator + std::reverse harness
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   benchmark log [N]             — race Chain::push_front vs vector insert(begin)
   battle warden                 — face the Warden of the Foundations
   help                          — this screen
   quit                          — leave the dungeon)

> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
  (found in bestiary)
> search Iron key
  Iron key  (wt 0.1, val 0)
  (found in inventory)
> search began
  began session as "Gary"
  (found in event log)
> log
   1.  search began — found in event log
   2.  search Iron key — found in inventory
   3.  search Goblin — found in bestiary
   4.  began session as "Gary"
  (newest first; chain length 4)
> log --oldest 3
   1.  began session as "Gary"
   2.  search Goblin — found in bestiary
   3.  search Iron key — found in inventory
  (oldest first; chain length 4)
> selftest iterator
  range-for over Chain<int>: OK
  std::find(Chain<int>, 42): OK
  std::distance(begin, end): OK
  range-for over const Chain<int>&: OK
  std::reverse(Chain<int>) — first now == 9: OK
  all phases OK
> quit
The lens dims. The lens does not remember what it saw — only how it moved.
```

## 2. Plant the bug — `operator++` follows `prev`

Changed `iterator::operator++` from `p_ = p_->next;` to `p_ = p_->prev;`,
rebuilt, then ran `search Goblin`, `search Iron key`, `search began`, `log`:

```
> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
  (found in bestiary)
> search Iron key
  Iron key  (wt 0.1, val 0)
  (found in inventory)
> search began
No such creature, item, or past event under that name.
> log
   1.  search began — not found
  (newest first; chain length 4)
```

`log` printed **one** entry even though the chain length is 4, and
`search began` (which should have found the "began session" event) reported
"not found". Restored `p_ = p_->next;` afterwards.

TODO (optional, your own words): why does the walk stop after one entry
instead of looping forever or crashing?

## 3. Skip `end()` — `end()` wraps `head_`

Changed `iterator end()` from `iterator(nullptr, this)` to
`iterator(head_, this)`, rebuilt, ran `search Goblin`, `search Iron key`,
`log`, `selftest iterator`:

```
> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
  (found in bestiary)
> search Iron key
  Iron key  (wt 0.1, val 0)
  (found in inventory)
> log
  (the chain is empty — nothing to remember yet)
> selftest iterator
  range-for over Chain<int>: FAIL — begin == end (stub returns true) — implement operator++ and operator==
  std::find(Chain<int>, 42): FAIL — std::find returned end() — likely operator++ stub or operator== stub
  std::distance(begin, end): FAIL — expected 100 — got 0 means begin == end immediately (operator== stub)
  range-for over const Chain<int>&: OK
  std::reverse(Chain<int>) — first now == 9: FAIL — operator-- not yet wired (Friday) — std::reverse can't walk back
  (see FAILs above)
```

The chain had entries (the session-start event plus two searches), but `log`
claimed it was empty and the range-based `for` body never ran. The
`const Chain<int>&` phase still passed because only the non-const `end()` was
changed. Restored `iterator(nullptr, this)` afterwards.

TODO: In one sentence — what invariant of the iterator contract did the broken
`end()` violate?

A: Returning iterator(head_, this) made them equal, so the range [begin, end) looked empty even though the chain had entries

## 4. The `std::sort` error

Added above `printHelp();` in `main()`:

```cpp
Chain<int> sortTest; sortTest.push_front(1);
std::sort(sortTest.begin(), sortTest.end());
```

Full compiler output:

```
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): error C2676: binary '-': 'const _RanIt' does not define this operator or a conversion to a type acceptable to the predefined operator
        with
        [
            _RanIt=dungeon::Chain<int>::iterator
        ]
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\xutility(4711): note: could be 'unknown-type std::operator -(const std::move_iterator<_Iter> &,const std::move_iterator<_Iter2> &) noexcept(<expr>)'
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): note: 'unknown-type std::operator -(const std::move_iterator<_Iter> &,const std::move_iterator<_Iter2> &) noexcept(<expr>)': could not deduce template argument for 'const std::move_iterator<_Iter> &' from 'const _RanIt'
        with
        [
            _RanIt=dungeon::Chain<int>::iterator
        ]
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\xutility(2176): note: or       'unknown-type std::operator -(const std::reverse_iterator<_BidIt> &,const std::reverse_iterator<_BidIt2> &) noexcept(<expr>)'
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): note: 'unknown-type std::operator -(const std::reverse_iterator<_BidIt> &,const std::reverse_iterator<_BidIt2> &) noexcept(<expr>)': could not deduce template argument for 'const std::reverse_iterator<_BidIt> &' from 'const _RanIt'
        with
        [
            _RanIt=dungeon::Chain<int>::iterator
        ]
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): note: the template instantiation context (the oldest one first) is
C:\comp_dungeon_main\project\floor-05-starter\main.cpp(111): note: see reference to function template instantiation 'void std::sort<dungeon::Chain<int>::iterator>(const _RanIt,const _RanIt)' being compiled
        with
        [
            _RanIt=dungeon::Chain<int>::iterator
        ]
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9090): note: see reference to function template instantiation 'void std::sort<_RanIt,std::less<void>>(const _RanIt,const _RanIt,_Pr)' being compiled
        with
        [
            _RanIt=dungeon::Chain<int>::iterator,
            _Pr=std::less<void>
        ]
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): error C2672: 'std::_Sort_unchecked': no matching overloaded function found
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9051): note: could be 'void std::_Sort_unchecked(_RanIt,_RanIt,iterator_traits<_Iter>::difference_type,_Pr)'
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): note: 'void std::_Sort_unchecked(_RanIt,_RanIt,iterator_traits<_Iter>::difference_type,_Pr)': expects 4 arguments - 3 provided
```

(Change reverted afterwards — `main.cpp` is back to its committed state.)

TODO: Find and quote the line that names the iterator requirement. In one
sentence: which operation is the standard library asking for that your
iterator doesn't provide?

A: error: C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include\algorithm(9085): error C2676: binary '-': 'const _RanIt' does not define this operator or a conversion to a type acceptable to the predefined operator
----
std::sort is asking for operator- between two iterators, meaning last - first, so it can compute the distance in constant time

## 5. `auto` vs spelled-out types

Temporarily added both versions of the same loop over `hero.eventLog` and ran
them; both compile and print identical output:

```cpp
// version A — auto
for (const auto& s : hero.eventLog) std::cout << "[auto] " << s << "\n";

// version B — spelled out
for (Chain<std::string>::const_iterator it = hero.eventLog.cbegin();
     it != hero.eventLog.cend(); ++it)
    std::cout << "[spelled] " << *it << "\n";
```

```
[auto] began session as "Gary"
[spelled] began session as "Gary"
```

(Change reverted afterwards.)

TODO: In two sentences — which version is easier to maintain when you later
change the container type, and why?

A: The auto version is easier to maintain. It never names the container's iterator type, so changing Chain to another container needs no edit to the loop. The spelled-out version hardcodes Chain<std::string>::const_iterator, so every loop like it has to be found and rewritten.

## 6. Reflection — one `printLog`

TODO: One paragraph. Floor 4½ had two near-identical `printLog` functions
(forward and backward); this week they collapsed into one. In your own words,
what abstraction does the iterator type provide that lets that collapse
happen?

A: Floor 4 and a half needed two almost the same printLog functions because the list was set up differently depending on which way the list was stored. One went forward using next pointers, and the other went backward using prev pointers. The iterator fixed this by making both work the same way. It lets you read an item, move to the next one, and check when you reach the end without messing with the nodes directly. Now printLog just goes through the list and prints everything out. To print backward, you just change which iterators you use instead of writing a whole new function. This also lets things like std::find, std::distance, and std::reverse work with Chain without needing to know how the list is set up.



## AI acknowledgment

Claude Sonnet 5.5 (Anthropic, 2026) was used to build the project, run the demo
transcript, and perform the "break it" exercises (planting each bug or
compile error temporarily, capturing the real runtime/compiler output, then
reverting every change) in a properly configured build environment. The
analysis (all TODO sections above) is my own.
