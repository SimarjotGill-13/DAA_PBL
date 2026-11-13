# Limitations and design decisions

This document states what the program does not do, and why some choices were made. A limitation listed here is a deliberate scope decision or a known restriction, not a hidden defect.

## 1. Sample data and offline operation

- The program runs entirely offline on deterministic sample data. The same input always gives the same output.
- The built-in dataset is small on purpose (3 products, 3 warehouses, 9 road nodes) so that every case can be traced by hand during an evaluation.
- Two larger or different datasets are provided in `data/` and can be loaded from the menu. A dataset file is validated completely before it replaces the current data; a file with any error is rejected and the current data is kept.
- Capacities are compile-time constants (`MAXV`, `MAXP`, `MAXW`, `MAXI`, `MAXPIN` in `include/common.h`). Larger data needs a change to those constants and a rebuild.
- There is no machine learning, no chatbot and no external service. Redistribution uses fixed, readable rules, not a forecast.

## 2. No external database or live inventory integration

- Inventory lives in memory and is lost when the program exits. There is no persistence layer.
- There is no connection to a warehouse management system, payment system or customer database.
- The order log holds at most 200 orders (`MAXORD`); later orders are still processed but are not added to the log.

## 3. One warehouse fulfils the complete order

- An order is served by a single warehouse that holds the full quantity. If the total stock across warehouses is enough but no single warehouse holds the whole quantity, the order is rejected with the reason "no single warehouse can fulfil the full quantity".
- Split shipments are not implemented. They would need a different decision rule (which warehouses, in what split, and how to compare a combined cost) and are listed as future work.
- Warehouse selection compares cost only: `distance x RATE + handling fee`. It does not model delivery time windows, vehicle capacity, shipping carriers or customer priority.
- A computed net cost below zero is treated as zero.

## 4. Inventory reservation behaviour

- Orders are processed one at a time, first come, first served. The first order reserves stock; later orders see the reduced sellable stock. This prevents overselling inside this program.
- Stock is reserved only after a warehouse has been selected. A rejected order never changes stock.
- Reserved stock is never released automatically. There is no order cancellation, expiry or return handling.
- This is a single-threaded console program. A real multi-user system would need transactions or locks so that two simultaneous requests cannot reserve the same unit. That is not implemented.

## 5. Damage recording assumptions

- The system only **records** inspection results; it cannot detect damage itself.
- Newly damaged units must come from units that are neither already damaged nor reserved, so a recorded damage can never push sellable stock below zero or take stock away from a confirmed order.
- Sellable stock is computed as `total - damaged - reserved` every time it is needed and is never stored, so a unit cannot be subtracted twice.
- Damaged stock is not written off, returned or replaced. There is no restock operation.

## 6. Graph and routing assumptions

- Road distances are integers. Absolute value is limited to 10,000 so that every path sum stays inside `int`.
- A road of length 0 is allowed. A missing road is a separate marker (`NOEDGE`).
- Two-way roads must be non-negative. A negative weight (a rebate or credit) is allowed only on a one-way road, and only if it does not create a negative cycle. The program rejects any road that would create one, because shortest paths are not defined when a cycle can lower the cost forever.
- Routing uses Dijkstra when every road is non-negative and Bellman-Ford when at least one road is negative. The choice is made automatically from the data.
- Distances are static. There is no live traffic, road closure schedule or time-dependent travel time (the closure in the demonstration scenario is applied by hand).
- Distance is measured from warehouse to customer, so one-way roads are handled correctly.
- The graph is stored as an adjacency matrix. That suits the small graphs used here (O(V^2) Dijkstra); a graph of tens of thousands of nodes would need an adjacency list and a heap. The stress test shows the heap version on a sparse graph, but the order workflow itself uses the matrix.

## 7. Algorithm limitations

| Algorithm | Limitation |
|---|---|
| Dijkstra | wrong on negative weights; single source |
| Bellman-Ford | O(VE); no shortest path exists when a negative cycle is reachable |
| Floyd-Warshall | O(V^3) and O(V^2) memory |
| Binary Search | needs sorted data (the program sorts once at load time) |
| Quick Sort | O(n^2) on sorted input with the last-element pivot; not stable |
| Merge Sort | needs O(n) extra memory |
| Activity Selection | one resource; all deliveries weigh the same |
| Interval Partitioning | assumes identical agents and that all deliveries must be served |
| Fractional Knapsack | invalid for indivisible parcels |
| 0/1 Knapsack DP | pseudo-polynomial: depends on the capacity value |
| Redistribution advice | a greedy heuristic, not an optimal plan |
| TSP demo | at most 8 stops; the problem is NP-hard |
| Strassen | sizes 2, 4 and 8 only; slower than the standard method at this size |
| Master Theorem solver | only recurrences of the form aT(n/b) + Theta(n^c) |
| String matching | patterns up to 31 characters |
| Chinese Remainder Theorem | moduli must be pairwise coprime |

## 8. Ethical and operational considerations

- **Fairness.** First come, first served is simple and predictable but is not the only possible rule. Other rules (priority customers, geographic fairness) would change who gets scarce stock. This choice should be stated openly to users of a real system.
- **Transparency.** Every decision can be explained from printed values: sellable stock per warehouse, distance, fee, cost and tie-break. Redistribution only recommends; a person decides.
- **Data accuracy.** Results are only as good as the stock counts and road distances supplied. Wrong damage records or outdated roads produce wrong warehouse choices.
- **Privacy.** The sample data contains no personal information. A real system would store customer addresses and PIN codes, which are personal data and need access control.
- **Environmental cost.** The cost model does not include emissions. A lower-cost warehouse may not be the lowest-emission choice.
- **Not for production use.** This is a teaching project. It has no authentication, audit trail, concurrency control or fault recovery.
