# Algorithms

Every algorithm below follows the same template: purpose, working principle, pseudocode, step-by-step example, why it is used, time complexity (best / average / worst), auxiliary space, limitations, and the source file and function. Part 1 is the core order-fulfilment workflow. Part 2 covers the supporting modules. Part 3 covers the supplementary demonstrations. Part 4 lists topics that are theory only. Nothing in Part 4 is presented as an implemented algorithm.

**Status words used in this document:** *Core* = used by the order workflow or a main menu module. *Comparison* = implemented so two strategies can be compared on the same input. *Demonstration* = standalone demo. *Theory* = explanation only.

---

# Part 1. Core order-fulfilment workflow

## 1.1 Which algorithm is used at each step

| Step | Task | Algorithm used | Why this one, and what was rejected |
|---|---|---|---|
| 1 | Validate PIN serviceability | Binary Search on the sorted PIN array | O(log n) after one sort. A hash table would be O(1) on average but needs a hash function and collision handling for a tiny static list. Linear search is O(n) |
| 2 | Find the product | Binary Search on sorted product IDs | Same reasoning |
| 3 | Find a product's inventory records | Lower-bound Binary Search on inventory sorted by (product, warehouse) | O(log n + k) for k warehouses; a full scan would be O(n) per order |
| 4 | Sort the data once at load time | Merge Sort | Stable and O(n log n) in every case. Quick Sort can degrade to O(n^2); Insertion and Selection Sort are O(n^2) |
| 5 | Filter eligible warehouses | Linear scan of k records | Needs one pass over at most 8 records |
| 6 | Shortest distance customer to every warehouse | Dijkstra when every road is non-negative, Bellman-Ford when a negative road exists (chosen automatically) | Dijkstra is faster; Bellman-Ford is the only one of the two that is correct with negative weights. Floyd-Warshall (O(V^3)) computes all pairs when only one source is needed |
| 7 | Choose the cheapest warehouse | Single linear minimum scan with tie-break | Only the minimum is needed, so sorting all candidates would waste work |
| 8 | Reserve stock | O(1) update | Done only after a warehouse is chosen |
| 9 | Redistribution advice | Greedy: largest deficit first, cheapest source first | Transparent rules; sorted with Merge Sort; distances from Dijkstra or Bellman-Ford |

**Warehouse selection is not one standalone algorithm.** It is a pipeline: filter by sellable stock, compute cost from the shortest-path distance, take the minimum, apply the tie-break. Dijkstra supplies distances only.

**Direction of roads.** A delivery travels from the warehouse to the customer. The program runs the shortest-path algorithm once from the customer on the *reversed* graph. That gives the distance from every warehouse to the customer in one run, and it stays correct when roads are one-way.

## 1.2 Core order workflow

- **Purpose:** accept an order only if it can be fulfilled by one warehouse, and pick the cheapest warehouse.
- **Principle:** validate, filter, route, compare, reserve.
- **Pseudocode:**
```
placeOrder(pin, product, qty):
    if qty <= 0: fail BAD_QTY
    node <- binarySearch(pins, pin);         if none: fail BAD_PIN
    p    <- binarySearch(products, product); if none: fail BAD_PRODUCT
    for each inventory record r of the product:
        sellable <- total - damaged - reserved
        total_sellable += sellable
        if sellable >= qty: candidates.add(r)
    if total_sellable < qty: fail INSUFFICIENT
    if candidates empty:     fail NO_SINGLE_WH
    dist <- shortestPathsTo(node)            // Dijkstra or Bellman-Ford
    best <- none
    for each candidate c:
        if dist[c.node] is INF: skip
        cost <- max(0, dist[c.node] * RATE + c.fee)
        best <- c if (cost, distance, warehouse id) is smaller
    if best is none: fail UNREACHABLE
    best.reserved += qty
    report warehouse, route, cost, stock
```
- **Steps (example):** PIN 100001, product 101, quantity 2. W1 sellable 10, W2 0, W3 5. Eligible: W1, W3. Distances to CustX: W1 6, W3 12. Costs: W1 6+5 = 11, W3 12+4 = 16. W1 wins.
- **Why used:** it follows the required rules: serviceability first, sellable stock computed (never stored), reserve only after allocation, deterministic tie-break.
- **Time:** O(log n) lookups + O(V^2) routing (Dijkstra) or O(VE) (Bellman-Ford) + O(k) selection. Best, average and worst differ only through the routing choice.
- **Auxiliary space:** O(V) for distances and parents, plus O(V^2) to build the reversed graph.
- **Limitations:** one warehouse fulfils the whole order; orders are processed one at a time; a net cost below zero is floored at zero.
- **Source:** `src/fulfil.c` — `placeOrder()`.

