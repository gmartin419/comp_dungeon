# Floor 8 Lab Notes — Everything Has a Place

## 1. The transcript

Demo sequence, pasted from the terminal (`selftest tree`, `taxonomy`,
`taxonomy --dfs`, `taxonomy --bfs`, then `lineage` on three creatures, one of
them a category — `Lich`, `Cave Troll`, `Beasts` — plus `Goblin`, which is not
classified yet):

```
=== THE HALL OF LINEAGES ===

What is your name, adventurer? Gary

Welcome, Gary.
Scrivener McCown looks up from a great branching chart that climbs the
wall like roots. "You found the Cisterns in order. Good. Order down
there was a line. Up here it branches." He taps the root of the chart.
"Every creature in the Hold hangs somewhere on this. Ask it questions."

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
   taxonomy                      — print McCown's classification tree as an outline
   taxonomy --dfs                — list every entry depth-first (pre-order)
   taxonomy --bfs                — list every entry breadth-first (level-order)
   lineage <name>                — show the root-to-creature classification path
   log [n]                       — show the last n events, newest first
   log --oldest [n]              — show the first n events, oldest first
   clone hero                    — deep-copy the hero, print both logs, let the copy die
   selftest chain                — Chain<T> leak + deep-copy harness
   selftest iterator             — Chain<T>::iterator harness
   selftest stack                — Stack<T> push/pop/top/LIFO harness
   selftest queue                — Queue<T> enqueue/dequeue/front/FIFO harness
   selftest tree                 — Tree<T> addChild/size/height/traversal/lineage harness
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   benchmark log [N]             — race Chain::push_front vs vector insert(begin)
   battle warden                 — face the Warden of the Foundations
   help                          — this screen
   quit                          — leave the dungeon)

> selftest tree
  empty Tree<int>: empty() && size==0 && height==0: OK
  addChild: root==1 with 3 children: OK
  size()==7 over the sample tree: OK
  height()==2 (longest path 1->2->5): OK
  preOrder == {1,2,5,6,3,4,7} (depth-first): OK
  levelOrder == {1,2,3,4,5,6,7} (breadth-first): OK
  lineage(6)=={1,2,6} and lineage(404) empty: OK
  all phases OK
> taxonomy
Creatures of the Hold
    - Undead
        - Corporeal
            - Skeleton
            - Necrothrall
            - Lich
        - Incorporeal
            - Wraith
            - Wisp
    - Beasts
        - Mammals
            - Shadow Hound
            - Ironclaw Bear
            - Cinder Bat
        - Giants
            - Cave Troll
            - Frostmaw
    - Vermin
        - Bone Spider
        - Ratking
        - Marsh Lurker
    - Constructs
        - Stone Sentinel
  (23 creatures and categories; height 3)
> taxonomy --dfs
  depth-first (pre-order):
    Creatures of the Hold
    Undead
    Corporeal
    Skeleton
    Necrothrall
    Lich
    Incorporeal
    Wraith
    Wisp
    Beasts
    Mammals
    Shadow Hound
    Ironclaw Bear
    Cinder Bat
    Giants
    Cave Troll
    Frostmaw
    Vermin
    Bone Spider
    Ratking
    Marsh Lurker
    Constructs
    Stone Sentinel
> taxonomy --bfs
  breadth-first (level-order):
    Creatures of the Hold
    Undead
    Beasts
    Vermin
    Constructs
    Corporeal
    Incorporeal
    Mammals
    Giants
    Bone Spider
    Ratking
    Marsh Lurker
    Stone Sentinel
    Skeleton
    Necrothrall
    Lich
    Wraith
    Wisp
    Shadow Hound
    Ironclaw Bear
    Cinder Bat
    Cave Troll
    Frostmaw
> lineage Lich
  Creatures of the Hold > Undead > Corporeal > Lich
> lineage Cave Troll
  Creatures of the Hold > Beasts > Giants > Cave Troll
> lineage Beasts
  Creatures of the Hold > Beasts
> lineage Goblin
  'Goblin' is not in McCown's catalogue.
> quit
McCown notes your departure on the chart. "You leave classified."
```

## 2. The recursion skeleton, in your words

The three functions as they are in `hero/Tree.h` (comments trimmed, and the
three shared parts marked **(1)** base case, **(2)** act on this node,
**(3)** recurse into the children):

