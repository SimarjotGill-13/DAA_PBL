#include "common.h"
#include <math.h>
#include <time.h>

static long cmpCount;
static int cmpIntCounted(const void *a, const void *b) { cmpCount++; return cmpInt(a, b); }

static double msSince(clock_t t) { return (double)(clock() - t) * 1000.0 / CLOCKS_PER_SEC; }

static unsigned lcgNext(unsigned *s) { *s = *s * 1664525u + 1013904223u; return *s >> 8; }

typedef int (*SortFn)(void *, int, size_t, int (*)(const void *, const void *));

static void runSort(const char *name, SortFn sort, const int *src, int n) {
    int *a = malloc(sizeof(int) * (size_t)n);
    if (!a) { printf("    %-11s skipped: out of memory\n", name); return; }
    memcpy(a, src, sizeof(int) * (size_t)n);
    cmpCount = 0;
    clock_t t = clock();
    int rc = sort(a, n, sizeof(int), cmpIntCounted);
    double ms = msSince(t);
    int ok = (rc == 0);
    for (int i = 1; i < n && ok; i++) if (a[i - 1] > a[i]) ok = 0;
    printf("    %-11s comparisons %10ld  time %8.2f ms  sorted correctly: %s\n", name, cmpCount, ms, ok ? "yes" : "NO");
    free(a);
}

void demoComplexity(void) {
    int n = 4000, *r = malloc(sizeof(int) * (size_t)n), *s = malloc(sizeof(int) * (size_t)n);
    unsigned seed = 42;
    if (!r || !s) { printf("Out of memory.\n"); free(r); free(s); return; }
    for (int i = 0; i < n; i++) { r[i] = (int)(lcgNext(&seed) % 100000); s[i] = i; }
    printf("Sorting %d integers (same input for both algorithms):\n  Random input:\n", n);
    runSort("Merge Sort", mergeSort, r, n); runSort("Quick Sort", quickSort, r, n);
    printf("  Already sorted input (worst case for a last-element pivot):\n");
    runSort("Merge Sort", mergeSort, s, n); runSort("Quick Sort", quickSort, s, n);
    printf("  Merge Sort stays O(n log n); Quick Sort degrades to O(n^2) on sorted input with this pivot.\n");

    int big = 100000, lin = 0, bin = 0, lo = 0, hi = big - 1, key = big - 1;
    for (int i = 0; i < big; i++) { lin++; if (i == key) break; }
    while (lo <= hi) { int mid = lo + (hi - lo) / 2; bin++; if (mid == key) break; if (mid < key) lo = mid + 1; else hi = mid - 1; }
    printf("\nSearching for the last of %d sorted items: Linear %d comparisons, Binary %d comparisons.\n", big, lin, bin);
    printf("Binary Search needs sorted data: sort once (O(n log n)), then every lookup is O(log n).\n");
    free(r); free(s);
}

