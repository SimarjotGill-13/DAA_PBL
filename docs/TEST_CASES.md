# Test cases

Run the automated tests from the project root:

```bash
make test            # builds, then runs ./order_optimizer --test (exit code 0 means all passed)
```

The same tests are available inside the program: menu 15. The three demonstration scenarios are menu 16 or `./order_optimizer --scenarios`.

The dataset-file tests (T105 to T107) open files in `data/`, so run them from the project root. If the files cannot be found they print `[SKIP]` instead of passing silently.

## 1. What is covered

| Category | Test IDs | What is checked |
|---|---|---|
| Normal operation | T01, T07, T09 | valid order, lowest-cost choice, nearest warehouse with stock |
| Invalid inputs | T02, T03, T13 to T16, T25 to T32, T36, T69, T88, T90, T94 to T99, T103 | bad PIN, product, quantity; non-numeric, trailing-character, out-of-range and overflow input; bad dataset files; invalid algorithm arguments |
| Empty and boundary cases | T17 to T24, T33, T56, T65, T74, T76, T86 | INT_MAX quantity, quantity equal to stock, quantity just above stock, empty inventory, empty order log, empty schedule, empty and single-element sorts, all-negative arrays, empty strings, empty pattern |
| Insufficient inventory | T04, T19, T20, T06b | not enough anywhere, enough in total but not in one warehouse, all stock zero, the last unit taken |
| Damaged stock | T05, T22 to T24 | damaged units excluded from sellable, damage limited to sellable units, reserved units protected |
| Unserviceable PIN codes | T02, T15 | unknown, zero and negative PIN codes |
| Unreachable warehouses and destinations | T11, T43, T79, T106 | isolated customer area, unreachable TSP stop |
| Equal-cost warehouse selection | T10 | equal cost, shorter distance wins |
| Overselling | T06a, T06b | first come first served, no double reservation |
| Negative weights | T45 to T53, T98 to T100, T107 | Dijkstra wrong, Bellman-Ford correct, automatic selection, negative cycles rejected, one-way arcs |
| Large-data and stress | T54, T101 to T104 | 20,000-element sorts, 3,000-node graph, order log overflow |
| Algorithm-specific | T33 to T41, T42 to T44, T55 to T63, T64 to T90 | see section 2 |
| Dataset loading | T91 to T100, T105 to T107 | valid file, rollback on error, built-in file equals built-in data |

## 2. Algorithm-specific test cases

| Algorithm | Input | Expected behaviour | Edge cases tested |
|---|---|---|---|
| Binary / Linear Search | sorted PINs | same answer from both | first, last, missing key |
| Merge / Quick Sort | arrays | same sorted output; Merge is stable | empty, one element, duplicates, reversed, INT_MIN and INT_MAX, 20,000 random |
| Dijkstra / Bellman-Ford / Floyd-Warshall | sample road graph | all three agree from every source | unreachable node, zero-length road, one-way road |
| Bellman-Ford | A->B 4, A->C 5, C->B -3, B->D 2 | B = 2, D = 4 while Dijkstra gives B = 4 | negative cycle added |
| Heap vs array Dijkstra | random graphs of 300 and 3000 nodes | identical distances | too-small graph rejected |
| Activity Selection / Interval Partitioning | sample deliveries | 3 selected, 3 agents | empty, identical intervals, touching intervals, too many deliveries |
| Knapsack | 4 parcels, capacity 50 | DP, backtracking, FIFO, LIFO, Least-Cost all 235; fractional 255 | DP equals an independent brute force; invalid size |
| Maximum Subarray | 16 daily changes | 43 over days 7 to 10 | all negative, empty |
| Powering | 2^10, 7^128 mod 13 | 1024; matches repeated multiplication | exponent 0 |
| Celebrity | 4-person relation | person 2 | no celebrity |
| Strassen | n = 2, 4, 8 | equals the standard product; 49 vs 64 and 343 vs 512 multiplications | size not a power of two |
| Huffman | sample text | fewer bits than 8-bit coding | empty text |
| Prim / Kruskal | sample roads | both 25 | directed or negative graph refused |
| LCS | SmartPhone, SmartWatch | length 6 | empty string |
| Matrix Chain | 30, 35, 15, 5, 10, 20, 25 | 15125 | one matrix, invalid dimension |
| Optimal BST | 34, 8, 50 | 142 | none |
| TSP | W1, CustX, CustY, HubA, HubC | brute force, Held-Karp and Branch and Bound agree | unreachable stop |
| N-Queens | n = 4, 6, 8 | 2, 4, 92 | n = 0 |
| Sum of Subsets | 7 weights, target 35 | equals an independent brute-force count | none |
| Max Flow | supply/dispatch/demand network | 14 | source equals sink |
| Bipartite Matching | 4 orders, 3 agents | 3 | none |
| String matching (4) | text with three matches | all four report 3 | overlapping matches, no match, pattern longer than text, empty pattern |
| CRT | x = 2 mod 3, 3 mod 5, 2 mod 7 | 23 mod 105 | moduli not coprime, modulus below 2 |
| Master Theorem | several recurrences | correct case and exponent | invalid a or b |