## 1.3 Dijkstra's algorithm (core)

- **Purpose:** shortest distance from one source to every node in a graph with non-negative edge weights.
- **Principle:** greedy. Keep a tentative distance per node. Repeatedly fix the unvisited node with the smallest tentative distance, then relax its outgoing edges. With non-negative weights, a fixed node can never be improved later.
- **Pseudocode:**
```
dijkstra(G, src):
    dist[all] <- INF; dist[src] <- 0; parent[all] <- none
    repeat |V| times:
        u <- unvisited node with the smallest dist
        if dist[u] = INF: stop
        mark u visited
        for each edge (u, v, w):
            if dist[u] + w < dist[v]:        // relaxation
                dist[v] <- dist[u] + w; parent[v] <- u
```
- **Steps (example, source CustX):** CustX = 0, HubA = 2, HubB = 5. Fix HubA: W1 = 6. Fix HubB: W2 = 11, HubC = 7, CustY = 12. Fix W1. Fix HubC: W3 = 12, CustY improves from 12 to 10. The rest are final. CustZ stays unreachable.
- **Why used:** road lengths are non-negative, and one run answers every warehouse.
- **Time:** best, average and worst are all O(V^2) with the array version; O((V + E) log V) with a binary heap.
- **Auxiliary space:** O(V).
- **Limitations:** wrong results with negative weights; single source; recomputed if roads change.
- **Source:** `src/graph.c` — `dijkstra()`; automatic selection in `shortestPaths()`; reversed-graph helper `distancesToNode()`. Heap and array variants for large sparse graphs: `dijkstraCsrHeap()`, `dijkstraCsrArray()`.

## 1.4 Bellman-Ford (core when a negative road exists)

- **Purpose:** shortest paths from one source when edge weights may be negative; detects negative cycles.
- **Principle:** relax every edge |V| - 1 times. A shortest path has at most |V| - 1 edges, so after that many passes all distances are final. One more pass that still improves something proves a negative cycle.
- **Pseudocode:**
```
bellmanFord(G, src):
    dist[all] <- INF; dist[src] <- 0
    repeat |V| - 1 times:
        for each edge (u, v, w): if dist[u] + w < dist[v]: dist[v] <- dist[u] + w
    for each edge (u, v, w): if dist[u] + w < dist[v]: return NEGATIVE_CYCLE
```
- **Steps (example):** A->B 4, A->C 5, C->B -3, B->D 2 from A. Pass 1 gives B = 4, C = 5, D = 6, then C->B improves B to 2. Pass 2 improves D to 4. Result B = 2, D = 4. Dijkstra gives B = 4, D = 6 here, which is wrong.
- **Why used:** a net cost (promo credit, rebate) can be negative even though physical distance cannot. The program switches to it automatically when any road is negative.
- **Time:** best O(E) (early stop when no change), average and worst O(VE).
- **Auxiliary space:** O(V).
- **Limitations:** slower than Dijkstra; with a negative cycle reachable, shortest distances are undefined. A negative two-way road is a negative cycle by itself, so negative roads must be one-way. The program refuses any road that would create a negative cycle (`graphAddArc()` returns 3).
- **Source:** `src/graph.c` — `bellmanFord()`, `graphHasNegativeCycle()`, `graphAddArc()`, `demoNegativeWeights()`.

## 1.5 Floyd-Warshall (comparison; also used by the TSP demo)

- **Purpose:** shortest distance between every pair of nodes.
- **Principle:** dynamic programming. d_k[i][j] = the shortest i to j path using only the first k nodes as intermediates.
- **Pseudocode:**
```
for k in nodes: for i in nodes: for j in nodes:
    if d[i][k] + d[k][j] < d[i][j]: d[i][j] <- d[i][k] + d[k][j]
```
- **Steps (example):** to improve W1 to CustX, check whether going through HubA beats the current entry.
- **Why used:** compared with Dijkstra and Bellman-Ford on the same graph; supplies the distance matrix for the TSP demo, where distances between all stops are needed.
- **Time:** best, average and worst O(V^3).
- **Auxiliary space:** O(V^2).
- **Limitations:** wasteful when only one source is needed; a negative cycle makes it invalid (detected by a negative diagonal entry).
- **Source:** `src/graph.c` — `floydWarshall()`.

