# Floor 3 Lab Notes — One Mold, Many Shapes

## 1. The transcript

Demo sequence, pasted from the terminal (`inventory`, `list`, `search Goblin`,
`search Iron key`, `inspect 3`, `inspect 99`, `sort inventory by weight`, `quit`):

```
=== THE FORGEMASTER'S VAULT ===

What is your name, adventurer? Tester

Welcome back, Tester.
Empty molds line the walls. The Forgemaster watches, silent.
This week you press one mold to hold every list the keep remembers.

(commands:
   search <name>                 — look up by name in bestiary or inventory
   list                          — list the bestiary
   inventory                     — list your inventory
   inspect <n>                   — show the nth item in your inventory
   sort inventory by <key> [asc|desc]
                                 — key is name, weight, or value
   benchmark [N]                 — race the search algorithms
   benchmark sort [N] [--sorted] [--bad-pivot]
                                 — race the sorting algorithms
   help                          — this screen
   quit                          — leave the dungeon)

> inventory
   1.  Rusty sword       (wt 4.0, val 5)
   2.  Healing potion    (wt 0.5, val 12)
   3.  Iron key          (wt 0.1, val 0)
   4.  Loaf of bread     (wt 0.1, val 1)
   5.  Cloak of shadows  (wt 1.5, val 80)
> list
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
> search Goblin
Goblin   HP 8   ATK 2   weakness: fire
> search Iron key
  Iron key  (wt 0.1, val 0)
> inspect 3
  Iron key  (wt 0.1, val 0)
> inspect 99
No such item. (index 98out of bounds for size 5)
> sort inventory by weight
   1.  Iron key          (wt 0.1, val 0)
   2.  Loaf of bread     (wt 0.1, val 1)
   3.  Healing potion    (wt 0.5, val 12)
   4.  Cloak of shadows  (wt 1.5, val 80)
   5.  Rusty sword       (wt 4.0, val 5)
> quit
The forge cools. The molds stand empty again.
```

Note: the `inspect 99` message reads `index 98out of bounds` (missing a space
between the index and "out") — a small formatting bug in `BagException`'s
constructor, left as-is since it doesn't affect the exception-handling behavior
this lab is about.

## 2. Break it — the template error

Added directly above `printHelp();` in `main()`, exactly as specified:

```cpp
Bag<int> numbers;
findByName(numbers, "seven");
```

Full compiler output (Output window, not the Error List summary):

```
C:\comp_dungeon_main\project\floor-03-starter\bestiary/Search.h(88): error C2228: left of '.name' must have class/struct/union
C:\comp_dungeon_main\project\floor-03-starter\bestiary/Search.h(88): note: type is 'const int'
C:\comp_dungeon_main\project\floor-03-starter\bestiary/Search.h(88): note: the template instantiation context (the oldest one first) is
C:\comp_dungeon_main\project\floor-03-starter\main.cpp(91): note: see reference to function template instantiation 'const T *dungeon::findByName<int>(const dungeon::Bag<T> &,const std::string &)' being compiled
        with
        [
            T=int
        ]
```

(Change reverted after capturing this — `main.cpp` is back to its committed state.)

TODO: Which file and line does the compiler blame? Which line did you actually
get wrong, and why are those two different? `numbers` is empty — the loop
inside `findByName` could never run even once. Why doesn't that save you?

A: search.h(88) was called. the line inside findByName. 
the line that was actually wrong was main.cpp(91)
it called findByName(numbers, "seven") with a bag<int>
they are different because the templates aren't checked until they are instantiated.
findByName is just a generic recipe until something calls it with T=int. 
The compiler then stamps out a real function for T=int, 
and that stamped-out code is what fails at line 88 because int has no .name member.
So the blame lands where the bad code got generated, not where I made the bad decision to call it that way.

Why an empty bag doesn't save you: This is a compile-time problem. The compiler has to generate valid code 
for the whole function body for T=int regardless of whether the loop ever actually runs. 
It doesn't know or care that numbers is empty. It's not executing anything, just compiling. 

## 3. Break it — the Swallowed Scream

Outer handler changed from:

```cpp
catch (const std::exception& e) {
    std::cout << "No such item. (" << e.what() << ")\n";
}
```

