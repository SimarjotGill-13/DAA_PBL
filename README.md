# E-Commerce Order Fulfilment and Delivery Optimizer

A modular, menu-driven C17 application for a Design and Analysis of Algorithms project. It simulates e-commerce order fulfilment: PIN-code serviceability, multi-warehouse inventory, overselling prevention, cost-based warehouse selection with **Dijkstra's shortest-path algorithm**, delivery scheduling, vehicle loading and stock-redistribution advice. Algorithms from the full DAA syllabus are included as demonstrations and comparisons.

- Core algorithm: **Dijkstra**. When a road has a negative net cost (a rebate), the program switches to **Bellman-Ford** automatically.
- No machine learning, no chatbot, no network access. All data is deterministic sample data, so everything works offline.

## Build and run

Requires a C17 compiler (gcc or clang), `make` and the standard maths library.

```bash
make            # builds ./order_optimizer with -std=c17 -Wall -Wextra -Wpedantic
./order_optimizer
make test       # runs the automated tests (exit code 0 means all passed)
make clean
```

Run these from the project root. Without make:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 -Iinclude -o order_optimizer src/*.c -lm
```

Command-line options: `./order_optimizer --test` runs the tests; `./order_optimizer --scenarios` runs the three demonstration scenarios.

## How an order is processed

1. Validate the quantity.
2. Check the PIN code is serviceable (binary search on the sorted PIN list).
3. Look up the product (binary search).
4. Compute sellable stock per warehouse: `sellable = total - damaged - reserved`. It is always computed and never stored, so nothing is subtracted twice.
5. Keep only warehouses that hold the full quantity.
6. Find the shortest distance from every warehouse to the customer with one run of Dijkstra (or Bellman-Ford if a road is negative).
7. Cost = `distance x RATE + warehouse handling fee`.
8. Choose the lowest cost. Tie-break: shorter distance, then lower warehouse ID.
9. Reserve stock only after a warehouse is chosen (first come, first served).
10. Print the warehouse, route, cost and updated stock, or a clear failure reason.

Example (menu 1, PIN 100001, product 101, quantity 2): W1 is selected at cost 11, route `CustX -> HubA -> W1`. W3 would cost 16, and W2 has no stock.

Failure reasons: invalid quantity, unserviceable PIN, invalid product, insufficient stock, no single warehouse with enough stock, unreachable destination, negative cycle in the road network.

## Menu

| # | Option |
|---|---|
| 1 | Place order |
| 2 | View inventory |
| 3 | Record damaged stock |
| 4 | View order log |
| 5 | Redistribution advice (recommendations only; nothing is executed) |
| 6 | Compare routing algorithms (Dijkstra, Bellman-Ford, Floyd-Warshall) |
| 7 | Add or change a road (validated; negative roads must be one-way) |
| 8 | Negative-weight demo (why Bellman-Ford is needed) |
| 9 | Delivery scheduling (Activity Selection, Interval Partitioning) |
| 10 | Vehicle load planning (Knapsack variants) |
| 11 | Load a dataset from a file |
| 12 | Reset to the built-in dataset |
| 13 | Supplementary algorithm demos (16 algorithms) |
| 14 | Complexity and analysis (benchmarks, recurrences, Master Theorem solver, theory notes) |
| 15 | Run the automated tests |
| 16 | Demonstration scenarios (normal, constraint shock, large-data stress) |

Built-in sample PINs: `100001` (CustX), `100002` (CustY), `100003` (CustZ, no road, unreachable). Products: `101`, `102`, `103`.

## Datasets

| File | Contents |
|---|---|
| (built in) | 9 nodes, 3 warehouses, 3 products. Small enough to trace by hand |
| `data/sample_dataset.txt` | the same data as a file (shows the file format) |
| `data/extended_dataset.txt` | 13 nodes, 4 warehouses, 5 products, 6 customer areas, one-way roads, an isolated area |
| `data/negative_roads_dataset.txt` | the built-in roads plus one one-way road with a rebate, so routing uses Bellman-Ford |

Load one with menu 11 (for example `data/extended_dataset.txt`). A file is checked completely before it replaces the current data. A file with an error is rejected and the current data is kept. The file format is documented by comments in `data/sample_dataset.txt`.

## Project structure

```text
DAA_Order_Optimizer/
├── README.md
├── Makefile
├── include/
│   └── common.h                shared constants, structs, status codes, prototypes
├── src/
│   ├── main.c                  menu and validated input
│   ├── data.c                  built-in dataset, dataset validation and file loader
│   ├── inventory.c             sellable stock, Merge/Quick Sort, Binary/Linear Search, damage recording
│   ├── graph.c                 Dijkstra, Bellman-Ford, Floyd-Warshall, negative-weight demo, heap Dijkstra
│   ├── fulfil.c                order placement and warehouse selection
│   ├── schedule.c              Activity Selection, Interval Partitioning, Knapsack variants
│   ├── redistribute.c          stock-transfer recommendations
│   ├── demos_greedy_dc.c       Max Subarray, Powering, Celebrity, Strassen, Huffman, Prim, Kruskal
│   ├── demos_dp_bt.c           LCS, Matrix Chain, Optimal BST, TSP, N-Queens, Sum of Subsets
│   ├── demos_flow_string.c     Max Flow, Matching, four string matchers, Chinese Remainder Theorem
│   ├── complexity.c            benchmarks, recurrences, Master Theorem, theory notes
│   └── tests.c                 automated tests and demonstration scenarios
├── data/                       datasets that can be loaded from the menu
└── docs/
    ├── ALGORITHMS.md           every algorithm: principle, pseudocode, complexity, source function
    ├── COMPLEXITY.md           complexity tables, recurrences, measured results
    ├── SYLLABUS_COVERAGE.md    every syllabus topic mapped to a source function
    ├── TEST_CASES.md           test categories and actual recorded results
    └── LIMITATIONS.md          scope, assumptions and known limits
```

## Algorithm choice at each step

| Task | Algorithm | Why |
|---|---|---|
| PIN / product / inventory lookup | Binary Search | data sorted once, O(log n) per query |
| Sorting | Merge Sort | stable, O(n log n) in every case (Quick Sort compared) |
| Shortest distance to customer | Dijkstra, or Bellman-Ford if a road is negative | Dijkstra is faster; only Bellman-Ford is correct with negative weights |
| Cheapest warehouse | linear minimum scan | only the minimum is needed |
| One agent's deliveries | Activity Selection | earliest finish first is optimal |
| Agents needed for all deliveries | Interval Partitioning | fewest agents = maximum overlap |
| Vehicle loading | 0/1 Knapsack DP | parcels are indivisible, so fractional is invalid |
| Redistribution | greedy, largest deficit first | transparent and explainable |

Full details, with pseudocode and complexity, are in `docs/ALGORITHMS.md`.

## Testing

`make test` runs the automated tests: orders, invalid input, boundaries, damaged stock, overselling, equal-cost tie-breaks, unreachable destinations, negative weights, dataset loading, every algorithm against a known answer or an independent check, and large-data cases. The recorded results are in `docs/TEST_CASES.md`. The tests use the built-in data, so run them before changing the dataset.

## Customising the data

Either load a file from `data/` (menu 11) or edit `src/data.c` and the capacity constants in `include/common.h`, then rebuild. Scheduling, knapsack and the demos keep their own small data inside their source files.

## Limitations

One warehouse fulfils a whole order, distances are static, inventory is in memory only, and processing is single-threaded. See `docs/LIMITATIONS.md` for the full list.
