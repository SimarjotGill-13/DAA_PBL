# Syllabus coverage

**Status legend:** *Implemented* = working algorithm in the source, exercised by the automated tests. *Demonstration* = implemented as a standalone demonstration. *Theory* = explained in text only, no algorithm. **Type legend:** *Core* = used by the order workflow or a main menu module. *Comparison* = implemented so strategies can be compared on the same input. *Demonstration* = standalone demo. *Theory* = explanation only.

Menu numbers refer to the main menu (`./order_optimizer`). "M13" means menu 13 (Supplementary algorithm demos).

## Unit I. Algorithm Foundations and Analysis

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| I.1 | Characteristics of algorithms | `complexity.c` | `showTheory()` | Theory (menu 14 > 4) | Theory |
| I.2 | Algorithm design techniques | `complexity.c` | `showTheory()` | Theory | Theory |
| I.3 | Algorithm analysis; best, average, worst case | `complexity.c` | `showTheory()`, `demoComplexity()` | Theory plus measured comparison | Theory / Comparison |
| I.4 | Growth of algorithms | `complexity.c` | `showTheory()` | Theory | Theory |
| I.5 | Asymptotic notations | `complexity.c` | `showTheory()` | Theory | Theory |
| I.6 | Recurrence relations | `complexity.c` | `demoRecurrences()` | Demonstration | Demonstration |
| I.7 | Iteration method | `complexity.c` | `demoRecurrences()` | Demonstration (evaluated numerically) | Demonstration |
| I.8 | Substitution method | `complexity.c` | `demoRecurrences()` | Demonstration (worked proof printed) | Demonstration |
| I.9 | Recursion tree method | `complexity.c` | `demoRecurrences()` | Demonstration (per-level costs printed) | Demonstration |
| I.10 | Master Theorem | `complexity.c` | `masterCase()`, `demoMasterTheorem()` | Implemented (classifier) | Demonstration |

## Unit II. Divide and Conquer

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| II.1 | Binary Search | `inventory.c` | `findPinNode()`, `findProductBinary()`, `invRange()` | Implemented | Core |
| II.2 | Merge Sort | `inventory.c` | `mergeSort()` | Implemented | Core |
| II.3 | Quick Sort | `inventory.c` | `quickSort()` | Implemented | Comparison |
| II.4 | Strassen's Matrix Multiplication | `demos_greedy_dc.c` | `strassenMultiply()` | Implemented (n = power of two, up to 8) | Demonstration |
| II.5 | Maximum Subarray | `demos_greedy_dc.c` | `maxSubarray()` | Implemented | Demonstration |
| II.6 | Powering a Number | `demos_greedy_dc.c` | `powerDC()`, `modPowerDC()` | Implemented | Demonstration |
| II.7 | Celebrity Problem | `demos_greedy_dc.c` | `celebrityFind()` | Implemented | Demonstration |

## Unit III. Greedy Algorithms

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| III.8 | Fractional Knapsack | `schedule.c` | `knapFractional()` | Implemented | Comparison |
| III.9 | Activity Selection | `schedule.c` | `activitySelection()` | Implemented | Core |
| III.10 | Huffman Encoding | `demos_greedy_dc.c` | `huffmanBits()` | Implemented | Demonstration |
| III.11 | Interval Partitioning | `schedule.c` | `intervalPartition()` | Implemented | Core |
| III.12 | Dijkstra | `graph.c` | `dijkstra()`, `shortestPaths()`, `distancesToNode()` | Implemented | Core (central) |
| III.13 | Prim's MST | `demos_greedy_dc.c` | `mstPrim()` | Implemented | Comparison |
| III.14 | Kruskal's MST | `demos_greedy_dc.c` | `mstKruskal()` | Implemented | Comparison |

## Unit IV. Dynamic Programming and Shortest Paths

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| IV.15 | 0/1 Knapsack (DP) | `schedule.c` | `knapDP()` | Implemented | Core (vehicle loading) |
| IV.16 | Matrix Chain Multiplication | `demos_dp_bt.c` | `matrixChainMin()` | Implemented | Demonstration |
| IV.17 | Longest Common Subsequence | `demos_dp_bt.c` | `lcsLength()` | Implemented | Demonstration |
| IV.18 | Optimal Binary Search Tree | `demos_dp_bt.c` | `obstMin()` | Implemented | Demonstration |
| IV.19 | Bellman-Ford | `graph.c` | `bellmanFord()` | Implemented | Core (selected automatically when a road is negative) |
| IV.20 | Travelling Salesperson (small-instance DP) | `demos_dp_bt.c` | `tspSolveAll()` (internal `heldKarp()`) | Implemented | Demonstration |
| IV.21 | Floyd-Warshall | `graph.c` | `floydWarshall()` | Implemented | Comparison (also feeds TSP) |
| IV (extra) | Differences between Divide and Conquer, Greedy and DP | `complexity.c` | `showTheory()` | Theory | Theory |