## 1.6 Binary Search and Linear Search (core / comparison)

- **Purpose:** find a key in an array.
- **Principle:** Binary Search repeatedly halves a sorted range. Linear Search checks every element.
- **Pseudocode:**
```
binarySearch(a, key): lo <- 0; hi <- n-1
    while lo <= hi: mid <- lo + (hi - lo)/2
        if a[mid] = key return mid; else if a[mid] < key: lo <- mid+1 else hi <- mid-1
    return none
```
- **Steps (example):** PINs 100001, 100002, 100003 and key 100003: mid = 100002 (smaller) then 100003 found, 2 comparisons.
- **Why used:** PIN, product and inventory lookups are repeated many times on data that is sorted once.
- **Time:** Binary best O(1), average and worst O(log n). Linear best O(1), average and worst O(n).
- **Auxiliary space:** O(1).
- **Limitations:** Binary Search needs sorted data. For 100,000 items the last element took 100,000 linear comparisons and 17 binary comparisons in the measured run (see `COMPLEXITY.md`).
- **Source:** `src/inventory.c` — `findPinNode()`, `findProductBinary()`, `invRange()`, `invIndex()` (binary); `findPinNodeLinear()`, `findProductLinear()` (linear).

## 1.7 Merge Sort (core) and Quick Sort (comparison)

- **Purpose:** sort records.
- **Principle:** Merge Sort splits in half, sorts each half, merges. Quick Sort partitions around a pivot and sorts each side.
- **Pseudocode:**
```
mergeSort(a): if n < 2 return; mergeSort(left); mergeSort(right); merge(left, right)
quickSort(a): pivot <- last element; partition so smaller items precede it; sort both sides
```
- **Steps (example):** [3,1,2] -> split [3] [1,2] -> [1,2] -> merge -> [1,2,3].
- **Why used:** Merge Sort is stable (equal keys keep their order) with a guaranteed bound, so it is used for products, PINs, inventory, deliveries and edges. Quick Sort is implemented for comparison.
- **Time:** Merge: best, average, worst O(n log n). Quick: best and average O(n log n), worst O(n^2) (already sorted input with a last-element pivot).
- **Auxiliary space:** Merge O(n). Quick O(log n) average stack, O(n) worst.
- **Limitations:** Merge needs extra memory; Quick is not stable and has a bad worst case. `mergeSort()` returns -1 on allocation failure.
- **Source:** `src/inventory.c` — `mergeSort()`, `quickSort()`.

## 1.8 Redistribution advice (core module)

- **Purpose:** recommend stock transfers between warehouses without executing them.
- **Principle:** greedy. surplus = sellable - safety stock - demand x horizon; deficit = demand x horizon + safety stock - sellable. Serve the largest deficit first; for each, try the cheapest source (Dijkstra / Bellman-Ford distance). Recommend only if transfer cost <= units x benefit.
- **Pseudocode:**
```
for each product:
    compute surplus[w], deficit[w] for each warehouse
    needs <- warehouses with deficit > 0, sorted by deficit descending
    for each need d:
        while deficit[d] > 0:
            s <- cheapest unused source with surplus > 0
            qty <- min(surplus[s], deficit[d])
            if cost(s, d) <= qty * BENEFIT: recommend move qty from s to d
```
- **Steps (example):** Phones: W2 needs 14, has 0; W1 can spare 4 at transfer cost 13 <= 20, so W1 to W2 is recommended; W3 can spare 1 at cost 13 > 5, so it is skipped.
- **Why used:** transparent rules that can be defended; machine learning is not required.
- **Time:** O(P x W^2) plus one routing run per destination.
- **Auxiliary space:** O(V + W).
- **Limitations:** a heuristic, not an optimal plan; one pass over destinations.
- **Source:** `src/redistribute.c` — `recommendTransfers()`.

---

# Part 2. Scheduling and loading

## 2.1 Activity Selection (core)

