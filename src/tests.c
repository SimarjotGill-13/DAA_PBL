#include "common.h"
#include <time.h>

static int passed, failed;
static void check(const char *name, int cond) {
    printf("  [%s] %s\n", cond ? "PASS" : "FAIL", name);
    if (cond) passed++; else failed++;
}
static void section(const char *s) { printf("\n%s\n", s); }
static void fresh(void) { loadBuiltinData(); }
static int same(const Inventory *a) { return memcmp(a, ds.inv, sizeof(Inventory) * (size_t)ds.nInv) == 0; }

static int writeFile(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fputs(text, f);
    fclose(f);
    return 0;
}

int runTests(void) {
    Order o;
    Inventory snap[MAXI];
    int s;
    passed = failed = 0;
    printf("Automated test cases (order output is suppressed)\n");

    section("A. Order workflow");
    fresh();
    s = placeOrder(100001, 101, 2, 0, &o);
    check("T01 valid order: W1 chosen, cost 11 (W3 would cost 16), routed by Dijkstra", s == ORD_OK && o.wh == 1 && o.cost == 11 && o.algo == ALGO_DIJKSTRA);
    check("T01b reserved = 2, total unchanged = 10, sellable = 8", ds.inv[invIndex(101, 1)].reserved == 2 && ds.inv[invIndex(101, 1)].total == 10 && sellable(&ds.inv[invIndex(101, 1)]) == 8);
    fresh(); memcpy(snap, ds.inv, sizeof(Inventory) * (size_t)ds.nInv);
    s = placeOrder(999999, 101, 1, 0, &o);
    check("T02 unserviceable PIN rejected, stock untouched", s == ORD_BAD_PIN && same(snap));
    s = placeOrder(100001, 999, 1, 0, &o);
    check("T03 invalid product ID rejected", s == ORD_BAD_PRODUCT && same(snap));
    s = placeOrder(100001, 102, 50, 0, &o);
    check("T04 insufficient sellable stock rejected, stock untouched", s == ORD_INSUFFICIENT && same(snap));
    fresh();
    s = placeOrder(100001, 103, 5, 0, &o);
    check("T05 damaged units excluded: W1 sellable 4 < 5, so W2 chosen (cost 13)", sellable(&ds.inv[invIndex(103, 1)]) == 4 && s == ORD_OK && o.wh == 2 && o.cost == 13);
    fresh();
    ds.inv[invIndex(102, 2)].total = 0;
    s = placeOrder(100001, 102, 1, 0, &o);
    check("T06a first customer gets the last unit (W1)", s == ORD_OK && o.wh == 1);
    s = placeOrder(100002, 102, 1, 0, &o);
    check("T06b second customer rejected, no double reservation", s == ORD_INSUFFICIENT && ds.inv[invIndex(102, 1)].reserved == 1);
    fresh();
    s = placeOrder(100002, 101, 3, 0, &o);
    check("T07 nearest warehouse W3 (distance 8) has stock and wins, cost 12", s == ORD_OK && o.wh == 3 && o.cost == 12);
    fresh();
    s = placeOrder(100002, 101, 8, 0, &o);
    check("T08 nearest W3 has only 5 < 8, alternative W1 chosen, cost 17", s == ORD_OK && o.wh == 1 && o.cost == 17);
    fresh();
    s = placeOrder(100001, 103, 1, 0, &o);
    check("T09 different costs W1 11 < W2 13 < W3 16 -> W1", s == ORD_OK && o.wh == 1 && o.cost == 11);
    fresh(); ds.warehouses[1].fee = 0;
    s = placeOrder(100001, 103, 1, 0, &o);
    check("T10 equal cost (11 vs 11): shorter distance wins -> W1", s == ORD_OK && o.wh == 1 && o.cost == 11);
    fresh(); memcpy(snap, ds.inv, sizeof(Inventory) * (size_t)ds.nInv);
    s = placeOrder(100003, 101, 1, 0, &o);
    check("T11 unreachable destination rejected, stock untouched", s == ORD_UNREACHABLE && same(snap));
    s = recommendTransfers(0);
    check("T12 redistribution gives 2 recommendations and does not change stock", s == 2 && same(snap));

    section("B. Invalid inputs, empty and boundary cases");
    fresh();
    check("T13 quantity 0 rejected", placeOrder(100001, 101, 0, 0, &o) == ORD_BAD_QTY);
    check("T14 negative quantity rejected", placeOrder(100001, 101, -4, 0, &o) == ORD_BAD_QTY);
    check("T15 PIN 0 and negative PIN rejected", placeOrder(0, 101, 1, 0, &o) == ORD_BAD_PIN && placeOrder(-7, 101, 1, 0, &o) == ORD_BAD_PIN);
    check("T16 product ID 0 rejected", placeOrder(100001, 0, 1, 0, &o) == ORD_BAD_PRODUCT);
    check("T17 huge quantity INT_MAX handled without overflow", placeOrder(100001, 101, INT_MAX, 0, &o) == ORD_INSUFFICIENT);
    fresh();
    check("T18 boundary: quantity equal to the largest sellable stock (10) succeeds", placeOrder(100001, 101, 10, 0, &o) == ORD_OK && o.wh == 1);
    fresh();
    check("T19 boundary: quantity 11 when total is 15 but no warehouse holds 11 -> no single warehouse", placeOrder(100001, 101, 11, 0, &o) == ORD_NO_SINGLE_WH);
    fresh();
    for (int i = 0; i < ds.nInv; i++) ds.inv[i].total = 0;
    check("T20 empty inventory everywhere -> insufficient", placeOrder(100001, 101, 1, 0, &o) == ORD_INSUFFICIENT);
    fresh();
    check("T21 empty order log at start", orderCount == 0);
    check("T22 damage: unknown record, quantity 0 and quantity too large are rejected",
          recordDamage(103, 9, 1) == 1 && recordDamage(103, 1, 0) == 2 && recordDamage(103, 1, 5) == 3);
    check("T23 damage: 1 unit accepted, sellable 4 -> 3", recordDamage(103, 1, 1) == 0 && sellable(&ds.inv[invIndex(103, 1)]) == 3);
    fresh();
    placeOrder(100001, 101, 10, 0, &o);
    check("T24 reserved units cannot be marked damaged", recordDamage(101, 1, 1) == 3);

    section("C. Input parsing (validation helper)");
    long v = 0;
    check("T25 '42' accepted", parseIntStrict("42", 0, 100, &v) && v == 42);
    check("T26 ' 7 ' with spaces accepted", parseIntStrict("  7  \n", 0, 100, &v) && v == 7);
    check("T27 '42abc' rejected (trailing characters)", !parseIntStrict("42abc", 0, 100, &v));
    check("T28 empty and blank text rejected", !parseIntStrict("", 0, 100, &v) && !parseIntStrict("   \n", 0, 100, &v));
    check("T29 '3.5' rejected", !parseIntStrict("3.5", 0, 100, &v));
    check("T30 out-of-range 101 and -1 rejected", !parseIntStrict("101", 0, 100, &v) && !parseIntStrict("-1", 0, 100, &v));
    check("T31 overflow 99999999999999999999 rejected", !parseIntStrict("99999999999999999999", LONG_MIN, LONG_MAX, &v));
    double dv;
    check("T32 decimal parsing accepts 2.5 and rejects '2.5x'", parseDoubleStrict("2.5", 0, 10, &dv) && dv == 2.5 && !parseDoubleStrict("2.5x", 0, 10, &dv));

    section("D. Sorting and searching");
    int e0[1], one[1] = {5}, dup[6] = {3, 1, 3, 2, 1, 3}, rev[6] = {6, 5, 4, 3, 2, 1}, q[6];
    check("T33 Merge Sort handles empty and single element", mergeSort(e0, 0, sizeof(int), cmpInt) == 0 && mergeSort(one, 1, sizeof(int), cmpInt) == 0 && one[0] == 5);
    mergeSort(dup, 6, sizeof(int), cmpInt);
    check("T34 Merge Sort with duplicates", dup[0] == 1 && dup[1] == 1 && dup[2] == 2 && dup[3] == 3 && dup[5] == 3);
    memcpy(q, rev, sizeof q); quickSort(q, 6, sizeof(int), cmpInt); mergeSort(rev, 6, sizeof(int), cmpInt);
    check("T35 Quick Sort and Merge Sort agree on reversed input", memcmp(q, rev, sizeof q) == 0 && rev[0] == 1 && rev[5] == 6);
    check("T36 invalid arguments rejected (n < 0, NULL comparator)", mergeSort(q, -1, sizeof(int), cmpInt) == -1 && mergeSort(q, 3, sizeof(int), NULL) == -1);
    int ext[2] = {INT_MIN, INT_MAX};
    mergeSort(ext, 2, sizeof(int), cmpInt);
    check("T37 comparison is overflow-safe (INT_MIN vs INT_MAX)", cmpInt(&ext[0], &ext[1]) < 0 && ext[0] == INT_MIN);
    typedef struct { int key, tag; } KT;
    KT kt[6] = {{2, 0}, {1, 1}, {2, 2}, {1, 3}, {2, 4}, {1, 5}};
    int stable = 1;
    mergeSort(kt, 6, sizeof(KT), cmpInt);
    for (int i = 1; i < 6; i++) if (kt[i].key == kt[i - 1].key && kt[i].tag < kt[i - 1].tag) stable = 0;
    check("T38 Merge Sort is stable (equal keys keep their order)", stable);
    fresh();
    int cm;
    check("T39 binary search: first, last and missing PIN", findPinNode(100001, &cm) == 6 && findPinNode(100003, &cm) == 8 && findPinNode(100004, &cm) == -1);
    check("T40 linear and binary search agree for every PIN", findPinNode(100002, &cm) == findPinNodeLinear(100002, &cm));
    check("T41 product binary search matches linear search", findProductBinary(102) == findProductLinear(102) && findProductBinary(5) == -1);

    section("E. Routing: Dijkstra, Bellman-Ford, Floyd-Warshall, negative weights");
    fresh();
    int agree = 1, dd[MAXV], dp[MAXV], bf[MAXV], bp[MAXV], fw[MAXV][MAXV];
    floydWarshall(&ds.roads, fw);
    for (int src = 0; src < ds.nNodes; src++) {
        dijkstra(&ds.roads, src, dd, dp);
        bellmanFord(&ds.roads, src, bf, bp);
        for (int i = 0; i < ds.nNodes; i++) if (dd[i] != bf[i] || dd[i] != fw[src][i]) agree = 0;
    }
    check("T42 all three algorithms agree from every source node", agree);
    dijkstra(&ds.roads, 6, dd, dp);
    check("T43 CustX distances: W1 6, W2 11, W3 12, CustZ unreachable", dd[0] == 6 && dd[1] == 11 && dd[2] == 12 && dd[8] >= INF);
    RouteAlgo used;
    shortestPaths(&ds.roads, 6, dd, dp, &used);
    check("T44 non-negative roads -> Dijkstra is selected", used == ALGO_DIJKSTRA);
    Graph g;
    graphInit(&g, 4);
    graphAddArc(&g, 0, 1, 4); graphAddArc(&g, 0, 2, 5); graphAddArc(&g, 2, 1, -3); graphAddArc(&g, 1, 3, 2);
    dijkstra(&g, 0, dd, dp);
    int neg = bellmanFord(&g, 0, bf, bp);
    check("T45 negative arc: Dijkstra is WRONG (B = 4), Bellman-Ford correct (B = 2, D = 4)", dd[1] == 4 && !neg && bf[1] == 2 && bf[3] == 4);
    shortestPaths(&g, 0, dd, dp, &used);
    check("T46 negative arc -> Bellman-Ford is selected automatically and is correct", used == ALGO_BELLMAN_FORD && dd[1] == 2 && dd[3] == 4);
    check("T47 Floyd-Warshall also handles the negative arc", floydWarshall(&g, fw) == 0 && fw[0][1] == 2 && fw[0][3] == 4);
    check("T48 negative cycle rejected when adding an arc, graph unchanged", graphAddArc(&g, 3, 2, -5) == 3 && g.w[3][2] == NOEDGE);
    g.w[3][2] = -5;
    check("T49 negative cycle detected by Bellman-Ford, Floyd-Warshall and the cycle check",
          bellmanFord(&g, 0, bf, bp) == 1 && floydWarshall(&g, fw) == 1 && graphHasNegativeCycle(&g) == 1);
    g.w[3][2] = NOEDGE;
    check("T50 negative TWO-WAY road rejected (code 4); bad node 1; bad weight 2",
          graphAddRoad(&g, 0, 3, -1) == 4 && graphAddArc(&g, 0, 9, 1) == 1 && graphAddArc(&g, 0, 3, 20000) == 2);
    check("T51 a road of length 0 is representable and routes correctly", graphAddRoad(&g, 0, 3, 0) == 0 && (dijkstra(&g, 0, dd, dp), dd[3] == 0));
    fresh();
    int hc = nodeByName("HubC"), ha = nodeByName("HubA");
    graphAddArc(&ds.roads, hc, ha, -1);
    s = placeOrder(100001, 101, 2, 0, &o);
    check("T52 order workflow switches to Bellman-Ford with a rebate road and picks W3 (cost 10)", s == ORD_OK && o.algo == ALGO_BELLMAN_FORD && o.wh == 3 && o.cost == 10);
    fresh();
    graphAddArc(&ds.roads, ha, hc, 1);                    /* a one-way road in the other direction only */
    check("T53 one-way roads: order still succeeds and uses the reversed graph", placeOrder(100001, 101, 2, 0, &o) == ORD_OK);
    Csr c;
    int ok = (csrBuildRandom(&c, 300, 900, 5, 20) == 0);
    int *d1 = malloc(sizeof(int) * 300), *d2 = malloc(sizeof(int) * 300);
    if (ok && d1 && d2) { dijkstraCsrArray(&c, 0, d1); dijkstraCsrHeap(&c, 0, d2); ok = memcmp(d1, d2, sizeof(int) * 300) == 0; } else ok = 0;
    check("T54 heap Dijkstra equals array Dijkstra on a random 300-node graph", ok);
    free(d1); free(d2); csrFree(&c);

    section("F. Scheduling and knapsack");
    Delivery dl[8] = {{1, 540, 630}, {2, 570, 660}, {3, 630, 720}, {4, 600, 690}, {5, 660, 750}, {6, 720, 780}, {7, 540, 600}, {8, 690, 810}};
    Delivery ch[MAXD], so[MAXD];
    int ag[MAXD];
    check("T55 Activity Selection on the sample picks 3; Interval Partitioning needs 3 agents", activitySelection(dl, 8, ch) == 3 && intervalPartition(dl, 8, so, ag) == 3);
    check("T56 empty schedule: 0 selected, 0 agents", activitySelection(dl, 0, ch) == 0 && intervalPartition(dl, 0, so, ag) == 0);
    Delivery same3[3] = {{1, 600, 660}, {2, 600, 660}, {3, 600, 660}};
    check("T57 three identical intervals: 1 selected, 3 agents", activitySelection(same3, 3, ch) == 1 && intervalPartition(same3, 3, so, ag) == 3);
    Delivery touch[3] = {{1, 600, 660}, {2, 660, 720}, {3, 720, 780}};
    check("T58 touching intervals are compatible: 3 selected, 1 agent", activitySelection(touch, 3, ch) == 3 && intervalPartition(touch, 3, so, ag) == 1);
    check("T59 too many deliveries rejected", activitySelection(dl, MAXD + 1, ch) == -1);
    Item it[4] = {{"P1", 10, 60}, {"P2", 20, 100}, {"P3", 30, 120}, {"P4", 15, 75}};
    char names[32];
    long calls, ex;
    int dpv = knapDP(it, 4, 50, names, sizeof names);
    check("T60 0/1 DP, backtracking and all three branch-and-bound modes give 235",
          dpv == 235 && knapBacktrack(it, 4, 50, &calls) == 235 && knapBB(it, 4, 50, 0, &ex) == 235 && knapBB(it, 4, 50, 1, &ex) == 235 && knapBB(it, 4, 50, 2, &ex) == 235);
    check("T61 fractional gives 255 (higher, but invalid for indivisible parcels)", knapFractional(it, 4, 50) == 255.0);
    int brute = 0;
    for (int m = 0; m < 16; m++) { int w = 0, val = 0; for (int i = 0; i < 4; i++) if (m & (1 << i)) { w += it[i].w; val += it[i].v; } if (w <= 50 && val > brute) brute = val; }
    check("T62 DP equals an independent brute-force over all 16 subsets", dpv == brute);
    check("T63 knapsack rejects invalid input (n = 0, capacity too large)", knapDP(it, 0, 50, names, sizeof names) == -1 && knapDP(it, 4, MAXCAP + 1, names, sizeof names) == -1);

    section("G. Algorithm cores (Divide and Conquer, Greedy, DP, Backtracking, Flow, Strings, Numbers)");
    int a16[16] = {13, -3, -25, 20, -3, -16, -23, 18, 20, -7, 12, -5, -22, 15, -4, 7}, lo, hi;
    check("T64 Maximum Subarray = 43 over days 7..10", maxSubarray(a16, 16, &lo, &hi) == 43 && lo == 7 && hi == 10);
    int allNeg[3] = {-5, -2, -9};
    check("T65 Maximum Subarray with all negatives = -2; n = 0 rejected", maxSubarray(allNeg, 3, &lo, &hi) == -2 && maxSubarray(allNeg, 0, &lo, &hi) == INT_MIN);
    long long slow = 1;
    for (int i = 0; i < 128; i++) slow = (slow * 7) % 13;
    check("T66 powering: 2^10 = 1024, 7^128 mod 13 matches repeated multiplication", powerDC(2, 10) == 1024 && powerDC(5, 0) == 1 && modPowerDC(7, 128, 13) == slow);
    int K[16] = {0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0}, K2[9] = {0, 1, 0, 1, 0, 1, 0, 1, 0};
    check("T67 Celebrity: person 2 found; no celebrity in a mutual-acquaintance group", celebrityFind(K, 4) == 2 && celebrityFind(K2, 3) == -1);
    Mat A, B, C1, C2;
    int strOk = 1;
    for (int n = 2; n <= 8; n *= 2) {
        long ms = 0, mt = 0;
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) { A[i][j] = (i * 5 + j * 3) % 9 - 4; B[i][j] = (i * 2 + j * 7) % 11 - 5; }
        strassenMultiply(n, A, B, C1, &ms); standardMultiply(n, A, B, C2, &mt);
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) if (C1[i][j] != C2[i][j]) strOk = 0;
        if (n == 4 && !(ms == 49 && mt == 64)) strOk = 0;
        if (n == 8 && !(ms == 343 && mt == 512)) strOk = 0;
    }
    check("T68 Strassen equals standard multiplication for n = 2, 4, 8 (49 vs 64 and 343 vs 512 multiplications)", strOk);
    check("T69 Strassen rejects a size that is not a power of two", strassenMultiply(3, A, B, C1, NULL) == -1);
    fresh();
    int hb = huffmanBits("ORDER SHIPPED ORDER DELIVERED", 0);
    check("T70 Huffman uses fewer bits than 8-bit coding; empty text rejected", hb > 0 && hb < 8 * 29 && huffmanBits("", 0) == -1);
    check("T71 Prim and Kruskal give the same minimum spanning forest weight (25)", mstPrim(&ds.roads, 0) == 25 && mstKruskal(&ds.roads, 0) == 25);
    check("T72 MST refuses a directed or negative graph", mstPrim(&g, 0) == -1 && mstKruskal(&g, 0) == -1);
    char out[64];
    check("T73 LCS(SmartPhone, SmartWatch) = 6", lcsLength("SmartPhone", "SmartWatch", out, sizeof out) == 6 && strcmp(out, "Smarth") == 0);
    check("T74 LCS with an empty string = 0", lcsLength("", "abc", out, sizeof out) == 0);
    int p[7] = {30, 35, 15, 5, 10, 20, 25};
    char par[128];
    check("T75 Matrix Chain minimum = 15125 multiplications", matrixChainMin(p, 6, par, sizeof par) == 15125);
    int p1[2] = {4, 7};
    check("T76 Matrix Chain with one matrix = 0; invalid dimension rejected", matrixChainMin(p1, 1, par, sizeof par) == 0 && matrixChainMin((int[]){4, 0, 3}, 2, par, sizeof par) == -1);
    int f3[3] = {34, 8, 50};
    check("T77 Optimal BST cost = 142", obstMin(f3, 3) == 142);
    int stops[5] = {nodeByName("W1"), nodeByName("CustX"), nodeByName("CustY"), nodeByName("HubA"), nodeByName("HubC")}, res[3];
    int tr = tspSolveAll(&ds.roads, stops, 5, 0, res);
    check("T78 TSP: brute force, Held-Karp and Branch and Bound all agree", tr == 0 && res[0] == res[1] && res[1] == res[2] && res[0] > 0);
    int stopsBad[2] = {nodeByName("W1"), nodeByName("CustZ")};
    check("T79 TSP reports an unreachable stop", tspSolveAll(&ds.roads, stopsBad, 2, 0, res) == -2);
    check("T80 N-Queens: 4 -> 2, 6 -> 4, 8 -> 92 solutions; n = 0 rejected", nQueensCount(4, 0) == 2 && nQueensCount(6, 0) == 4 && nQueensCount(8, 0) == 92 && nQueensCount(0, 0) == -1);
    int sw[7] = {10, 7, 5, 18, 12, 20, 15}, bc = 0;
    for (int m = 0; m < 128; m++) { int t = 0; for (int i = 0; i < 7; i++) if (m & (1 << i)) t += sw[i]; if (t == 35) bc++; }
    check("T81 Sum of Subsets count equals an independent brute-force count", subsetSumCount(sw, 7, 35, 0) == bc && bc > 0);
    int cap[FLOWN][FLOWN] = {{0}}, fl[FLOWN][FLOWN];
    cap[0][1] = 10; cap[0][3] = 5; cap[1][4] = 6; cap[1][5] = 3; cap[2][4] = 4; cap[2][5] = 4; cap[3][4] = 2; cap[3][5] = 5; cap[4][6] = 8; cap[5][6] = 7;
    check("T82 Max Flow = 14; source equals sink rejected", maxFlow(7, cap, 0, 6, fl) == 14 && maxFlow(7, cap, 2, 2, fl) == -1);
    int el[MAXM][MAXM] = {{0}}, mr[MAXM];
    el[0][0] = el[0][1] = 1; el[1][0] = 1; el[2][1] = el[2][2] = 1; el[3][2] = 1;
    check("T83 Bipartite matching size = 3", bipartiteMatching(4, 3, el, mr) == 3);
    const char *T = "Phone,Charger,Earbuds,Phone,Earbuds,Charger,Earbuds", *P = "Earbuds";
    long c1, c2;
    int sp;
    check("T84 Naive, Rabin-Karp, Automaton and KMP all find 3 matches",
          naiveSearch(T, P, 0, &c1) == 3 && rabinKarpSearch(T, P, 0, &c2, &sp) == 3 && automatonSearch(T, P, 0) == 3 && kmpSearch(T, P, 0, &c2) == 3);
    check("T85 overlapping matches: 'aaaa' in 'aaaaaa' = 3 for all four", naiveSearch("aaaaaa", "aaaa", 0, &c1) == 3 && rabinKarpSearch("aaaaaa", "aaaa", 0, &c1, &sp) == 3 && automatonSearch("aaaaaa", "aaaa", 0) == 3 && kmpSearch("aaaaaa", "aaaa", 0, &c1) == 3);
    check("T86 no match = 0; pattern longer than text = 0; empty pattern rejected", kmpSearch(T, "xyz", 0, &c1) == 0 && naiveSearch("ab", "abc", 0, &c1) == 0 && kmpSearch(T, "", 0, &c1) == -1);
    long r3[3] = {2, 3, 2}, m3[3] = {3, 5, 7}, x, M;
    check("T87 CRT: x = 23 (mod 105) and it satisfies every congruence", crtSolve(r3, m3, 3, &x, &M) == 0 && x == 23 && M == 105 && x % 3 == 2 && x % 5 == 3 && x % 7 == 2);
    long mb[2] = {4, 6};
    check("T88 CRT rejects moduli that are not coprime, and a modulus below 2", crtSolve(r3, mb, 2, &x, &M) == 1 && crtSolve(r3, (long[]){1, 5}, 2, &x, &M) == 1);

    section("H. Recurrences");
    double e;
    check("T89 Master Theorem cases: merge sort case 2, Strassen case 1 (2.807), T(n)=T(n/2)+n case 3",
          masterCase(2, 2, 1, &e) == 2 && masterCase(7, 2, 2, &e) == 1 && e > 2.80 && e < 2.81 && masterCase(1, 2, 1, &e) == 3);
    check("T90 Master Theorem rejects invalid recurrences (b = 1, a = 0)", masterCase(2, 1, 1, &e) == 0 && masterCase(0, 2, 1, &e) == 0);

    section("I. Dataset loading");
    fresh();
    const char *good = "nodes 4\nnode 0 WA\nnode 1 H1\nnode 2 CA\nnode 3 CB\nroad 0 1 3\nroad 1 2 2\nroad 1 3 4\nproduct 7 Widget\nwarehouse 5 WA 0 1\n"
                       "inventory 7 5 9 1 2\npin 123456 2\npin 123457 3\ndemand 7 5 3\n";
    writeFile("optimizer_test_dataset.tmp", good);
    char err[160];
    check("T91 valid dataset file loads", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) == 0 && ds.nNodes == 4 && ds.nProducts == 1);
    s = placeOrder(123456, 7, 3, 0, &o);
    check("T92 orders work on the loaded data (cost 5 + fee 1 = 6)", s == ORD_OK && o.wh == 5 && o.cost == 6);
    check("T93 loaded stock numbers kept (total 9, damaged 1, reserved 2 + 3)", ds.inv[0].total == 9 && ds.inv[0].damaged == 1 && ds.inv[0].reserved == 5);
    Dataset before = ds;
    writeFile("optimizer_test_dataset.tmp", "nodes 2\nbogus 1 2\n");
    check("T94 unknown keyword rejected and the current data is kept", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) != 0 && memcmp(&before, &ds, sizeof ds) == 0);
    check("T95 missing file reported", loadDatasetFile("no_such_file.txt", err, sizeof err) != 0);
    writeFile("optimizer_test_dataset.tmp", "nodes 3\nproduct 1 A\nproduct 1 B\nwarehouse 1 W 0 1\ninventory 1 1 5 0 0\npin 123456 1\n");
    check("T96 duplicate product id rejected", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) != 0);
    writeFile("optimizer_test_dataset.tmp", "nodes 3\nproduct 1 A\nwarehouse 1 W 0 1\ninventory 1 1 5 4 3\npin 123456 1\n");
    check("T97 damaged + reserved > total rejected", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) != 0);
    writeFile("optimizer_test_dataset.tmp", "nodes 3\narc 0 1 2\narc 1 2 -3\narc 2 0 -1\nproduct 1 A\nwarehouse 1 W 0 1\ninventory 1 1 5 0 0\npin 123456 1\n");
    check("T98 a negative cycle in the file is rejected", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) != 0);
    writeFile("optimizer_test_dataset.tmp", "nodes 3\nroad 0 1 -2\n");
    check("T99 a negative two-way road in the file is rejected", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) != 0);
    writeFile("optimizer_test_dataset.tmp", "nodes 3\nnode 0 W\nnode 1 H\nnode 2 C\narc 0 1 4\narc 1 2 5\narc 0 2 20\narc 2 1 -2\nproduct 1 A\nwarehouse 1 W 0 1\ninventory 1 1 5 0 0\npin 123456 2\n");
    check("T100 a one-way negative arc loads and the order uses Bellman-Ford", loadDatasetFile("optimizer_test_dataset.tmp", err, sizeof err) == 0 && placeOrder(123456, 1, 1, 0, &o) == ORD_OK && o.algo == ALGO_BELLMAN_FORD && o.dist == 9);
    remove("optimizer_test_dataset.tmp");
    {
        FILE *fp = fopen("data/sample_dataset.txt", "r");
        if (fp) {
            fclose(fp);
            Dataset ref;
            fresh(); ref = ds;
            check("T105 data/sample_dataset.txt loads and equals the built-in dataset", loadDatasetFile("data/sample_dataset.txt", err, sizeof err) == 0 && memcmp(&ref, &ds, sizeof ds) == 0);
            check("T106 data/extended_dataset.txt: order succeeds (W1, cost 9); isolated area A6 is unreachable",
                  loadDatasetFile("data/extended_dataset.txt", err, sizeof err) == 0 && placeOrder(560001, 202, 5, 0, &o) == ORD_OK && o.wh == 1 && o.cost == 9 &&
                  placeOrder(560006, 202, 1, 0, &o) == ORD_UNREACHABLE);
            check("T107 data/negative_roads_dataset.txt: Bellman-Ford chosen, W3 cost 10",
                  loadDatasetFile("data/negative_roads_dataset.txt", err, sizeof err) == 0 && placeOrder(100001, 101, 2, 0, &o) == ORD_OK && o.algo == ALGO_BELLMAN_FORD && o.wh == 3 && o.cost == 10);
        } else printf("  [SKIP] T105-T107 data files not found (run the tests from the project root)\n");
    }

    section("J. Large-data and stress cases");
    fresh();
    int N = 20000, *ra = malloc(sizeof(int) * (size_t)N), *rb = malloc(sizeof(int) * (size_t)N);
    int sortOk = (ra && rb);
    unsigned seed = 3;
    for (int i = 0; sortOk && i < N; i++) { seed = seed * 1664525u + 1013904223u; ra[i] = (int)(seed >> 8) % 100000; rb[i] = ra[i]; }
    if (sortOk) { sortOk = mergeSort(ra, N, sizeof(int), cmpInt) == 0 && quickSort(rb, N, sizeof(int), cmpInt) == 0 && memcmp(ra, rb, sizeof(int) * (size_t)N) == 0; for (int i = 1; i < N; i++) if (ra[i - 1] > ra[i]) sortOk = 0; }
    check("T101 sorting 20000 random integers: Merge Sort and Quick Sort agree and are sorted", sortOk);
    free(ra); free(rb);
    Csr big;
    int bigOk = (csrBuildRandom(&big, 3000, 15000, 9, 30) == 0);
    int *x1 = malloc(sizeof(int) * 3000), *x2 = malloc(sizeof(int) * 3000);
    if (bigOk && x1 && x2) { dijkstraCsrArray(&big, 0, x1); dijkstraCsrHeap(&big, 0, x2); bigOk = memcmp(x1, x2, sizeof(int) * 3000) == 0; } else bigOk = 0;
    check("T102 3000-node, 15000-road graph: array and heap Dijkstra give identical distances", bigOk);
    free(x1); free(x2); csrFree(&big);
    check("T103 invalid graph sizes rejected by the generator", csrBuildRandom(&big, 2, 5, 1, 10) == -1 && csrBuildRandom(&big, 10, 5, 1, 10) == -1);
    fresh();
    int made = 0;
    for (int i = 0; i < MAXORD + 20; i++) { placeOrder(100001, 103, 1, 0, &o); made++; }
    check("T104 order log never overflows (capacity respected)", made == MAXORD + 20 && orderCount <= MAXORD);

    fresh();
    printf("\nResult: %d passed, %d failed\n", passed, failed);
    return failed;
}

