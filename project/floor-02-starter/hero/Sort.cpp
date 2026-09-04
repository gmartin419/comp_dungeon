// COMP 2450 — Floor 2 starter
// hero/Sort.cpp — YOU implement this file.
//
// Three functions to write. Read Sort.h for their contracts.
//
// The big idea this week: sorting is not one algorithm, it is a family
// of tradeoffs. Merge sort is predictable but copies. Quicksort is fast
// on average but betrays you on bad inputs. std::sort is what you
// actually ship. You will write the first two, race all three in
// `benchmark sort`, and argue — in a commit message — which one the
// game should call.
//
// Tips for the Pivot Wraith:
//   * If you pick the FIRST element as your quicksort pivot, a sorted
//     input becomes O(n^2). The `benchmark sort --bad-pivot --sorted`
//     harness exists to show you exactly that.
//   * The middle element is the cheapest defense. Good enough for this
//     week. Real production code (std::sort) does median-of-three and
//     switches algorithms on bad recursion depth.
//
// Submit when:  `sort inventory by weight` produces ascending weight,
//               `sort inventory by name desc` produces reverse alphabetical,
//               and `benchmark sort` gives three timing columns on every row.

#include "Sort.h"
#include <algorithm>  // you will want std::sort in sortInventory
#include <sstream>


namespace dungeon {

    namespace {
        void merge(std::vector<Item>& v, std::size_t lo, std::size_t mid, std::size_t hi, const Comparator& cmp) {
            if (hi - lo < 2) {
                return;
            }

            std::vector<Item> scratch;
            scratch.reserve(hi - lo);
            // allocating capactity for high - lo items
            // up front

			// Two cursors, one for each half of the range. 
            // The left half is [lo, mid) and the right half is [mid, hi). 
            // The merge loop will pick the smaller of the two front items and 
            // append it to the scratch vector. When one half is exhausted, we 
            // append the rest of the other half.
            std::size_t i = lo;
            std::size_t j = mid;
            

            // merge loop
            // while both halves still have items,
            // pick the smaller front of queue and append
			while (i < mid && j < hi) {
				if (!cmp(v[i], v[j])) {
					scratch.push_back(v[i++]);
				}
				else {
					scratch.push_back(v[j++]);
				}
			}
            // one half is drained but the other still has
            // items
			while (i < mid) {
				scratch.push_back(v[i++]);
			}
			while (j < hi) {
				scratch.push_back(v[j++]);
			}

            // copy the merged result back into v
			// at positions [lo, hi)
			for (std::size_t k = 0; k < scratch.size(); k++) {
                v[lo + k] = std::move(scratch[k]);
			}
        }

        void mergeSortImpl(std::vector<Item>& v, std::size_t lo, std::size_t hi, const Comparator& cmp) {
			// base case: if the range has fewer than 2 items, it's already sorted
            if (hi - lo < 2) {
                return;
            }
			// Recursively sort the left and right halves, then merge them.
            std::size_t mid = lo + (hi - lo) / 2;

            mergeSortImpl(v, lo, mid, cmp);
            mergeSortImpl(v, mid, hi, cmp);
            merge(v, lo, mid, hi, cmp);
        }

        // create a partition
        std::size_t partition(std::vector<Item>& v,
            std::size_t low, std::size_t high,
            const Comparator& cmp) {
            // high is our last index (inclusive)
            // 1. pick the pivot
            std::size_t mid = low + (high - low) / 2;
            std::swap(v[mid], v[high]);
            const Item pivot = v[high];
            // compute middle index
            // std::swap exchanges two items
            // w/o copying the whole struct
            // in lomuto, assumes the pivot lives at the high position
            // so by moivng our pivot there, we can follow classic lomuto

            //lomuto scan
            std::size_t store = low; //boundary
            //[low, store) --> strictly less than teh pivot
            //[store, high) --> >= pivot
            for (std::size_t i = low; i < high; ++i) {
                if (cmp(v[i], pivot)) {
                    std::swap(v[store], v[i]);
                    ++store;
                }
            }
            std::swap(v[store], v[high]);
            return store;
        }

        void quicksortImpl(std::vector<Item>& v,
            std::size_t low, std::size_t high,
            const Comparator& cmp) {
            // Base case
            if (low >= high) return;
            std::size_t p = partition(v, low, high, cmp);
            if (p > low) quicksortImpl(v, low, p - 1, cmp);
            quicksortImpl(v, p + 1, high, cmp);
        }
        Comparator makeComparator(const std::string& key,
            bool descending) {
            Comparator cmp;
            if (key == "name") {
                cmp = [](const Item& a, const Item& b) {
                    return a.name < b.name;
                    };
            }
            else if (key == "weight") {
                cmp = [](const Item& a, const Item& b) {
                    return a.weight < b.weight;
                    };
            }
            else if (key == "value") {
                cmp = [](const Item& a, const Item& b) {
                    return a.value < b.value;
                    };
            }
            else return nullptr;
            if (descending) {
                Comparator asc = cmp;
                cmp = [asc](const Item& a, const Item& b) {
                    return asc(b, a);
                    };
            }
            return cmp;
        }

    }

// ---- 1. Merge sort ------------------------------------------------------

void mergeSort(std::vector<Item>& inventory, const Comparator& cmp) {
	mergeSortImpl(inventory, 0, inventory.size(), cmp);    
}
    



// ---- 2. Quicksort -------------------------------------------------------

void quicksort(std::vector<Item>& inventory, const Comparator& cmp) {
    if (inventory.size() < 2) return;
    quicksortImpl(inventory, 0, inventory.size() - 1, cmp);
    
}

// ---- 3. sortInventory (the seam) ----------------------------------------

bool sortInventory(Hero& hero, const std::string& criterion) {
    
    // treating this like a standard input
    std::istringstream in(criterion);
    std::string key;
    std::string dir;
    // reading a couple of tokens
    in >> key >> dir;

    bool descending = (dir == "desc");
    Comparator cmp = makeComparator(key, descending);
    if (!cmp) return false;
    std::sort(hero.inventory.begin(), hero.inventory.end(), cmp);
    return true;
}

}  // namespace dungeon


// Notes:
// comparator: a function (in c++, lambda)
// does a come strictly before b
// cmp(a,b) == true if strictly < b
// cmp(a,b) == false if a >= b
// strict weak ordering
// python: sorted(x, key = fn)
