# Floor 9 Lab Notes — A Tree That Sorts Itself

## 1. The transcript

Demo sequence, pasted from the terminal (`selftest bst`, `catalogue`,
`recall Lich`, `recall Wraith`, `recall Drake` — which is absent — then
`forget Goblin`, `forget Wraith`, `forget Drake` (absent), `catalogue` again,
`benchmark bst`, `quit`):

```
=== THE HALVING HALL ===

What is your name, adventurer? Gary

Welcome, Gary.
Scrivener McCown stands before a tall, narrow chart where every name
forks just twice — smaller to the left, larger to the right. "On the
last floor my tree remembered where things belonged," he says. "This
one remembers where they ARE. Ask it for a beast by name; watch it
throw away half the chart with every question."

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
   catalogue                     — list the bestiary in name order (BST in-order)
   recall <name>                 — look up a monster fast in the BST
   forget <name>                 — remove a monster from the BST
   log [n]                       — show the last n events, newest first
   log --oldest [n]              — show the first n events, oldest first
   clone hero                    — deep-copy the hero, print both logs, let the copy die
   selftest chain                — Chain<T> leak + deep-copy harness
   selftest iterator             — Chain<T>::iterator harness
   selftest stack                — Stack<T> push/pop/top/LIFO harness
   selftest queue                — Queue<T> enqueue/dequeue/front/FIFO harness
   selftest tree                 — Tree<T> addChild/size/height/traversal/lineage harness
   selftest bst                  — BST<T> insert/find/inorder/remove harness
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   benchmark log [N]             — race Chain::push_front vs vector insert(begin)
   benchmark bst [N]             — race linear scan vs BST lookup over the bestiary
   battle warden                 — face the Warden of the Foundations
   help                          — this screen
   quit                          — leave the dungeon)

> selftest bst
  empty BST<int>: empty() && size==0 && height==0: OK
  insert: root==5 and size==7 over the sample: OK
  contains: 7 and 2 present, 6 and 404 absent: OK
  inOrder == {2,3,4,5,7,8,9} (ascending): OK
  degenerate: sorted inserts give height==5, not balanced: OK
  deep copy: mutating the original leaves the copy unchanged: OK
  remove: leaf(2), one-child(3), two-children(5) all correct: OK
  all phases OK
> catalogue
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
  (15 creatures, alphabetical; tree height 6)
> recall Lich
Lich   HP 35   ATK 6   weakness: holy
  (recalled from the tree)
> recall Wraith
Wraith   HP 14   ATK 4   weakness: holy
  (recalled from the tree)
> recall Drake
  'Drake' is not in McCown's catalogue.
> forget Goblin
  McCown strikes 'Goblin' from the tree. (14 remain.)
> forget Wraith
  McCown strikes 'Wraith' from the tree. (13 remain.)
> forget Drake
  'Drake' is not in the tree to forget.
> catalogue
Bone Spider   HP 6   ATK 2   weakness: fire
Cave Troll   HP 28   ATK 7   weakness: fire
Cinder Bat   HP 4   ATK 1   weakness: water
Frostmaw   HP 22   ATK 6   weakness: fire
Ironclaw Bear   HP 24   ATK 5   weakness: slashing
Lich   HP 35   ATK 6   weakness: holy
Marsh Lurker   HP 18   ATK 3   weakness: fire
Necrothrall   HP 16   ATK 4   weakness: light
Ratking   HP 9   ATK 2   weakness: fire
Shadow Hound   HP 12   ATK 4   weakness: light
Skeleton   HP 10   ATK 3   weakness: blunt
Stone Sentinel   HP 30   ATK 5   weakness: blunt
Wisp   HP 3   ATK 1   weakness: holy
  (13 creatures, alphabetical; tree height 5)
> benchmark bst
  (the table is in section 5)
> quit
McCown notes your departure. "You leave in order."
```