- **Purpose:** one delivery agent, choose the largest set of non-overlapping deliveries.
- **Principle:** greedy by earliest finish time; touching intervals (end = next start) are compatible.
- **Pseudocode:**
```
sort deliveries by end time
last <- -infinity
for each d in order: if d.start >= last: choose d; last <- d.end
```
- **Steps (example):** sorted by end, D7 (9:00-10:00), D4 (10:00-11:30), D6 (12:00-13:00) are chosen: 3 deliveries.
- **Why used:** earliest finish leaves the most room; an exchange argument proves it optimal.
- **Time:** O(n log n) in every case (dominated by the sort; the scan is O(n)).
- **Auxiliary space:** O(n).
- **Limitations:** one resource; drops conflicting jobs; unweighted (every delivery counts equally).
- **Source:** `src/schedule.c` — `activitySelection()`.

## 2.2 Interval Partitioning (core)

- **Purpose:** serve all deliveries using the fewest agents.
- **Principle:** greedy by earliest start; give each delivery any free agent, or open a new one. The minimum equals the maximum number of deliveries overlapping at one moment.
- **Pseudocode:**
```
sort deliveries by start time; k <- 0
for each d: find an agent a with free_at[a] <= d.start
    if none: a <- k; k <- k + 1
    free_at[a] <- d.end; assign d to a
```
- **Steps (example):** the 8 sample deliveries need 3 agents; the maximum overlap is 3.
- **Why used:** the real requirement is often to serve everything; this tells how many agents are needed.
- **Time:** O(n log n + nk) in every case.
- **Auxiliary space:** O(n).
- **Limitations:** assumes agents are identical and all deliveries must be served.
- **Source:** `src/schedule.c` — `intervalPartition()`.
- **Difference from 2.1:** Activity Selection picks compatible jobs for one resource; Interval Partitioning assigns all overlapping jobs to many resources.

## 2.3 Fractional Knapsack (comparison)

- **Purpose:** maximum value when items can be split.
- **Principle:** greedy by value/weight ratio.
- **Pseudocode:** `sort by v/w descending; take whole items while they fit; take a fraction of the next`.
- **Steps (example):** capacity 50 with parcels (10,60), (20,100), (15,75), (30,120): takes 10 + 20 + 15 kg and 5/30 of the last, value 255.
- **Why used:** shows why parcels cannot be treated as divisible: 255 is higher than the true optimum of 235 only because it cuts a parcel.
- **Time:** O(n log n). **Auxiliary space:** O(1) beyond the sorted copy.
- **Limitations:** invalid for indivisible parcels.
- **Source:** `src/schedule.c` — `knapFractional()`.

## 2.4 0/1 Knapsack by Dynamic Programming (core)

- **Purpose:** choose indivisible parcels to maximise value within a weight limit.
- **Principle:** dp[i][w] = best value using the first i parcels with capacity w; each cell is the better of skipping or taking parcel i.
- **Pseudocode:**
```
dp[0][w] <- 0
for i in 1..n: for w in 0..W:
    dp[i][w] <- dp[i-1][w]
    if wt[i] <= w: dp[i][w] <- max(dp[i][w], dp[i-1][w - wt[i]] + val[i])
```
- **Steps (example):** result 235 using P1, P2, P4 (45 kg).
- **Why used:** greedy by ratio is not optimal for 0/1; DP is exact and easy to trace.
- **Time:** O(nW) in every case (pseudo-polynomial).
- **Auxiliary space:** O(nW) for the table (O(W) if only the value is needed).
- **Limitations:** depends on the numeric size of W; the decision version is NP-complete.
- **Source:** `src/schedule.c` — `knapDP()`.

## 2.5 0/1 Knapsack by Backtracking (comparison)

- **Purpose:** the same problem solved by exhaustive search with feasibility pruning.
- **Principle:** include or exclude each parcel; abandon a branch when the weight exceeds the capacity.
- **Pseudocode:** `bt(i, w, v): if w > W return; if i = n: best <- max(best, v); bt(i+1, w+wt[i], v+val[i]); bt(i+1, w, v)`.
- **Steps (example):** 29 recursive calls on the sample, result 235.
- **Time:** best O(n) with heavy pruning, average and worst O(2^n). **Auxiliary space:** O(n) stack.
- **Limitations:** exponential; prunes only on weight.
- **Source:** `src/schedule.c` — `knapBacktrack()`.