/* Stress scenario: large inputs, two valid strategies on the same data. Times are measured, not assumed. */
void demoStress(void) {
    int n = 20000, *r = malloc(sizeof(int) * (size_t)n), *s = malloc(sizeof(int) * (size_t)n);
    unsigned seed = 7;
    if (!r || !s) { printf("Out of memory.\n"); free(r); free(s); return; }
    for (int i = 0; i < n; i++) { r[i] = (int)(lcgNext(&seed) % 1000000); s[i] = i; }
    printf("[1] Sorting %d integers\n  Random:\n", n);
    runSort("Merge Sort", mergeSort, r, n); runSort("Quick Sort", quickSort, r, n);
    printf("  Sorted (Quick Sort worst case):\n");
    runSort("Merge Sort", mergeSort, s, n); runSort("Quick Sort", quickSort, s, n);
    free(r); free(s);

    int big = 1000000;
    int *a = malloc(sizeof(int) * (size_t)big);
    if (a) {
        for (int i = 0; i < big; i++) a[i] = i * 2;
        int key = (big - 1) * 2, lin = 0, bin = 0, lo = 0, hi = big - 1;
        for (int i = 0; i < big; i++) { lin++; if (a[i] == key) break; }
        while (lo <= hi) { int mid = lo + (hi - lo) / 2; bin++; if (a[mid] == key) break; if (a[mid] < key) lo = mid + 1; else hi = mid - 1; }
        printf("\n[2] Searching %d sorted items for the last one: Linear %d comparisons, Binary %d comparisons.\n", big, lin, bin);
        free(a);
    }

    Csr g;
    int V = 5000, M = 25000;
    printf("\n[3] Shortest paths on a random road network: %d nodes, %d roads (same graph for both)\n", V, M);
    if (csrBuildRandom(&g, V, M, 11, 20) != 0) { printf("  could not build the graph (out of memory)\n"); return; }
    int *d1 = malloc(sizeof(int) * (size_t)V), *d2 = malloc(sizeof(int) * (size_t)V);
    if (d1 && d2) {
        clock_t t = clock();
        int e1 = dijkstraCsrArray(&g, 0, d1);
        double ms1 = msSince(t);
        t = clock();
        int e2 = dijkstraCsrHeap(&g, 0, d2);
        double ms2 = msSince(t);
        int same = (e1 == 0 && e2 == 0 && memcmp(d1, d2, sizeof(int) * (size_t)V) == 0);
        printf("  Dijkstra, array scan  O(V^2 + E)      : %8.2f ms\n", ms1);
        printf("  Dijkstra, binary heap O((V + E) log V): %8.2f ms\n", ms2);
        printf("  Distances identical: %s\n", same ? "yes" : "NO");
        printf("  Small road networks: the array version is simplest. Large sparse ones: the heap version wins.\n");
    } else printf("  out of memory\n");
    free(d1); free(d2); csrFree(&g);
}

/* ---------------- recurrences ---------------- */

/* T(n) = a T(n/b) + Theta(n^c). Returns the Master Theorem case (1, 2 or 3), or 0 if the input is
   invalid. exponent = log_b(a). For polynomial f(n) the regularity condition of case 3 holds. */
int masterCase(int a, int b, double c, double *exponent) {
    if (a < 1 || b < 2 || c < 0) return 0;
    double e = log((double)a) / log((double)b);
    if (exponent) *exponent = e;
    if (fabs(c - e) < 1e-9) return 2;
    return (c < e) ? 1 : 3;
}

static void describeMaster(int a, int b, double c) {
    double e;
    int k = masterCase(a, b, c, &e);
    if (!k) { printf("  invalid recurrence\n"); return; }
    printf("  T(n) = %dT(n/%d) + n^%g : log_%d(%d) = %.3f -> case %d -> ", a, b, c, b, a, e, k);
    if (k == 1) printf("Theta(n^%.3f)\n", e);
    else if (k == 2) { if (c == 0) printf("Theta(log n)\n"); else printf("Theta(n^%g log n)\n", c); }
    else printf("Theta(n^%g)\n", c);
}

static long long tRec(long long n) { return n <= 1 ? 1 : 2 * tRec(n / 2) + n; }

