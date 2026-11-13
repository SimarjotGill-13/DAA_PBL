# Complexity analysis

All complexities use n for input size, V and E for graph nodes and edges, W for knapsack capacity, k for the number of agents or warehouses, m for pattern length. "Auxiliary space" means extra memory beyond the input.

## 1. Time and space of every implemented algorithm

| Algorithm | Best | Average | Worst | Auxiliary space | Source function |
|---|---|---|---|---|---|
| Binary Search | O(1) | O(log n) | O(log n) | O(1) | `findPinNode()` |
| Linear Search | O(1) | O(n) | O(n) | O(1) | `findPinNodeLinear()` |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) | O(n) | `mergeSort()` |
| Quick Sort (last pivot) | O(n log n) | O(n log n) | O(n^2) | O(log n) avg, O(n) worst | `quickSort()` |
| Dijkstra (array) | O(V^2) | O(V^2) | O(V^2) | O(V) | `dijkstra()` |
| Dijkstra (binary heap) | O((V+E) log V) | O((V+E) log V) | O((V+E) log V) | O(V + E) | `dijkstraCsrHeap()` |
| Dijkstra (array, adjacency list) | O(V^2 + E) | O(V^2 + E) | O(V^2 + E) | O(V) | `dijkstraCsrArray()` |
| Bellman-Ford | O(E) | O(VE) | O(VE) | O(V) | `bellmanFord()` |
| Floyd-Warshall | O(V^3) | O(V^3) | O(V^3) | O(V^2) | `floydWarshall()` |
| Negative-cycle check | O(VE) | O(VE) | O(VE) | O(V) | `graphHasNegativeCycle()` |
| Order placement (one order) | O(log n + V^2) | same | O(VE) with negative roads | O(V^2) | `placeOrder()` |
| Redistribution advice | O(P W^2) + routing | same | same | O(V + W) | `recommendTransfers()` |
| Activity Selection | O(n log n) | O(n log n) | O(n log n) | O(n) | `activitySelection()` |
| Interval Partitioning | O(n log n + nk) | same | same | O(n) | `intervalPartition()` |
| Fractional Knapsack | O(n log n) | O(n log n) | O(n log n) | O(n) | `knapFractional()` |
| 0/1 Knapsack DP | O(nW) | O(nW) | O(nW) | O(nW) table | `knapDP()` |
| 0/1 Knapsack Backtracking | O(n) pruned | O(2^n) | O(2^n) | O(n) | `knapBacktrack()` |
| 0/1 Knapsack Branch and Bound | O(n) | pruned | O(2^n) | O(2^n) live nodes | `knapBB()` |
| Maximum Subarray (D&C) | O(n log n) | O(n log n) | O(n log n) | O(log n) | `maxSubarray()` |
| Powering a number | O(log n) | O(log n) | O(log n) | O(log n) | `powerDC()` |
| Celebrity Problem | O(n) | O(n) | O(n) | O(1) | `celebrityFind()` |
| Strassen | O(n^2.81) | O(n^2.81) | O(n^2.81) | O(n^2) | `strassenMultiply()` |
| Standard matrix multiplication | O(n^3) | O(n^3) | O(n^3) | O(1) | `standardMultiply()` |
| Huffman (simple minimum search) | O(n^2) | O(n^2) | O(n^2) | O(n) | `huffmanBits()` |
| Prim (matrix) | O(V^2) | O(V^2) | O(V^2) | O(V) | `mstPrim()` |
| Kruskal | O(E log E) | O(E log E) | O(E log E) | O(V + E) | `mstKruskal()` |
| LCS | O(mn) | O(mn) | O(mn) | O(mn) | `lcsLength()` |
| Matrix Chain Multiplication | O(n^3) | O(n^3) | O(n^3) | O(n^2) | `matrixChainMin()` |
| Optimal BST | O(n^3) | O(n^3) | O(n^3) | O(n^2) | `obstMin()` |
| TSP brute force | O(n!) | O(n!) | O(n!) | O(n) | `tspSolveAll()` |
| TSP Held-Karp | O(n^2 2^n) | O(n^2 2^n) | O(n^2 2^n) | O(n 2^n) | `tspSolveAll()` |
| TSP Branch and Bound | pruned | pruned | O(n!) | O(n!) live nodes | `tspSolveAll()` |
| N-Queens | pruned | pruned | O(n!) | O(n) | `nQueensCount()` |
| Sum of Subsets | pruned | pruned | O(2^n) | O(n) | `subsetSumCount()` |
| Ford-Fulkerson (DFS) | O(E) | O(E f) | O(E f) | O(V^2) | `maxFlow()` |
| Bipartite matching | O(E) | O(VE) | O(VE) | O(V) | `bipartiteMatching()` |
| Naive string matching | O(n) | O(nm) | O(nm) | O(1) | `naiveSearch()` |
| Rabin-Karp | O(n + m) | O(n + m) | O(nm) | O(1) | `rabinKarpSearch()` |
| Finite automaton matching | O(n) | O(n) | O(n) (+ O(m^3 sigma) build) | O(m sigma) | `automatonSearch()` |
| KMP | O(n + m) | O(n + m) | O(n + m) | O(m) | `kmpSearch()` |
| Chinese Remainder Theorem | O(k log M) | O(k log M) | O(k log M) | O(1) | `crtSolve()` |