## 2.6 0/1 Knapsack by Branch and Bound: FIFO, LIFO, Least-Cost (comparison)

- **Purpose:** the same problem with bound-based pruning.
- **Principle:** each node has an upper bound (profit so far plus a fractional fill of the remaining parcels). A node whose bound cannot beat the best known value is pruned. The three variants differ only in which live node is expanded next: FIFO (queue), LIFO (stack), Least-Cost (best bound first).
- **Pseudocode:**
```
live <- {root}; best <- 0
while live not empty: take a node by the chosen rule
    if bound <= best: continue
    expand include-child (if it fits) and exclude-child; update best; push children whose bound > best
```
- **Steps (example):** all three modes return 235; expanded nodes were 6 (FIFO), 14 (LIFO), 5 (Least-Cost) in the measured run.
- **Time:** best O(n), average depends on pruning, worst O(2^n). **Auxiliary space:** O(2^n) live nodes worst case.
- **Limitations:** the bound quality decides the speed.
- **Source:** `src/schedule.c` — `knapBB()` (mode 0, 1, 2).

---

# Part 3. Supplementary demonstrations

These do not belong to the order workflow and are kept separate from it.

## 3.1 Maximum Subarray (Divide and Conquer)
- **Purpose:** the consecutive stretch of a sequence with the largest sum.
- **Principle:** the best subarray lies in the left half, the right half, or crosses the middle; compute all three and take the maximum.
- **Pseudocode:** `maxSub(lo,hi): if lo = hi return a[lo]; return max(maxSub(left), maxSub(right), crossing(lo,mid,hi))`.
- **Steps (example):** the sample sequence gives 43 over days 7 to 10.
- **Why used:** a clean Divide and Conquer example with recurrence T(n) = 2T(n/2) + n.
- **Time:** O(n log n) in every case. **Space:** O(log n) stack.
- **Limitations:** a linear scan (Kadane) is O(n).
- **Source:** `src/demos_greedy_dc.c` — `maxSubarray()`.

## 3.2 Powering a number (Divide and Conquer)
- **Purpose:** x^n with few multiplications.
- **Principle:** x^n = (x^(n/2))^2, times x if n is odd.
- **Pseudocode:** `power(x,n): if n = 0 return 1; h <- power(x, n/2); return n odd ? h*h*x : h*h`.
- **Steps (example):** 2^10 = (2^5)^2, 2^5 = (2^2)^2 x 2, ...
- **Why used:** needed for modular hashing and CRT.
- **Time:** O(log n). **Space:** O(log n) stack.
- **Limitations:** overflow for large results (the modular version avoids it).
- **Source:** `src/demos_greedy_dc.c` — `powerDC()`, `modPowerDC()`.

## 3.3 Celebrity Problem
- **Purpose:** find the person known by everyone who knows nobody.
- **Principle:** each question "does a know b?" eliminates one of the two as the celebrity.
- **Pseudocode:** `c <- 0; for i in 1..n-1: if knows(c,i): c <- i; verify c`.
- **Steps (example):** candidate 0 knows 1, so candidate 1; 1 knows 2, so candidate 2; 2 knows nobody; verified.
- **Time:** O(n) knows-checks. **Space:** O(1).
- **Limitations:** needs the full relation matrix.
- **Source:** `src/demos_greedy_dc.c` — `celebrityFind()`.

## 3.4 Strassen matrix multiplication (Divide and Conquer)
- **Purpose:** multiply matrices with 7 sub-multiplications instead of 8.
- **Principle:** split each matrix into four blocks; compute M1..M7 from sums and differences of blocks; combine.
- **Pseudocode:** `strassen(A,B): if n = 1 return a*b; split into blocks; compute M1..M7 recursively; combine into C`.
- **Steps (example):** for n = 4 Strassen uses 49 scalar multiplications; the standard method uses 64.
- **Why used:** shows how a recurrence T(n) = 7T(n/2) + O(n^2) beats O(n^3).
- **Time:** O(n^2.81). **Space:** O(n^2) for the temporary blocks.
- **Limitations:** implemented for n a power of two up to 8; large constants make it slower than the standard method for small n; numerically less stable for floating point.
- **Source:** `src/demos_greedy_dc.c` — `strassenMultiply()`, `standardMultiply()`.

