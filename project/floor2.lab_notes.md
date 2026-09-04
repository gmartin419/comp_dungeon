[lab-notes.md](https://github.com/user-attachments/files/31850706/lab-notes.md)

# Floor 2 Lab Notes — Race the Sorts

## 1. Benchmark sweep output
## 
Full `benchmark sort` sweep (random input), pasted from the terminal:

```
-- Race the Sorts (random input) --
  N=     10  mergeSort=   0.002 ms  quicksort=    0.001 ms  std::sort=   0.001 ms
  N=    100  mergeSort=   0.017 ms  quicksort=    0.005 ms  std::sort=   0.005 ms
  N=   1000  mergeSort=   0.212 ms  quicksort=    0.077 ms  std::sort=   0.080 ms
  N=  10000  mergeSort=   2.690 ms  quicksort=    1.224 ms  std::sort=   1.077 ms
  N= 100000  mergeSort=  30.343 ms  quicksort=   14.986 ms  std::sort=  14.962 ms
```

## 2. Where does `std::sort` pull ahead?

TODO: Looking at the table above, at what N does `std::sort` clearly beat both
hand-rolled sorts? Quote the row and explain what you're reading off it.
Answer: at n=1,000 (0.080) is actually a hair behind the hand rolled quicksort. statistically they are tied.
the crossover happens at n=10,000. sort drops to 1.077 ms whereas quicksort climbs to 1.224 ms. now sort is clearly ahead and it stays that
way (barely) because at n=100,000 it is back to almost being tied 14.962 ms and 14.986 ms. so my honest answer is they are pretty neck and neck
but std::sort establish a lead starting at n=10,000. not a big lead, but its a lead nonetheless  

## 3. `--sorted` run

Full `benchmark sort --sorted` sweep, pasted from the terminal:

```
-- Race the Sorts (pre-sorted input) --
  N=     10  mergeSort=   0.002 ms  quicksort=    0.000 ms  std::sort=   0.000 ms
  N=    100  mergeSort=   0.017 ms  quicksort=    0.004 ms  std::sort=   0.001 ms
  N=   1000  mergeSort=   0.222 ms  quicksort=    0.053 ms  std::sort=   0.026 ms
  N=  10000  mergeSort=   1.831 ms  quicksort=    0.746 ms  std::sort=   0.213 ms
  N= 100000  mergeSort=  21.654 ms  quicksort=   10.024 ms  std::sort=   2.637 ms
```

TODO: Which sort got faster on pre-sorted input? Which got slower? Why?
none got slower but every sort got faster on pre-sorted input, but by very different amounts
std::sort: 14.962 ms --> 2.637 ms at N=100,000 -->(~5.7x faster) -> by far the biggest jump
quicksort (middle pivot): 14.986 ms → 10.024 ms ->(~1.5x faster)
mergesort: 30.343 ms --> 21.654 ms -------------->(~1.4x faster)
					    numbers provided by Claude

Why: mergesort and middle-pivot quicksort are still doing the same O(n log n) amount of work regardless of order — their speedup is a low-level effect, mostly better branch prediction (comparisons resolve in a consistent direction on sorted data, so the cpu's branch predictor stops mispredicting). std::sort is usually an introsort: quicksort based, but it falls back to insertion sort for small partitions, and insertion sort is near-linear (practically O(n)) on data that's already sorted or nearly so. That fallback is why its speedup is so much larger than the others'.
## 4. The Pivot Wraith (`--sorted --bad-pivot`)

| N     | quicksort (middle pivot, `--sorted`) | quicksort\* (first-element pivot, `--sorted --bad-pivot`) |
|-------|----------------------------------------|---------------------------------------------------------------|
| 2500  | 0.187 ms                               | 5.278 ms                                                      |
| 5000  | 0.358 ms                               | 22.807 ms                                                     |
| 10000 | 0.731 ms                               | 90.738 ms                                                     |

Raw output:

```
> benchmark sort --sorted 2500
  N=   2500  mergeSort=   0.786 ms  quicksort=    0.187 ms  std::sort=   0.075 ms
> benchmark sort --sorted --bad-pivot 2500
  N=   2500  mergeSort=   0.560 ms  quicksort*=   5.278 ms  std::sort=   0.066 ms
> benchmark sort --sorted 5000
  N=   5000  mergeSort=   0.913 ms  quicksort=    0.358 ms  std::sort=   0.099 ms
> benchmark sort --sorted --bad-pivot 5000
  N=   5000  mergeSort=   1.144 ms  quicksort*=  22.807 ms  std::sort=   0.100 ms
> benchmark sort --sorted 10000
  N=  10000  mergeSort=   2.082 ms  quicksort=    0.731 ms  std::sort=   0.217 ms
> benchmark sort --sorted --bad-pivot 10000
  N=  10000  mergeSort=   2.008 ms  quicksort*=  90.738 ms  std::sort=   0.217 ms
```

Commands to run for this table:

```
benchmark sort --sorted 2500
benchmark sort --sorted --bad-pivot 2500
benchmark sort --sorted 5000
benchmark sort --sorted --bad-pivot 5000
benchmark sort --sorted 10000
benchmark sort --sorted --bad-pivot 10000
```

TODO: Each time N doubles, roughly what happens to the middle-pivot time? To the
first-element-pivot time?
Name the growth rate of each. Then, in one sentence:
why is already-sorted input the worst possible case for a first-element pivot —
and why is that alarming?

Middle Pivot: time roughly doubles when N doubles. (O(n log n)) 

First element pivot: the time nearly quadruples here when N doubles. (O(n^2))

picking the first element as a picot on an already sorted array always picks the smallest remaining element,
remaining element, so every partition splits into nothing and everything else. its alarming because sorted data is common place in practice,
not some rare edge case so this worst case is exactly what a careless implementation will actually hit in production


## 5. Reflection — which sort would you ship?

TODO: One paragraph. If you could only ship one of the three sorts in production,
which would you ship? Defend your choice in terms of worst case, average case,
and what the data is likely to look like.

I would choose std::sort and here is why. std::sort has the most consistent readings and if you look up on the benchmarks, you can see how it surpasses the other methods in the general sense. yes the other methods do not trail behind that far but in the market, every milli-second matters. yes user input can through off your algorithm and that would probably be the worst case but the average will look more like random and the lists will be unorderly. this leads
to my last support, the data is most likely going to be jumbled and std::sort likes that stuff so naturally its going to excel in that area. In my mind,
if the goods outweigh the bad, that's a justifiable compromise for me.


## Claude citation: AI acknowledgment: Claude Sonnet 5 (Anthropic, 2026) was used to compile and run the benchmark sort commands in a properly configured build environment, and to format the raw terminal output into this file's structure. The analysis (Q2–Q4) and reflection (Q5) are my own.