## 2. Recurrence relations and their solutions

| Algorithm | Recurrence | Method | Solution |
|---|---|---|---|
| Binary Search | T(n) = T(n/2) + 1 | Master, case 2 (a=1, b=2, c=0) | Theta(log n) |
| Merge Sort | T(n) = 2T(n/2) + n | Master, case 2 (a=2, b=2, c=1); also iteration and recursion tree | Theta(n log n) |
| Maximum Subarray | T(n) = 2T(n/2) + n | same as Merge Sort | Theta(n log n) |
| Powering a number | T(n) = T(n/2) + 1 | Master, case 2 | Theta(log n) |
| Strassen | T(n) = 7T(n/2) + n^2 | Master, case 1 (log2 7 = 2.807 > 2) | Theta(n^2.81) |
| Standard matrix multiplication (D&C, 8 sub-products) | T(n) = 8T(n/2) + n^2 | Master, case 1 (log2 8 = 3) | Theta(n^3) |
| Quick Sort, balanced | T(n) = 2T(n/2) + n | Master, case 2 | Theta(n log n) |
| Quick Sort, worst case | T(n) = T(n-1) + n | Iteration: n + (n-1) + ... + 1 | Theta(n^2) |

The program evaluates T(n) = 2T(n/2) + n directly and compares it with n(log2 n + 1) (menu 14, option 2). The Master Theorem solver (menu 14, option 3) classifies any T(n) = aT(n/b) + Theta(n^c).

**Master Theorem recap.** Compare c with log_b a. If c < log_b a, T(n) = Theta(n^(log_b a)). If c = log_b a, T(n) = Theta(n^c log n). If c > log_b a, T(n) = Theta(n^c) (the regularity condition holds for polynomial f).

## 3. Best, average and worst-case comparisons

**Sorting**

| | Best | Average | Worst | Space | Stable | Used here |
|---|---|---|---|---|---|---|
| Merge Sort | n log n | n log n | n log n | O(n) | yes | main sort |
| Quick Sort | n log n | n log n | n^2 | O(log n) | no | compared |
| Heap Sort | n log n | n log n | n log n | O(1) | no | theory only |
| Insertion Sort | n | n^2 | n^2 | O(1) | yes | theory only |
| Selection Sort | n^2 | n^2 | n^2 | O(1) | no | theory only |

Merge Sort is chosen because stability and a guaranteed bound matter more here than saving memory. Insertion Sort is best for tiny or nearly sorted arrays. Heap Sort sorts in place with a guaranteed bound but is not stable. Selection Sort always does about n^2/2 comparisons. These three are not implemented because no step of the project needs them.

**Shortest paths**