to:

```cpp
catch (...) { }
```

Rebuilt, then ran `inspect 99` followed by `inspect 3`:

```
> inspect 99
> inspect 3
  Iron key  (wt 0.1, val 0)
> quit
```

`inspect 99` printed **nothing at all** — no error message, no blank line
acknowledging the command even ran, just the next `>` prompt as if the command
had never been typed. `inspect 3` immediately afterward worked normally, as
though nothing had gone wrong the line before.

(Change reverted after capturing this — `main.cpp` is back to its committed
state, verified clean against `git diff`.)

TODO: Describe exactly what appears on screen, including what does *not*
appear (see the raw output above). Then: deleting the handler altogether
would make the program die outright. Explain why the empty handler is
*worse* than dying.

A: What appears: Nothing. just the next > prompt. The very next command (inspect 3) works normally, with zero sign anything went wrong.
Why this is worse than crashing: A crash is loud. catch (...) { } hides the failure completely: the exception fires, gets caught, and is thrown away with no trace. The program looks fine while quietly having failed. That's dangerous because bugs like this can sit unnoticed for a long time since there's no evidence anything happened.

## 4. On paper — checked vs. unchecked

TODO: A teammate opens a pull request changing `inspect`'s
`hero.inventory.at(n - 1)` to `hero.inventory[n - 1]`, with the note *"at() is
slower and we already validate the input upstream."* Write the review comment
you would leave. It has to name (a) a specific command a player could type
that breaks it, (b) what the program is now permitted to do instead of
throwing, and (c) why *"I tested it and it worked"* is not evidence that the
change is safe. Finish with one sentence naming the single situation where
their change would be the right call.

A: Changing .at(n - 1) to [n - 1] isn't safe. If a player types inspect 99 on a 5-item inventory,
.at() throws a catchable exception that we turn into "No such item." [] does no bounds checking at all. 
it's undefined behavior, meaning the program is now free to do anything: read garbage memory, silently return a bogus item, or crash, and which one happens can change by platform, build, or compiler flags. "I tested it and it worked" isn't proof of safety because undefined behavior isn't required to fail. 
it can happily "work" by luck every time you test it and still be broken.
The one case where [] would be the right call is a tight internal loop where the index is mathematically guaranteed 
in-range by the surrounding code (e.g., for (i = 0; i < size; i++)), not anywhere user input reaches the index.

## 5. On paper — why `std::exception`

TODO: Suppose `BagException` did *not* inherit from `std::exception`. It would
still compile: `throw` accepts any type at all, and `what()` would still be
there. Explain which line in `main.cpp` silently stops doing its job, what the
player sees instead when they type `inspect 99`, and why the compiler cannot
warn you about it. Then: what is the smallest change to `main.cpp` that makes
it work again, and what does that change cost you every time the game grows a
new kind of failure?

A: Which line stops working: catch (const std::exception& e) in main.cpp. A catch clause can only catch things that are the type it names. 
If BagException no longer derives from std::exception, this handler doesn't match it anymore.

What the player sees: Typing inspect 99 throws a BagException that nothing catches, so it propagates all the way up uncaught
the program calls std::terminate and the whole game crashes

Why the compiler can't warn you: This isn't a type error. catch (const std::exception&) is perfectly valid code. The compiler has no way of knowing you intended it to catch BagException too

Smallest fix: Add another catch clause, e.g. catch (const BagException& e) { ... }, that specifically matches BagException.

The cost: Every time the game grows a new exception type that also doesn't inherit from std::exception, 
you have to remember to bolt on yet another explicit catch clause for it. 
Inheriting from std::exception in the first place is what lets one single catch automatically cover every current and future exception type.
## AI acknowledgment

Claude Sonnet 5 (Anthropic, 2026) was used to build the project, run the demo
transcript, and perform the two "break it" exercises (making the specified
temporary code changes, capturing the real compiler/runtime output, and
reverting the changes afterward) in a properly configured build environment.
Claude also found and reported a pre-existing structural bug in `main.cpp`
(a missing brace and a misplaced `return 0;` that caused the program to exit
after one command) — the fix itself was written and committed by the student.
The analysis (Q2–Q5) is my own.