```cpp
static std::size_t countFrom(const Node* n) {
    if (n == nullptr) { return 0; }                               // (1)
    std::size_t total = 1;                                        // (2) count myself
    for (std::size_t i = 0; i < n->children.size(); ++i)          // (3)
        total += countFrom(n->children[i]);
    return total;
}

static std::size_t heightFrom(const Node* n) {
    if (n == nullptr || n->children.size() == 0) return 0;        // (1)
    std::size_t best = 0;
    for (std::size_t i = 0; i < n->children.size(); ++i)          // (3)
        best = std::max(best, heightFrom(n->children[i]));
    return 1 + best;                                              // (2) add my own level
}

static void preOrderInto(const Node* n, std::vector<T>& out) {
    if (n == nullptr) return;                                     // (1)
    out.push_back(n->data);                                       // (2) record myself
    for (std::size_t i = 0; i < n->children.size(); ++i)          // (3)
        preOrderInto(n->children[i], out);
}
```

TODO (your own words): circle the three parts in your own copy. In two
sentences: what is the *one* thing that differs between counting, measuring
height, and listing pre-order?

A: Only part 2, what you do at the node differs. Counting adds 1, height adds 1 to the best child’s height, and pre-order pushes n->data so the base case and the loop over children stay the same

## 3. DFS vs BFS by hand

The sample tree from `selftest tree` is `1` with children `2, 3, 4`; `2` has
children `5, 6`; `4` has the child `7` (`3` is a leaf). The self-test asserts
these two orders, and both phases pass:

```
preOrder   == {1, 2, 5, 6, 3, 4, 7}
levelOrder == {1, 2, 3, 4, 5, 6, 7}
```

The real taxonomy's two orders are in section 1 (`taxonomy --dfs` and
`taxonomy --bfs`).

TODO: write out the pre-order and level-order of the sample tree by hand
(before looking at the lines above) and check them. Then, in one sentence:
which node moved the *most* positions between the two orders on the real
taxonomy, and why?

A: By hand, the pre-order is 1, 2, 5, 6, 3, 4, 7 and the level-order is 1, 2, 3, 4, 5, 6, 7, matching the self-test. On the real taxonomy, Constructs moved the most, from 22nd in DFS to 5th in BFS, 17 positions. It’s a upper level node, so BFS lists it with the other level 1 nodes, but DFS doesn’t reach it until every subtree before is finished

## 4. The missing Goblin

I tried this in a scratch copy of the Floor 8 folder. I added one path line
to the data file:

```
Creatures of the Hold/Humanoids/Goblin
```

This puts the Goblin under a **new** `Humanoids` category rather than under
`Beasts` or `Vermin`.

Before the edit:

```
> lineage Goblin
  'Goblin' is not in McCown's catalogue.
```

I changed **only** `data/taxonomy.txt` (no `.cpp` touched) and ran the build.
This did *not* make the Goblin appear:

```
ninja: no work to do.
> lineage Goblin
  'Goblin' is not in McCown's catalogue.
```

Nothing needed recompiling, so the build did nothing, and `CMakeLists.txt`
only copies `data/` next to the executable as a `POST_BUILD` step (which runs
when the program is re-linked). The `data/` copy in `build/` was stale. Two
things fixed it:

- Launching the same `the_descent.exe` from the source folder, so it reads
  the edited `data/taxonomy.txt` directly (no rebuild at all):

```
> lineage Goblin
  Creatures of the Hold > Humanoids > Goblin
```

- Touching a `.cpp` (I touched `main.cpp`, no content change) so the build
  re-linked and re-copied `data/`:

```
[1/2] Building CXX object CMakeFiles\the_descent.dir\main.cpp.obj
[2/2] Linking CXX executable the_descent.exe
> lineage Goblin
  Creatures of the Hold > Humanoids > Goblin
> taxonomy
  ...
    - Humanoids
        - Goblin
  (25 creatures and categories; height 3)
```

The count went from 23 to 25: the new `Humanoids` category plus the Goblin.

TODO: In one sentence, argue for the branch you picked for the Goblin.

A: branch: I filed the Goblin under a new Humanoids category because it’s neither a beast, a vermin, or undead, and a new branch avoids stretching an existing category and making it mo big (hit the road dude)

TODO: Did you have to recompile any `.cpp`, or just edit the data? Why does
that distinction matter? (Use what happened above: the program reads
`data/taxonomy.txt` at run time, but the copy next to the executable only
refreshes on a re-link.)