/* ---------------- demonstration scenarios ---------------- */

void runScenarios(void) {
    Order o;
    printf("=== Scenario 1: normal order fulfilment ===\n");
    loadBuiltinData();
    placeOrder(100001, 101, 2, 0, &o);
    placeOrder(100002, 102, 1, 0, &o);
    placeOrder(100001, 103, 3, 0, &o);
    placeOrder(100002, 101, 8, 0, &o);
    printOrders();
    printf("Every order went to the cheapest eligible warehouse; stock was reserved only after allocation.\n");

    printf("\n=== Scenario 2: inventory and road constraint shocks (same order before and after) ===\n");
    loadBuiltinData();
    placeOrder(100001, 101, 2, 0, &o);
    printf("Baseline: PIN 100001, product 101, qty 2 -> W%d, cost %d, distance %d\n", o.wh, o.cost, o.dist);
    loadBuiltinData();
    printf("Shock A: W1 inspection finds 9 of its 10 Phones damaged.\n");
    recordDamage(101, 1, 9);
    int s = placeOrder(100001, 101, 2, 0, &o);
    printf("  -> %s", s == ORD_OK ? "" : statusText(s));
    if (s == ORD_OK) printf("W%d, cost %d (W1 now has 1 sellable, so the order moves to the next cheapest eligible warehouse)\n", o.wh, o.cost);
    else printf("\n");
    loadBuiltinData();
    int ha = nodeByName("HubA"), cx = nodeByName("CustX");
    ds.roads.w[ha][cx] = NOEDGE; ds.roads.w[cx][ha] = NOEDGE;
    printf("Shock B: the road HubA-CustX is closed.\n");
    s = placeOrder(100001, 101, 2, 0, &o);
    printf("  -> %s", s == ORD_OK ? "" : statusText(s));
    if (s == ORD_OK) printf("W%d, cost %d, distance %d (W1 is now farther away, so the cheapest eligible warehouse changes)\n", o.wh, o.cost, o.dist);
    else printf("\n");
    loadBuiltinData();
    for (int i = 0; i < ds.nInv; i++) ds.inv[i].damaged = ds.inv[i].total;
    s = placeOrder(100001, 101, 1, 0, &o);
    printf("Shock C: every warehouse reports all stock damaged -> %s\n", statusText(s));
    loadBuiltinData();

    printf("\n=== Scenario 3: large-data stress ===\n");
    demoStress();
}