## 3.5 Huffman coding (Greedy)
- **Purpose:** prefix-free codes that minimise total encoded length.
- **Principle:** repeatedly merge the two least frequent symbols into one node.
- **Pseudocode:** `while more than one node: take the two smallest; create a parent with their sum`.
- **Steps (example):** the sample text uses fewer bits than 8-bit coding.
- **Time:** O(n^2) with the simple minimum search (O(n log n) with a heap). **Space:** O(n).
- **Limitations:** needs the frequencies in advance.
- **Source:** `src/demos_greedy_dc.c` — `huffmanBits()`.

## 3.6 Prim and Kruskal (Greedy, comparison)
- **Purpose:** minimum spanning tree (forest) of an undirected, non-negative road network.
- **Principle:** Prim grows one tree by the cheapest crossing edge. Kruskal adds the cheapest edge that does not form a cycle (union-find).
- **Pseudocode:** `Prim: key[v] <- min edge to tree; add the smallest key each round. Kruskal: sort edges; add if find(u) != find(v)`.
- **Steps (example):** both give total weight 25 on the sample network.
- **Time:** Prim O(V^2) with a matrix; Kruskal O(E log E). **Space:** O(V) and O(V + E).
- **Limitations:** the program refuses directed or negative graphs.
- **Source:** `src/demos_greedy_dc.c` — `mstPrim()`, `mstKruskal()`.

## 3.7 Longest Common Subsequence (DP)
- **Purpose:** longest sequence appearing in both strings in order.
- **Principle:** L[i][j] = L[i-1][j-1] + 1 on a match, else the larger of L[i-1][j] and L[i][j-1].
- **Steps (example):** SmartPhone and SmartWatch give "Smarth", length 6.
- **Time:** O(mn). **Space:** O(mn).
- **Limitations:** strings up to 63 characters here.
- **Source:** `src/demos_dp_bt.c` — `lcsLength()`.

## 3.8 Matrix Chain Multiplication (DP)
- **Purpose:** the cheapest order in which to multiply a chain of matrices.
- **Principle:** m[i][j] = min over k of m[i][k] + m[k+1][j] + p[i]·p[k+1]·p[j+1].
- **Steps (example):** dimensions 30, 35, 15, 5, 10, 20, 25 give 15125 multiplications.
- **Time:** O(n^3). **Space:** O(n^2).
- **Limitations:** finds the order only; it does not multiply the matrices.
- **Source:** `src/demos_dp_bt.c` — `matrixChainMin()`.

## 3.9 Optimal Binary Search Tree (DP)
- **Purpose:** arrange sorted keys in a BST that minimises the expected search cost.
- **Principle:** cost[i][j] = min over roots r of cost[i][r-1] + cost[r+1][j] + sum of frequencies i..j.
- **Steps (example):** frequencies 34, 8, 50 give cost 142 with k3 at the root.
- **Time:** O(n^3). **Space:** O(n^2).
- **Limitations:** search frequencies only (no failed-search probabilities); up to 12 keys.
- **Source:** `src/demos_dp_bt.c` — `obstMin()`.

## 3.10 Travelling Salesperson: Held-Karp DP, Branch and Bound, brute force
- **Purpose:** the shortest tour visiting every stop once and returning to the depot.
- **Principle:** Held-Karp stores the best cost for each (visited set, last stop). Branch and Bound expands the node with the best lower bound first. Brute force tries every order. Distances between stops come from Floyd-Warshall.
- **Steps (example):** W1 -> CustX, CustY, HubA, HubC; all three methods give the same cost.
- **Time:** brute force O(n!); Held-Karp O(n^2 2^n); Branch and Bound O(n!) worst case but pruned.
- **Space:** O(n) / O(n 2^n) / live-node list.
- **Limitations:** at most 8 stops; TSP is NP-hard.
- **Source:** `src/demos_dp_bt.c` — `tspSolveAll()` (internal `brute()`, `heldKarp()`, `tspBB()`).

## 3.11 N-Queens (Backtracking)
- **Purpose:** place n queens so none attack each other.
- **Principle:** one queen per row; abandon a partial board at the first conflict.
- **Steps (example):** 4 -> 2 solutions, 6 -> 4, 8 -> 92.
- **Time:** worst O(n!). **Space:** O(n).
- **Source:** `src/demos_dp_bt.c` — `nQueensCount()`.