## 3. Demonstration scenarios

1. **Normal order fulfilment.** Four orders processed in sequence; each is routed to the cheapest eligible warehouse.
2. **Capacity or inventory constraint shock.** The same order is run (A) after W1 reports damaged stock, (B) after a road closure, and (C) after all stock is damaged. The chosen warehouse and cost change, or the order is rejected with a reason.
3. **Large-data stress.** Sorting 20,000 integers (Merge Sort vs Quick Sort, random and sorted input), searching 1,000,000 items (Linear vs Binary), and shortest paths on a 5,000-node, 25,000-road network (array Dijkstra vs heap Dijkstra). In every comparison both strategies get the same input and their results are checked for equality.

## 4. Actual results

These are copied from real runs, not written by hand.

- **Date:** 2026-11-12
- **Environment:** Linux, gcc 13.3.0, `-std=c17 -Wall -Wextra -Wpedantic -O2`, built with `make` from the project root
- **Build:** zero warnings and zero errors
- **Also run:** the same tests built with `-fsanitize=address,undefined`; no sanitizer messages.

### 4.1 `./order_optimizer --test`

```text
Automated test cases (order output is suppressed)

A. Order workflow
  [PASS] T01 valid order: W1 chosen, cost 11 (W3 would cost 16), routed by Dijkstra
  [PASS] T01b reserved = 2, total unchanged = 10, sellable = 8
  [PASS] T02 unserviceable PIN rejected, stock untouched
  [PASS] T03 invalid product ID rejected
  [PASS] T04 insufficient sellable stock rejected, stock untouched
  [PASS] T05 damaged units excluded: W1 sellable 4 < 5, so W2 chosen (cost 13)
  [PASS] T06a first customer gets the last unit (W1)
  [PASS] T06b second customer rejected, no double reservation
  [PASS] T07 nearest warehouse W3 (distance 8) has stock and wins, cost 12
  [PASS] T08 nearest W3 has only 5 < 8, alternative W1 chosen, cost 17
  [PASS] T09 different costs W1 11 < W2 13 < W3 16 -> W1
  [PASS] T10 equal cost (11 vs 11): shorter distance wins -> W1
  [PASS] T11 unreachable destination rejected, stock untouched
  [PASS] T12 redistribution gives 2 recommendations and does not change stock

B. Invalid inputs, empty and boundary cases
  [PASS] T13 quantity 0 rejected
  [PASS] T14 negative quantity rejected
  [PASS] T15 PIN 0 and negative PIN rejected
  [PASS] T16 product ID 0 rejected
  [PASS] T17 huge quantity INT_MAX handled without overflow
  [PASS] T18 boundary: quantity equal to the largest sellable stock (10) succeeds
  [PASS] T19 boundary: quantity 11 when total is 15 but no warehouse holds 11 -> no single warehouse
  [PASS] T20 empty inventory everywhere -> insufficient
  [PASS] T21 empty order log at start
No inventory record for product 103 at warehouse 9.
Damaged quantity must be at least 1.
Rejected: only 4 sellable unit(s) can be marked damaged (reserved units are protected).
  [PASS] T22 damage: unknown record, quantity 0 and quantity too large are rejected
Recorded 1 damaged unit(s). Sellable is now 3.
  [PASS] T23 damage: 1 unit accepted, sellable 4 -> 3
Rejected: only 0 sellable unit(s) can be marked damaged (reserved units are protected).
  [PASS] T24 reserved units cannot be marked damaged

C. Input parsing (validation helper)
  [PASS] T25 '42' accepted
  [PASS] T26 ' 7 ' with spaces accepted
  [PASS] T27 '42abc' rejected (trailing characters)
  [PASS] T28 empty and blank text rejected
  [PASS] T29 '3.5' rejected
  [PASS] T30 out-of-range 101 and -1 rejected
  [PASS] T31 overflow 99999999999999999999 rejected
  [PASS] T32 decimal parsing accepts 2.5 and rejects '2.5x'

D. Sorting and searching
  [PASS] T33 Merge Sort handles empty and single element
  [PASS] T34 Merge Sort with duplicates
  [PASS] T35 Quick Sort and Merge Sort agree on reversed input
  [PASS] T36 invalid arguments rejected (n < 0, NULL comparator)
  [PASS] T37 comparison is overflow-safe (INT_MIN vs INT_MAX)
  [PASS] T38 Merge Sort is stable (equal keys keep their order)
  [PASS] T39 binary search: first, last and missing PIN
  [PASS] T40 linear and binary search agree for every PIN
  [PASS] T41 product binary search matches linear search

E. Routing: Dijkstra, Bellman-Ford, Floyd-Warshall, negative weights
  [PASS] T42 all three algorithms agree from every source node
  [PASS] T43 CustX distances: W1 6, W2 11, W3 12, CustZ unreachable
  [PASS] T44 non-negative roads -> Dijkstra is selected
  [PASS] T45 negative arc: Dijkstra is WRONG (B = 4), Bellman-Ford correct (B = 2, D = 4)
  [PASS] T46 negative arc -> Bellman-Ford is selected automatically and is correct
  [PASS] T47 Floyd-Warshall also handles the negative arc
  [PASS] T48 negative cycle rejected when adding an arc, graph unchanged
  [PASS] T49 negative cycle detected by Bellman-Ford, Floyd-Warshall and the cycle check
  [PASS] T50 negative TWO-WAY road rejected (code 4); bad node 1; bad weight 2
  [PASS] T51 a road of length 0 is representable and routes correctly
  [PASS] T52 order workflow switches to Bellman-Ford with a rebate road and picks W3 (cost 10)
  [PASS] T53 one-way roads: order still succeeds and uses the reversed graph
  [PASS] T54 heap Dijkstra equals array Dijkstra on a random 300-node graph

F. Scheduling and knapsack
  [PASS] T55 Activity Selection on the sample picks 3; Interval Partitioning needs 3 agents
  [PASS] T56 empty schedule: 0 selected, 0 agents
  [PASS] T57 three identical intervals: 1 selected, 3 agents
  [PASS] T58 touching intervals are compatible: 3 selected, 1 agent
  [PASS] T59 too many deliveries rejected
  [PASS] T60 0/1 DP, backtracking and all three branch-and-bound modes give 235
  [PASS] T61 fractional gives 255 (higher, but invalid for indivisible parcels)
  [PASS] T62 DP equals an independent brute-force over all 16 subsets
  [PASS] T63 knapsack rejects invalid input (n = 0, capacity too large)

G. Algorithm cores (Divide and Conquer, Greedy, DP, Backtracking, Flow, Strings, Numbers)
  [PASS] T64 Maximum Subarray = 43 over days 7..10
  [PASS] T65 Maximum Subarray with all negatives = -2; n = 0 rejected
  [PASS] T66 powering: 2^10 = 1024, 7^128 mod 13 matches repeated multiplication
  [PASS] T67 Celebrity: person 2 found; no celebrity in a mutual-acquaintance group
  [PASS] T68 Strassen equals standard multiplication for n = 2, 4, 8 (49 vs 64 and 343 vs 512 multiplications)
  [PASS] T69 Strassen rejects a size that is not a power of two
  [PASS] T70 Huffman uses fewer bits than 8-bit coding; empty text rejected
  [PASS] T71 Prim and Kruskal give the same minimum spanning forest weight (25)
  [PASS] T72 MST refuses a directed or negative graph
  [PASS] T73 LCS(SmartPhone, SmartWatch) = 6
  [PASS] T74 LCS with an empty string = 0
  [PASS] T75 Matrix Chain minimum = 15125 multiplications
  [PASS] T76 Matrix Chain with one matrix = 0; invalid dimension rejected
  [PASS] T77 Optimal BST cost = 142
  [PASS] T78 TSP: brute force, Held-Karp and Branch and Bound all agree
  [PASS] T79 TSP reports an unreachable stop
  [PASS] T80 N-Queens: 4 -> 2, 6 -> 4, 8 -> 92 solutions; n = 0 rejected
  [PASS] T81 Sum of Subsets count equals an independent brute-force count
  [PASS] T82 Max Flow = 14; source equals sink rejected
  [PASS] T83 Bipartite matching size = 3
  [PASS] T84 Naive, Rabin-Karp, Automaton and KMP all find 3 matches
  [PASS] T85 overlapping matches: 'aaaa' in 'aaaaaa' = 3 for all four
  [PASS] T86 no match = 0; pattern longer than text = 0; empty pattern rejected
  [PASS] T87 CRT: x = 23 (mod 105) and it satisfies every congruence
  [PASS] T88 CRT rejects moduli that are not coprime, and a modulus below 2

H. Recurrences
  [PASS] T89 Master Theorem cases: merge sort case 2, Strassen case 1 (2.807), T(n)=T(n/2)+n case 3
  [PASS] T90 Master Theorem rejects invalid recurrences (b = 1, a = 0)

I. Dataset loading
  [PASS] T91 valid dataset file loads
  [PASS] T92 orders work on the loaded data (cost 5 + fee 1 = 6)
  [PASS] T93 loaded stock numbers kept (total 9, damaged 1, reserved 2 + 3)
  [PASS] T94 unknown keyword rejected and the current data is kept
  [PASS] T95 missing file reported
  [PASS] T96 duplicate product id rejected
  [PASS] T97 damaged + reserved > total rejected
  [PASS] T98 a negative cycle in the file is rejected
  [PASS] T99 a negative two-way road in the file is rejected
  [PASS] T100 a one-way negative arc loads and the order uses Bellman-Ford
  [PASS] T105 data/sample_dataset.txt loads and equals the built-in dataset
  [PASS] T106 data/extended_dataset.txt: order succeeds (W1, cost 9); isolated area A6 is unreachable
  [PASS] T107 data/negative_roads_dataset.txt: Bellman-Ford chosen, W3 cost 10

J. Large-data and stress cases
  [PASS] T101 sorting 20000 random integers: Merge Sort and Quick Sort agree and are sorted
  [PASS] T102 3000-node, 15000-road graph: array and heap Dijkstra give identical distances
  [PASS] T103 invalid graph sizes rejected by the generator
  [PASS] T104 order log never overflows (capacity respected)

Result: 109 passed, 0 failed
```

