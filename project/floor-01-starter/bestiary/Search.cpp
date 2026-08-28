// COMP 2450 — Floor 1 starter
// bestiary/Search.cpp — YOU implement this file.
//
// Four functions to write. Read Search.h for their contracts.
//
// The big idea this week: the same question ("is X in the list?") has
// three implementations with three VERY different Big-O costs. You will
// write all three, race them in `benchmark`, and then argue — in a commit
// message — which one the rest of the game should use. The code is easy.
// The *thinking* is the point.
//
// Tips for the Unsorted Lich:
//   * binarySearch (and binarySearchRecursive) only work if the bestiary
//     is sorted by name.
//   * main.cpp already calls sortBestiary() right after loading,
//     so you can assume the precondition holds when these run.
//   * If you ever doubt, scan the vector and assert it is sorted.
//
// Submit when:  `search Goblin` returns the goblin's stats and `search Ghost`
//               reports "no such creature." Then run `benchmark` and capture
//               the output for your lab-notes.md.

#include "Search.h"
#include <cassert>
#include <algorithm>
namespace dungeon {

const Monster* linearSearch(const std::vector<Monster>& bestiary,
                            const std::string&         name) {
    // TODO Floor 1 (Mon): walk every entry; return its address when name matches.
    //                     If you reach the end without a match, return nullptr.
    //
    // Think before you type:
    //   - You return `const Monster*` (a pointer into the vector), NOT
    //     `Monster` (a copy). Why a pointer? What would you even return
    //     from a "copy" version when the name is not found?
    //   - In the range-for loop, `for (auto m : bestiary)` makes a COPY
    //     of each monster each iteration. `for (const auto& m : bestiary)`
    //     does not. Which do you want — and why does the difference matter
    //     more for a `Monster` than for an `int`?
    //   - How do you take the address of the element you're looking at?
    //     (Two common idioms. Pick whichever makes your loop read cleanly.)

	for (const auto& m : bestiary) {
		if (m.name == name) {
			return &m;
		}
	}

 
    return nullptr;
}

const Monster* binarySearch(const std::vector<Monster>& bestiary,
                            const std::string&         name) {

    assert(std::is_sorted(bestiary.begin(), bestiary.end(), 
        [](const Monster& a, const Monster& b)
        {return a.name < b.name;}));
    // closed range --> [low, high]
    // half opened range --> [low, high)
    // if we go half-open:
    //  1) it matches pythons range(low, high)
    //  2) using std::size_t for indices
    //      - size_t is unsigned
    //      - we can end up doinug high - 1 when
    //      - high is already at 0
    // ====================================================================
    // ====================================================================
    // low range
    std::size_t low = 0;
    // High rangee3
    std::size_t high = bestiary.size();
    while (low < high) {
        std::size_t mid = low + (high - low) / 2;
        const std::string& here = bestiary[mid].name;
        if (here == name) {
            return &bestiary[mid];
        }
        else if (here < name) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }
    return nullptr;
}

//================================================================================
// Recursive binary search helper: searches the sorted bestiary by repeatedly
// checking the middle element. If the middle name matches, return a pointer
// to the monster. If the target comes after the middle name, recursively search
// the right half; otherwise, search the left half. The base case (low >= high)
// stops the recursion when there are no elements left to search.
//================================================================================

namespace {
    const Monster* binSearchRec(
        const std::vector<Monster>& bestiary,
        const std::string& name,
        std::size_t low,
        std::size_t high
    ) {
        //base case first
        if (low >= high) return nullptr;
        //recursive case
        std::size_t mid = low + (high - low) / 2;
        const std::string& here = bestiary[mid].name;
        if (here == name) return &bestiary[mid];
        else if (here < name) {
            return binSearchRec(bestiary, name, mid + 1, high);

        }
        else {
            return binSearchRec(bestiary, name, low, mid);
        }
    }
}

const Monster* binarySearchRecursive(const std::vector<Monster>& bestiary,
                                     const std::string&         name) {
    // TODO Floor 1 (Fri): same contract as binarySearch, but recursive.
    //   Recommended pattern: write a `static` helper in this file with extra
    //   (low, high) parameters, and have this public function call it with
    //   the initial range. Same precondition: bestiary must be sorted.
    //
    // Think before you type:
    //   - Every recursion needs a BASE CASE and a RECURSIVE CASE. What is
    //     the smallest range where you already know the answer without
    //     looking further? That is your base case.
    //   - Convince yourself, for each recursive call, that the new range
    //     is a STRICT SUBSET of the old one. If it isn't, you will recurse
    //     until the stack blows up. (Try it at N=100,000 if curious.)
    //   - Why `static` for the helper? It has nothing to do with OOP here.
    //     Look up "internal linkage" — it keeps the helper private to this
    //     .cpp, so two files can have `helper(...)` without a link error.
    //   - After it works: run `benchmark`. Does the recursive version cost
    //     more per call than the iterative one? A little? A lot? Why might
    //     that be? Write the answer in lab-notes.md.

    return binSearchRec(bestiary, name, 0, bestiary.size());

    return nullptr;
}

const Monster* findMonster(const std::vector<Monster>& bestiary,
                           const std::string&         name) {
    // TODO Floor 1: pick ONE of the three searches above and delegate.
    //
    // Think before you type:
    //   - At the real bestiary's size (15 monsters), does it matter which
    //     you pick? Run benchmark at N=10 and look at the microseconds.
    //     Answer: In my opionon, a list this small does not necasiarly matter. The microseconds for each is
    //              N=10  query=last    linear=0.616 us  binary=1.244 us  recursive=0.201 us. we can see that binary
    //              and lineararity are pretty close but recursive was faster. so if you really want, you could go with the recursive
    //              option.
    //   - At N=100,000, does it matter? By how much?
    //     Answer: my computer is super slow so Im not sure if my data is worth analyzing but but binary and linear are super
    //             slow and my recursive is very fast compared to the two. 
    //             N= 100000  query=last    linear=  2530.383 us  binary=2974.171 us  recursive=   0.857 us
    //             N = 100000  query = absent  linear = 1539.535 us  binary = 3039.543 us  recursive = 0.680 us
    //   - This is a JUDGMENT, not a fact. Whatever you pick, write WHY in
    //     your commit message. That reasoning is the graded artifact.
    //     Answer: I would choose the binary Recursive becuase that is the fastest sorting system that ran the best on my machine
    //             I also so the most consistant data in this category so In my opion, that is the best option
    // 
    return binarySearch(bestiary, name);
}

}