Goblin and Wraith are gone from the second `catalogue`, the rest is still
alphabetical, and the count went from 15 to 13.

## 2. The invariant, by hand

Inserting `8, 3, 10, 1, 6, 14, 4, 7, 13` into an empty BST, one at a time.
These drawings were generated by running the values through the real
`BST<int>` as a check against the paper drawings. Each tree is rotated 90°: the
root is at the left, the **right** subtree is drawn above, and the **left**
subtree below.

```
after insert(8)   (size 1, height 0)
8

after insert(3)   (size 2, height 1)
8
      3

after insert(10)   (size 3, height 1)
      10
8
      3

after insert(1)   (size 4, height 2)
      10
8
      3
            1

after insert(6)   (size 5, height 2)
      10
8
            6
      3
            1

after insert(14)   (size 6, height 2)
            14
      10
8
            6
      3
            1

after insert(4)   (size 7, height 3)
            14
      10
8
            6
                  4
      3
            1

after insert(7)   (size 8, height 3)
            14
      10
8
                  7
            6
                  4
      3
            1

after insert(13)   (size 9, height 3)
            14
                  13
      10
8
                  7
            6
                  4
      3
            1
```

Tracing `find(7)` on the final tree:

```
at 8: 7 < 8 -> go LEFT
at 3: 7 > 3 -> go RIGHT
at 6: 7 > 6 -> go RIGHT
at 7: equal -> FOUND
nodes looked at: 4 of 9; never looked at: 5 (10, 1, 4, 14, 13); tree height: 3
```

TODO (your own words): redo the nine inserts on paper and compare with the
drawings above. Then, in one sentence: how does the number of nodes `find(7)`
looked at (4) relate to the tree's height (3)?

A: The number of nodes looked at (4) is the height plus one, 3 + 1, because find follows a single root to node path and a path of 3 edges touches 4 nodes. So the height is the upper limit on the work, no lookup can ever examine more than height + 1 nodes

## 3. In-order is sorted

In-order traversal of the tree above:

```
1, 3, 4, 6, 7, 8, 10, 13, 14
```

That is ascending.

TODO: In two sentences: *why* does "left, node, right" produce sorted order —
what does the invariant guarantee about everything in the left subtree?

A: The invariant guarantees that everything in a node’s left subtree is smaller than the node and everything in its right subtree is larger. So visiting the left subtree first, then the node, then the right subtree emits all the smaller values, then the node, then all the larger values. Applied recursively at every node = ascending order

## 4. The three removals

From the tree above, removing a leaf, then a one-child node, then the root.
Same drawing style (rotated 90°). The case named in each heading is what the
node looked like when it was removed:

```
after remove(1)   [leaf -> LEAF case]   (size 8, height 3)
            14
                  13
      10
8
                  7
            6
                  4
      3

after remove(10)   [one-child node -> ONE-CHILD case]   (size 7, height 3)
      14
            13
8
                  7
            6
                  4
      3

after remove(8)   [root, two children -> TWO-CHILDREN case]   (size 6, height 3)
      14
13
                  7
            6
                  4
      3
```

For the root, its in-order successor — the leftmost node of its right
subtree — was **13**, so 13 moved up into the root. In-order after all three
removals: `3, 4, 6, 7, 13, 14` (still ascending).

TODO: Compare with your own paper drawings. For the root: in your own words,
why was the node that moved up (13) the only legal choice?

A: 13 was the only legal choice because it was the leftmost node and it is the smallest value that is still bigger than 8. so naturally it had to move up

## 5. The degeneracy experiment

`benchmark bst`, one run, MSVC Release build. The tree here is built from
*shuffled* data:

```
-- Bestiary: linear scan vs BST lookup (1000 iterations per cell) --
  N=    100  query=last    linear=     0.585 us  BST=     0.115 us  (tree height 13)
  N=    100  query=absent  linear=     0.085 us  BST=     0.081 us  (tree height 13)
  N=   1000  query=last    linear=     5.473 us  BST=     0.263 us  (tree height 22)
  N=   1000  query=absent  linear=     1.898 us  BST=     0.263 us  (tree height 22)
  N=  10000  query=last    linear=    55.945 us  BST=     0.309 us  (tree height 30)
  N=  10000  query=absent  linear=     7.284 us  BST=     0.154 us  (tree height 30)
  N= 100000  query=last    linear=   604.956 us  BST=     0.203 us  (tree height 40)
  N= 100000  query=absent  linear=   249.014 us  BST=     0.155 us  (tree height 40)

Linear scan grows with N; BST lookup grows with log N — the same
win binary search gave you on Floor 1, but now the structure STAYS
searchable as you insert and remove, with no re-sort step.
One catch: this tree was built from SHUFFLED data. Insert the
monsters already sorted and the BST degenerates into a stick
(height ~N), and lookup drops right back to linear — the problem
the next floor's self-balancing tree exists to solve.
```

Linear scan went from 0.585 us to 604.956 us (about 1000x) as N went from 100
to 100,000, while the BST went from 0.115 us to 0.203 us.

To see the closing note for myself, I also inserted the 15 real bestiary
names into a `BST<std::string>` in **alphabetical** order (instead of the
file's shuffled order):

```
size 15, height 14
recall Wraith (the last name) looks at 15 of 15 nodes
```

For comparison, the catalogue in section 1 — built from the shuffled file —
has height 6.

TODO: In two sentences, explain the closing note: if you had inserted the
monsters in *alphabetical* order instead of shuffled, what would the tree's
height have been, and what would have happened to `recall`'s speed?

A: IF the monsters had been inserted in alphabetical order, every new name would be larger than all the ones before it and would go to the right of the last node, so the tree would be a stick with a height of 14, not 6. Then recall would lose its halving advantage and drop back to linear speed, looking up Wraith, the last name, would examine all 15 of 15 nodes, exactly like the linear scan

## 6. Reflection

TODO: One paragraph. You have now built five containers on pointers —
`Chain`, `Stack`, `Queue`, `Tree`, and `BST`. This is the first one that is
*fast because of its shape*. What did the ordering rule buy you that the
general `Tree` couldn't, and what did it cost you (think about `remove`)?
And: the destructor you *read* on Floor 8, you *wrote* this week. Did it feel
different to write than to read?

A: The ordering rule is what makes the BST fast because of its shape. In the general Tree, finding a name meant checking every node, but the BST throws away a whole subtree with each comparison, so lookup cost follows the height instead of the size. That’s why the benchmark stayed around 0.2 µs while the linear scan grew about 1000x. The cost showed up in remove: in Tree you could just detach a node, but here the invariant has to survive every deletion. That forces three separate cases leaf, one child, two children, and the two-children case needs the in order successor to be found and moved up. The speed also depends on the data’s insertion order, because sorted inserts turn the tree into a stick. Writing the destructor felt different from reading it on Floor 8. Reading it, I could just trust that each node frees its children. Writing it, I had to think about ownership, ordering, and what happens if a pointer is deleted twice or never, so I understood why it works rather than just seeing that it does. 

nice Riddler costume by the way!



## AI acknowledgment

Claude Sonnet 5.5 (Anthropic, 2026) was used to run the demo transcript and
benchmark, to generate the tree drawings and the `find` trace used as a check
for my paper work, and to run the sorted-insert height experiment, all in
scratch copies. While preparing these notes it also found a bug in my
`removeFrom`: the right-hand branch repeated `value < n->data` instead of
`n->data < value`, which made `selftest bst` phase 7 fail and made `forget`
silently remove the wrong node for any name to the right of a node. I
approved the one-token fix (`BST.h`, `removeFrom`), and the output above is
from the fixed code. The analysis (all TODO sections above) is my own.