void demoRecurrences(void) {
    printf("Merge Sort recurrence: T(n) = 2T(n/2) + n, T(1) = 1\n\n");
    printf("1. ITERATION (expand): T(n) = 2T(n/2) + n = 4T(n/4) + 2n = 8T(n/8) + 3n = ...\n"
           "   after k steps: 2^k T(n/2^k) + kn. Stop at n/2^k = 1, so k = log2 n:  T(n) = n + n log2 n.\n");
    printf("   Check against the recurrence evaluated directly:\n   n      T(n) by recursion   n(log2 n + 1)\n");
    for (long long n = 1; n <= 1024; n *= 4) {
        int lg = 0;
        for (long long x = n; x > 1; x /= 2) lg++;
        printf("   %-6lld %-19lld %lld\n", n, tRec(n), n * (lg + 1));
    }
    printf("\n2. RECURSION TREE for n = 16 (cost per level):\n");
    for (int lvl = 0, nodes = 1, sz = 16; sz >= 1; lvl++, nodes *= 2, sz /= 2)
        printf("   level %d: %2d node(s) x cost %2d = %d\n", lvl, nodes, sz, nodes * sz);
    printf("   Every level costs n, and there are log2 n + 1 levels: total n(log2 n + 1).\n");
    printf("\n3. SUBSTITUTION: guess T(n) <= c n log2 n. Then T(n) <= 2c(n/2)log2(n/2) + n = c n log2 n - c n + n,\n"
           "   which is <= c n log2 n whenever c >= 1. Base case T(2) <= c*2*1 holds for c >= 2. So T(n) = O(n log n).\n");
    printf("\n4. MASTER THEOREM, T(n) = aT(n/b) + Theta(n^c); compare c with log_b a:\n");
    describeMaster(1, 2, 0);   /* binary search */
    describeMaster(2, 2, 1);   /* merge sort */
    describeMaster(7, 2, 2);   /* Strassen */
    describeMaster(4, 2, 1);
    describeMaster(1, 2, 1);
    printf("   case 1 (c < log_b a): leaves dominate -> n^(log_b a).  case 2 (c = log_b a): every level equal -> n^c log n.\n"
           "   case 3 (c > log_b a): the root dominates -> n^c.\n");
    printf("   Not covered: recurrences such as T(n) = T(n-1) + n (use the iteration method: n + (n-1) + ... = O(n^2)).\n");
}

void demoMasterTheorem(void) {
    long a, b;
    double c;
    printf("Solve T(n) = a T(n/b) + Theta(n^c)\n");
    if (readIntRange("a (number of subproblems, 1..1000): ", 1, 1000, &a) != 1) return;
    if (readIntRange("b (size divisor, 2..1000): ", 2, 1000, &b) != 1) return;
    if (readDoubleRange("c (exponent of the combine cost, 0..20): ", 0, 20, &c) != 1) return;
    describeMaster((int)a, (int)b, c);
}

void showTheory(void) {
    printf(
"UNIT I  ALGORITHM FOUNDATIONS\n"
"  Characteristics: input, output, definiteness (each step is clear), finiteness (it stops), effectiveness.\n"
"  Analysis: count basic operations as a function of input size n (RAM model); best, average and worst case.\n"
"  Asymptotic notations: O(g): f <= c*g for large n (upper bound).  Omega(g): f >= c*g (lower bound).\n"
"  Theta(g): both (tight).  o and omega are the strict versions.\n"
"  Growth order: 1 < log n < n < n log n < n^2 < n^3 < 2^n < n!\n"
"  Recurrence methods: iteration, substitution, recursion tree, Master Theorem (see the recurrence menu).\n\n"
"DIVIDE AND CONQUER vs GREEDY vs DYNAMIC PROGRAMMING\n"
"  Divide and Conquer: INDEPENDENT subproblems, combine the answers (Merge Sort, Binary Search).\n"
"  Greedy            : one locally best choice at a time, never reconsidered (Dijkstra, Activity Selection).\n"
"  Dynamic Prog.     : OVERLAPPING subproblems solved once and stored (0/1 Knapsack, LCS, Floyd-Warshall).\n\n"
"BRANCH AND BOUND\n"
"  FIFO: live nodes in a queue. LIFO: live nodes in a stack. Least-Cost: always expand the live node with\n"
"  the best bound (priority queue). The same bounding idea prunes branches that cannot beat the best so far.\n\n"
"COMPLEXITY CLASSES (theory only, nothing here is an algorithm)\n"
"  P  : decidable in polynomial time (Dijkstra, sorting, matching).\n"
"  NP : a proposed solution can be CHECKED in polynomial time.\n"
"  NP-complete: in NP and every NP problem reduces to it (decision 0/1 Knapsack, decision TSP, Subset Sum).\n"
"  NP-hard: at least as hard as NP-complete, not necessarily in NP (optimisation TSP).\n"
"  Polynomial-time reducibility: A <=p B means a polynomial-time solver for B gives one for A.\n"
"  Project link: multi-stop routing is TSP (NP-hard), so it is used only for a few stops.\n");
}
