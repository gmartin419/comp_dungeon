# Floor 10 Lab Notes — The Word and the Meaning

## 1. The transcript

Pasted from the terminal. Hero name entered at the prompt: Gary. Includes three
`lookup`s (Lich, Wraith, and the absent Drake) and one `strike`.

```
> selftest registry
  index: size==5, distinct weaknesses==3, count[holy]==3: OK
  knows: Lich present, Drake absent: OK
  recall: Wraith->hp==14, Drake==nullptr: OK
  hasWeakness: holy present, water absent: OK
  countWeakness: holy==3, water==0: OK
  forget: drop a shared weakness (count--), drop a last weakness (erased), absent==false: OK
  all phases OK
> knows Lich
  McCown knows 'Lich'.
> knows Drake
  McCown does not know 'Drake'.
> weaknesses
  blunt  (2 monsters)
  fire  (6 monsters)
  holy  (3 monsters)
  light  (2 monsters)
  slashing  (1 monsters)
  water  (1 monsters)
  (6 distinct weaknesses)
> lookup Lich
Lich   HP 35   ATK 6   weakness: holy
  (looked up in the Registry)
> lookup Wraith
Wraith   HP 14   ATK 4   weakness: holy
  (looked up in the Registry)
> lookup Drake
  'Drake' is not in the Hall of Names.
> weakness fire
  6 monsters fear fire.
> names
Bone Spider   HP 6   ATK 2   weakness: fire
Cave Troll   HP 28   ATK 7   weakness: fire
Cinder Bat   HP 4   ATK 1   weakness: water
Frostmaw   HP 22   ATK 6   weakness: fire
Goblin   HP 8   ATK 2   weakness: fire
Ironclaw Bear   HP 24   ATK 5   weakness: slashing
Lich   HP 35   ATK 6   weakness: holy
Marsh Lurker   HP 18   ATK 3   weakness: fire
Necrothrall   HP 16   ATK 4   weakness: light
Ratking   HP 9   ATK 2   weakness: fire
Shadow Hound   HP 12   ATK 4   weakness: light
Skeleton   HP 10   ATK 3   weakness: blunt
Stone Sentinel   HP 30   ATK 5   weakness: blunt
Wisp   HP 3   ATK 1   weakness: holy
Wraith   HP 14   ATK 4   weakness: holy
  (15 names, alphabetical — std::map keeps them ordered)
> strike Goblin
  Struck 'Goblin' from the Hall of Names. (14 remain.)
> weakness fire
  5 monsters fear fire.
> recall Goblin
Goblin   HP 8   ATK 2   weakness: fire
  (recalled from the tree)
> benchmark names

-- The Hall of Names: linear scan vs BST vs std::map (1000 iterations per cell) --
  N=    100  query=last    linear=     0.628 us  BST=   0.130 us  std::map=   0.142 us  (tree height 13)
  N=    100  query=absent  linear=     0.095 us  BST=   0.047 us  std::map=   0.047 us  (tree height 13)
  N=   1000  query=last    linear=     4.867 us  BST=   0.165 us  std::map=   0.108 us  (tree height 22)
  N=   1000  query=absent  linear=     1.138 us  BST=   0.114 us  std::map=   0.073 us  (tree height 22)
  N=  10000  query=last    linear=    47.317 us  BST=   0.182 us  std::map=   0.154 us  (tree height 30)
  N=  10000  query=absent  linear=     9.261 us  BST=   0.132 us  std::map=   0.104 us  (tree height 30)
  N= 100000  query=last    linear=   422.140 us  BST=   0.134 us  std::map=   0.179 us  (tree height 40)
  N= 100000  query=absent  linear=    95.472 us  BST=   0.105 us  std::map=   0.136 us  (tree height 40)

std::map::find tracks the BST — both grow with log N, both crush
the linear scan. The difference is the code: the BST was a class you
wrote across a whole floor; std::map was one declaration. And unlike
your plain BST, std::map is balanced (a red-black tree), so it NEVER
degenerates into a stick — the library gives you the balanced tree of
Floor 9½ and a key->value index on top of it, for free.

> quit
McCown notes your departure. "You leave in order."
```

## 2. Set vs. map, by hand

TODO: In two sentences each, give one bestiary task that wants a `std::set`
(you only need to know something is present) and one that wants a `std::map`
(you need a value back). Name the single member call each task turns into.