## Unit V. Backtracking and Branch and Bound

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| V.22 | N-Queens | `demos_dp_bt.c` | `nQueensCount()` | Implemented | Demonstration |
| V.23 | Sum of Subsets | `demos_dp_bt.c` | `subsetSumCount()` | Implemented | Demonstration |
| V.24 | 0/1 Knapsack by Backtracking | `schedule.c` | `knapBacktrack()` | Implemented | Comparison |
| V.25 | FIFO Branch and Bound | `schedule.c` | `knapBB()` with mode 0 | Implemented | Comparison |
| V.26 | LIFO Branch and Bound | `schedule.c` | `knapBB()` with mode 1 | Implemented | Comparison |
| V.27 | Least-Cost Branch and Bound | `schedule.c`, `demos_dp_bt.c` | `knapBB()` with mode 2; `tspSolveAll()` | Implemented | Comparison |
| V.28 | 0/1 Knapsack by Branch and Bound | `schedule.c` | `knapBB()` | Implemented | Comparison |
| V.29 | TSP by Branch and Bound | `demos_dp_bt.c` | `tspSolveAll()` (internal `tspBB()`) | Implemented | Demonstration |

## Unit VI. Flow Networks, String Matching and Complexity

| # | Topic | Source file | Function | Status | Type |
|---|---|---|---|---|---|
| VI.30 | Ford-Fulkerson Maximum Flow | `demos_flow_string.c` | `maxFlow()` | Implemented | Demonstration |
| VI.31 | Maximum Bipartite Matching | `demos_flow_string.c` | `bipartiteMatching()` | Implemented | Demonstration |
| VI.32 | Naive String Matching | `demos_flow_string.c` | `naiveSearch()` | Implemented | Comparison |
| VI.33 | Rabin-Karp | `demos_flow_string.c` | `rabinKarpSearch()` | Implemented | Comparison |
| VI.34 | String Matching with Finite Automata | `demos_flow_string.c` | `automatonSearch()` | Implemented | Comparison |
| VI.35 | Knuth-Morris-Pratt | `demos_flow_string.c` | `kmpSearch()` | Implemented | Comparison |
| VI.36 | Chinese Remainder Theorem | `demos_flow_string.c` | `crtSolve()` | Implemented | Demonstration |
| VI.37 | Complexity class P | `complexity.c` | `showTheory()` | Theory | Theory |
| VI.38 | NP and NP-Complete problems | `complexity.c` | `showTheory()` | Theory | Theory |
| VI.39 | NP-Hard problems | `complexity.c` | `showTheory()` | Theory | Theory |
| VI.40 | Polynomial-time reducibility | `complexity.c` | `showTheory()` | Theory | Theory |

## Project-specific supporting algorithms (not syllabus items)

| Algorithm | Source file | Function | Type |
|---|---|---|---|
| Linear Search | `inventory.c` | `findPinNodeLinear()`, `findProductLinear()` | Comparison |
| Heap-based Dijkstra (adjacency list) | `graph.c` | `dijkstraCsrHeap()` | Comparison (stress scenario) |
| Array-based Dijkstra on adjacency lists | `graph.c` | `dijkstraCsrArray()` | Comparison (stress scenario) |
| Negative-cycle detection | `graph.c` | `graphHasNegativeCycle()` | Core (protects every road edit) |
| Redistribution advice | `redistribute.c` | `recommendTransfers()` | Core module |
| Order placement and warehouse selection | `fulfil.c` | `placeOrder()` | Core |

## Incomplete, missing or limited topics (stated honestly)

- **No mandatory topic is missing.** Every item of Units I to VI above is implemented, demonstrated, or explained as theory, as marked.
- **Theory-only topics** are not implemented as algorithms: algorithm characteristics, asymptotic notations, growth of functions, P, NP, NP-Complete, NP-Hard and reducibility. They are explained in `showTheory()` and the docs.
- **Strassen** is implemented only for matrix sizes 2, 4 and 8 and is a demonstration, not a production multiplier.
- **TSP** is limited to 8 stops by design (it is NP-hard).
- **Insertion, Selection and Heap Sort** appear in the sorting comparison as theory. They are not implemented because they are not in the provided syllabus list and no step of the project needs them.
- **Number and Polynomial Representation:** the provided syllabus lists only Powering a Number and the Chinese Remainder Theorem under this theme, so no polynomial arithmetic is implemented.
- **Master Theorem:** the classifier covers recurrences of the form aT(n/b) + Theta(n^c) only.