| | Negative edges | Output | When it is the right tool |
|---|---|---|---|
| Dijkstra | no | one source | non-negative roads; the normal case here |
| Bellman-Ford | yes | one source | any negative net cost; also detects negative cycles |
| Floyd-Warshall | yes (no negative cycle) | all pairs | many queries on a small graph; distance matrix for TSP |

## 4. Theoretical versus measured performance

Measured values below come from one run of the program on the machine described in section 6. They are indicative, not a rigorous benchmark.

| Test (same input for both algorithms) | Theory predicts | Measured |
|---|---|---|
| Sort 4000 random integers: Merge Sort | about n log2 n = 47,900 comparisons or fewer | 42,782 comparisons, 0.57 ms |
| Sort 4000 random integers: Quick Sort | about 1.39 n log2 n on average | 51,774 comparisons, 0.36 ms |
| Sort 4000 sorted integers: Merge Sort | about (n/2) log2 n | 23,728 comparisons, 0.26 ms |
| Sort 4000 sorted integers: Quick Sort | exactly n(n-1)/2 = 7,998,000 | 7,998,000 comparisons, 30.90 ms |
| Sort 20000 random integers: Merge / Quick | n log n growth | 260,911 / 323,095 comparisons; 3.04 / 2.14 ms |
| Sort 20000 sorted integers: Merge / Quick | n(n-1)/2 = 199,990,000 for Quick | 139,216 / 199,990,000 comparisons; 1.39 / 764.54 ms |
| Search last of 100,000 sorted items | Linear 100,000, Binary about log2 n = 17 | 100,000 / 17 comparisons |
| Search last of 1,000,000 sorted items | Linear 1,000,000, Binary about 20 | 1,000,000 / 20 comparisons |
| Dijkstra on 5000 nodes, 25,000 roads: array scan | O(V^2 + E), about 25 million steps | 18.8 to 19.5 ms over four runs |
| Dijkstra on the same graph: binary heap | O((V + E) log V), about 0.4 million steps | 0.78 to 0.80 ms over four runs |

**Reading the results.** The comparison counts match theory closely: Quick Sort's sorted-input count equals n(n-1)/2 exactly, and Merge Sort stays near n log n. Quick Sort is faster than Merge Sort on random data and far slower on sorted data with this pivot rule. The heap version of Dijkstra is about 24 times faster than the array version on this large sparse graph, while on the 9-node sample graph either is instant, which is why the array version is used for the order workflow. Distances from the two Dijkstra versions were identical in every run.

## 5. Benchmark methodology

- **Input:** deterministic pseudo-random data from a fixed linear congruential generator (fixed seeds), so every run uses the same input. The sorted case is the sequence 0, 1, 2, ...
- **Fairness:** both algorithms in a comparison receive an identical copy of the same input, and results are checked (sortedness for sorts, identical distance arrays for Dijkstra).
- **Counting:** comparisons are counted by a wrapper around the comparison function, so counts do not depend on the machine.
- **Timing:** `clock()` around a single run. Timing includes memory copying done inside the algorithm but not input generation. One run per measurement; differences below a few percent are noise.
- **Compile settings:** `-std=c17 -O2`.
- **Reproduce:** run `./order_optimizer`, choose 14 then 1 for the 4000-element sort and search comparison, or run `./order_optimizer --scenarios` for the stress scenario (20000-element sorts, 1,000,000-element search, 5000-node Dijkstra).

## 6. Hardware and compiler used for the numbers above

| Item | Value |
|---|---|
| Date of run | 2026-11-12 |
| OS | Linux 6.18 (sandbox environment) |
| CPU | Intel Xeon processor @ 2.10 GHz, 1 vCPU |
| Compiler | gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| Flags | `-std=c17 -Wall -Wextra -Wpedantic -O2` |

**Your machine (fill in after running the program yourself):**

| Item | Value |
|---|---|
| Date of run | |
| OS | |
| CPU | |
| Compiler and version | |
| Merge Sort, 4000 sorted: comparisons / ms | |
| Quick Sort, 4000 sorted: comparisons / ms | |
| Dijkstra array vs heap, 5000 nodes: ms | |