A: A task for `std::set` is checking whether a weakness such as holy is in the bestiary at all, since I only need a yes or no and no value comes back. That turns into a single call, weaknesses.count("holy") (or weaknesses.contains("holy")

A task for `std::map` is finding how many monsters share a weakness, or looking up a monster's stats by name, because I need the value that goes with the key. That turns into a single find call, weaknessCounts.find("holy"), which gives back an iterator whose ->second is the count.


## 3. Why is it sorted?

The `weaknesses` and `names` output is in the transcript above. Both come out
sorted even though the monsters were inserted in file order.

TODO: In two sentences: what *are* `std::set` / `std::map` under the hood, and
which floor did you build that thing by hand?

A: `std::set` and `std::map` are balanced binary search trees that keep their keys in sorted order, which is why `weaknesses` and `names` print alphabetically no matter what order the monsters were inserted in. I built that thing by hand on Floor 9, and Floor 9 and a half was the balancing step that stops it from turning into a stick


## 4. `recall`, then and now

TODO: In two sentences: what did you give up by switching `recall` from your
Floor 9 recursive descent to one `map::find`, and what did you gain?

A: I gave up seeing and controlling the search myself, since the descent, the comparisons, and the recursion are all hidden inside the library call. I gained one line of code that is already tested, balanced so it never degenerates into a stick


## 5. The Phantom Key

TODO: Explain why `weaknessCounts["thunder"]` used to *read* a count corrupts
the map. What does `operator[]` do on a missing key, what should you have
called instead, and how does making `countWeakness` `const` turn the bug into
a compile error?

A: operator[] does not just read, on a missing key it inserts a new entry with a default value (0 for an `int`) and then returns a reference to it. So weaknessCounts["thunder"] with no monster who fears thunder silently adds a "thunder": 0 entry, which makes `weaknesses` report a sixth weakness that does not exist and makes the distinct count wrong. I should have called `find` instead, and returned 0 if the iterator equals end(), because find only looks and never changes the map. Making `countWeakness` `const` turns the bug into a compile error because operator[] is a non-const function that may insert, so a const member function is not allowed to call it on weaknessCounts_, and the compiler stops me before the bug can run.


## 6. The pair and the `const` key

TODO: Iterating a `std::map` hands you a `std::pair<const Key, Value>`. Why is
the key half `const` — what would break if you could mutate a key in place
while it sat in the tree?

A: The key is `const` because the tree is ordered by key, and each entry sits in the position its key determines. If I could change a key in place, the entry would stay where it was while its key changed, so it would be out of order in the tree. After that, `find` would walk the wrong way and miss it, and iteration would no longer be sorted, so the whole map would silently break. To change a key, I have to erase the entry and insert a new one so it lands in the correct spot.


## 7. Three views, one source

After `strike Goblin`, `recall Goblin` still found it in the tree (transcript
above). A separate run of `strike Goblin` then `names` confirmed the Registry
side:

```
> strike Goblin
  Struck 'Goblin' from the Hall of Names. (14 remain.)
> names
  ... (Goblin no longer listed) ...
  (14 names, alphabetical — std::map keeps them ordered)
```

TODO: In two sentences: why is that correct, not a bug — why can the Bag, the
BST, and the Registry legitimately disagree about whether "Goblin" exists?

A: It is correct because `strike` only removes Goblin from the Registry's own containers, while the BST and the Bag are separate containers with their own copies of the data, so they were never told. Each view is its own container that holds its own copy, so unless I strike Goblin from every one of them they can legitimately disagree, and keeping them in sync is the job of whoever updates them.


## 8. Reflection

TODO: One paragraph. What changed in how you work when the data structure is
a black box you trust instead of code you maintain? And: the map answers in
O(log n) by keeping everything in order — what would you trade away to answer
in O(1)?

A: When the structure is a black box I trust, I stopped worrying about pointers, rotations, and cleanup, and spent my time on picking the right container for the question I was asking and reading what it promises, like the `operator[]` insertion behavior we got burned by, instead of debugging my own code. My attention moved from how the thing works to how to use it correctly and what its contract says. To get O(1) I would use a hash table (`std::unordered_map`), and I would trade away the sorted order: names would no longer come out alphabetical, I would lose cheap range queries and finding the next-largest key, and the worst case could degrade if many keys collide, plus the extra memory for the buckets. That is the right trade when I only need exact lookups, and the wrong one when I need things in order.


## AI acknowledgment

Claude Opus 5.5 (Anthropic, 2026) was used to build the project and run the
demo commands in a properly configured build environment, and to paste the raw
terminal output into this file. The code and all analysis (every TODO above)
are my own.