A: recompile: I didn’t recompile any .cpp, only edited the data file. The distinction matters because the program reads taxonomy.txt at run time, so data changes need no code changes. The copy next to the executable only refreshes on a re-link, so a stale one can hide your edit

## 5. Post-order, then put it back

In a scratch copy I moved `out.push_back(n->data);` in `preOrderInto` from
before the loop to after it:

```cpp
static void preOrderInto(const Node* n, std::vector<T>& out) {
    if (n == nullptr) return;
    for (std::size_t i = 0; i < n->children.size(); ++i)
        preOrderInto(n->children[i], out);
    out.push_back(n->data);          // moved here, after the loop
}
```

`taxonomy --dfs` then printed:

```
> taxonomy --dfs
  depth-first (pre-order):
    Skeleton
    Necrothrall
    Lich
    Corporeal
    Wraith
    Wisp
    Incorporeal
    Undead
    Shadow Hound
    Ironclaw Bear
    Cinder Bat
    Mammals
    Cave Troll
    Frostmaw
    Giants
    Beasts
    Bone Spider
    Ratking
    Marsh Lurker
    Vermin
    Stone Sentinel
    Constructs
    Creatures of the Hold
```

(The heading still says "pre-order" because that text is printed by
`main.cpp`, which I didn't change.) `selftest tree` caught the change:

```
  preOrder == {1,2,5,6,3,4,7} (depth-first): FAIL — preOrderInto must push the node BEFORE recursing into its children
```

Restore: nothing to restore — the project's `Tree.h` was never changed, so
`preOrderInto` is still pre-order.

TODO: In one sentence, what changed in the order, and why is this now called
*post*-order?

A: The node now comes after its children instead of before them, so the output lists leaves first and the root last. It’s called post-order because each node is visited after its subtree

## 6. Why this tree is slow to search

In a scratch copy I added a counter to `findFrom` — `++g_findVisits;` right
after the `nullptr` check, so it counts every node `findFrom` looks at — and
called `lineage` on a few names. The tree has 23 nodes:

| `lineage(...)`          | found? | nodes `findFrom` visited |
|-------------------------|--------|-------------------------:|
| `Creatures of the Hold` | yes    |                        1 |
| `Lich`                  | yes    |                        6 |
| `Cave Troll`            | yes    |                       16 |
| `Stone Sentinel`        | yes    |               **23** (all) |
| `Goblin`                | no     |               **23** (all) |

`Stone Sentinel` is the last leaf in `data/taxonomy.txt`, so `lineage` checked
all 23 nodes to reach it. Looking up the absent `Goblin` also checks all 23
before it can say "not found". Each count equals that node's position in the
`taxonomy --dfs` listing above (`Lich` is 6th, `Cave Troll` 16th, `Stone
Sentinel` 23rd).

TODO: In two sentences: why does a general tree give you no search speedup
over McCown's old flat list — and what single rule would the structure need
before search could skip whole branches?

A: A general tree has no rule about where a name lives, so a search may have to check every node (O(n)), just like the flat list. To skip whole branches, the tree would need an ordering rule, like a BST’s “left subtree smaller, right subtree larger”, so one comparison can rule out half the remaining nodes

## 7. Reflection

TODO: One paragraph. You have now built four containers on top of pointers —
`Chain`, `Stack`, `Queue`, and `Tree`. The first three were lines; this one
branches. What felt genuinely new about the tree, and what felt like the same
lesson as before? (Heads up: the destructor you *read* this week, you will
*write* next week, because a BST owns its nodes the same way.)

A: : What felt new was that a node has many children, so recursion became the natural way to walk the structure, and there are two valid traversal orders over the same data. Height and size also fall out of the recursive pattern with only one line changing. What felt the same was the pointer ownership: each node is allocated and linked by hand, something has to free it all, and searching without extra structure is still a linear scan like Chain, Stack and Queue. The destructor is the same lesson again, which is why next week’s BST will need to delete its children before itself



## AI acknowledgment

Claude Sonnet 5.5 (Anthropic, 2026) was used to run the demo transcript and
to perform the experiment exercises (the Goblin data edit and rebuild, the
post-order change, and the `findFrom` visit counter), capturing the real
output. All experiments were run in scratch copies so the project's own code
and data were not modified. The analysis (all TODO sections above) is my own.