## 3.12 Sum of Subsets (Backtracking)
- **Purpose:** find subsets of parcel weights that exactly fill a vehicle.
- **Principle:** include or exclude each weight; prune when the sum exceeds the target or the remainder cannot reach it.
- **Time:** O(2^n). **Space:** O(n).
- **Source:** `src/demos_dp_bt.c` — `subsetSumCount()`.

## 3.13 Ford-Fulkerson maximum flow
- **Purpose:** the most units that can move from warehouses to customer areas under dispatch capacities.
- **Principle:** repeatedly find an augmenting path in the residual graph (DFS) and push the bottleneck amount; residual edges allow undoing earlier choices.
- **Steps (example):** supply 15, demand 15, maximum flow 14: dispatch capacity is the bottleneck.
- **Time:** O(E·f) for integer capacities. **Space:** O(V^2) here.
- **Limitations:** can be slow when f is large; capacities must be integers.
- **Source:** `src/demos_flow_string.c` — `maxFlow()`.

## 3.14 Maximum Bipartite Matching
- **Purpose:** assign as many orders as possible to eligible delivery agents.
- **Principle:** augmenting paths; a flow network with unit capacities.
- **Steps (example):** four orders and three agents give a matching of size 3.
- **Time:** O(V·E). **Space:** O(V).
- **Source:** `src/demos_flow_string.c` — `bipartiteMatching()`.

## 3.15 String matching: Naive, Rabin-Karp, Finite Automaton, KMP (comparison)
- **Purpose:** find every occurrence of a pattern in a text (product-name search).
- **Principle:** Naive slides and compares. Rabin-Karp compares rolling hashes and verifies on a match. The automaton has one state per matched prefix length and reads each text character once. KMP uses a prefix table so the text pointer never moves back.
- **Steps (example):** pattern "Earbuds" in the sample text appears 3 times for all four.
- **Time:** Naive O(nm) worst; Rabin-Karp O(n + m) average and O(nm) worst; Automaton O(n) matching after an O(m^3·sigma) build; KMP O(n + m).
- **Space:** O(1), O(1), O(m·sigma), O(m).
- **Limitations:** patterns are limited to 31 characters here.
- **Source:** `src/demos_flow_string.c` — `naiveSearch()`, `rabinKarpSearch()`, `automatonSearch()`, `kmpSearch()`.

## 3.16 Chinese Remainder Theorem
- **Purpose:** solve x = r_i (mod m_i) for pairwise coprime moduli.
- **Principle:** x = sum of r_i · M_i · (M_i^-1 mod m_i) mod M, where M is the product of the moduli.
- **Steps (example):** x = 2 (mod 3), 3 (mod 5), 2 (mod 7) gives x = 23 (mod 105).
- **Time:** O(k log M). **Space:** O(1).
- **Limitations:** moduli must be pairwise coprime; the program rejects other input.
- **Source:** `src/demos_flow_string.c` — `crtSolve()`.

## 3.17 Recurrence solving (Algorithm Foundations)
- **Purpose:** find the running time of recursive algorithms.
- **Methods implemented as demonstrations:** the iteration method, a recursion tree for n = 16, the substitution proof, and a Master Theorem classifier for T(n) = aT(n/b) + Theta(n^c).
- **Steps (example):** T(n) = 2T(n/2) + n gives Theta(n log n); the code also evaluates the recurrence directly and compares it with n(log2 n + 1).
- **Limitations:** the Master Theorem classifier covers polynomial combine costs only.
- **Source:** `src/complexity.c` — `masterCase()`, `demoRecurrences()`, `demoMasterTheorem()`.

---

# Part 4. Theory only (not implemented algorithms)

| Topic | Where it is explained |
|---|---|
| Algorithm characteristics, analysis, best/average/worst case | `showTheory()` in `src/complexity.c` and `COMPLEXITY.md` |
| Asymptotic notations (O, Omega, Theta) | `showTheory()` |
| Divide and Conquer vs Greedy vs Dynamic Programming | `showTheory()` |
| P, NP, NP-complete, NP-hard | `showTheory()`; TSP and 0/1 Knapsack are the project's examples |
| Polynomial-time reducibility | `showTheory()` |
| Insertion, Selection, Heap Sort | Compared in theory in `COMPLEXITY.md`; deliberately not implemented because Merge Sort is chosen |