### 4.2 `./order_optimizer --scenarios`

Timings vary from run to run; the numbers below are from one run.

```text
=== Scenario 1: normal order fulfilment ===
ID   PIN      Product Qty  WH     Cost  Result
1    100001   101     2    W1     11    SUCCESS
2    100002   102     1    W2     13    SUCCESS
3    100001   103     3    W1     11    SUCCESS
4    100002   101     8    W1     17    SUCCESS
Every order went to the cheapest eligible warehouse; stock was reserved only after allocation.

=== Scenario 2: inventory and road constraint shocks (same order before and after) ===
Baseline: PIN 100001, product 101, qty 2 -> W1, cost 11, distance 6
Shock A: W1 inspection finds 9 of its 10 Phones damaged.
Recorded 9 damaged unit(s). Sellable is now 1.
  -> W3, cost 16 (W1 now has 1 sellable, so the order moves to the next cheapest eligible warehouse)
Shock B: the road HubA-CustX is closed.
  -> W3, cost 16, distance 12 (W1 is now farther away, so the cheapest eligible warehouse changes)
Shock C: every warehouse reports all stock damaged -> Insufficient sellable stock across all warehouses

=== Scenario 3: large-data stress ===
[1] Sorting 20000 integers
  Random:
    Merge Sort  comparisons     260911  time     3.27 ms  sorted correctly: yes
    Quick Sort  comparisons     323095  time     2.10 ms  sorted correctly: yes
  Sorted (Quick Sort worst case):
    Merge Sort  comparisons     139216  time     1.39 ms  sorted correctly: yes
    Quick Sort  comparisons  199990000  time   753.78 ms  sorted correctly: yes

[2] Searching 1000000 sorted items for the last one: Linear 1000000 comparisons, Binary 20 comparisons.

[3] Shortest paths on a random road network: 5000 nodes, 25000 roads (same graph for both)
  Dijkstra, array scan  O(V^2 + E)      :    18.87 ms
  Dijkstra, binary heap O((V + E) log V):     0.75 ms
  Distances identical: yes
  Small road networks: the array version is simplest. Large sparse ones: the heap version wins.
```

### 4.3 Menu and input checks run by hand

Every one of the 16 main menu options and all sub-menu entries was run once with scripted input. The input-validation checks fed the program non-numeric text, trailing characters, an empty line, an overflowing number, a negative number, an out-of-range number, a 300-character line, and end of input in the middle of a prompt. In every case the program printed a message or stopped cleanly, and no sanitizer message appeared.
