#include "common.h"

/* ---- Longest Common Subsequence (DP): O(mn) time, O(mn) space ---- */
/* Returns the LCS length (-1 if a string is longer than 63) and writes one LCS into out. */
int lcsLength(const char *a, const char *b, char *out, size_t cap) {
    static int L[64][64];
    int m = (int)strlen(a), n = (int)strlen(b);
    if (m > 63 || n > 63) return -1;
    for (int i = 0; i <= m; i++) L[i][0] = 0;
    for (int j = 0; j <= n; j++) L[0][j] = 0;
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            L[i][j] = (a[i - 1] == b[j - 1]) ? L[i - 1][j - 1] + 1 : (L[i - 1][j] > L[i][j - 1] ? L[i - 1][j] : L[i][j - 1]);
    int len = L[m][n];
    if (out && cap > (size_t)len) {
        int k = len, i = m, j = n;
        out[k] = '\0';
        while (i > 0 && j > 0) {
            if (a[i - 1] == b[j - 1]) { out[--k] = a[i - 1]; i--; j--; }
            else if (L[i - 1][j] >= L[i][j - 1]) i--; else j--;
        }
    }
    return len;
}
void demoLCS(void) {
    char out[64];
    int len = lcsLength("SmartPhone", "SmartWatch", out, sizeof out);
    printf("LCS of \"SmartPhone\" and \"SmartWatch\" = \"%s\" (length %d)\n", out, len);
    printf("Use: similarity between product names. O(mn) time and space.\n");
}

/* ---- Matrix Chain Multiplication (DP): O(n^3) time, O(n^2) space ----
   Matrix i has size p[i] x p[i+1]. Returns the minimum number of scalar multiplications, or -1. */
static void appendStr(char *buf, size_t cap, const char *s) {
    size_t l = strlen(buf);
    if (l + strlen(s) + 1 <= cap) strcat(buf, s);
}
static void parenStr(int s[][17], int i, int j, char *buf, size_t cap) {
    if (i == j) { char t[16]; snprintf(t, sizeof t, "A%d", i + 1); appendStr(buf, cap, t); return; }
    appendStr(buf, cap, "(");
    parenStr(s, i, s[i][j], buf, cap);
    appendStr(buf, cap, " x ");
    parenStr(s, s[i][j] + 1, j, buf, cap);
    appendStr(buf, cap, ")");
}
int matrixChainMin(const int p[], int n, char *paren, size_t cap) {
    static long long m[17][17];
    static int s[17][17];
    if (n < 1 || n > 16) return -1;
    for (int i = 0; i <= n; i++) if (p[i] < 1 || p[i] > 100) return -1;
    for (int i = 0; i < n; i++) m[i][i] = 0;
    for (int len = 2; len <= n; len++)
        for (int i = 0; i + len - 1 < n; i++) {
            int j = i + len - 1;
            m[i][j] = LLONG_MAX;
            for (int k = i; k < j; k++) {
                long long c = m[i][k] + m[k + 1][j] + (long long)p[i] * p[k + 1] * p[j + 1];
                if (c < m[i][j]) { m[i][j] = c; s[i][j] = k; }
            }
        }
    if (paren && cap) { paren[0] = '\0'; parenStr(s, 0, n - 1, paren, cap); }
    return (int)m[0][n - 1];
}
void demoMatrixChain(void) {
    int p[7] = {30, 35, 15, 5, 10, 20, 25};
    char par[128];
    int c = matrixChainMin(p, 6, par, sizeof par);
    printf("Matrices A1..A6 with sizes 30x35, 35x15, 15x5, 5x10, 10x20, 20x25\n");
    printf("Minimum scalar multiplications: %d\nBest order: %s\n", c, par);
    printf("Parenthesisation does not change the product, only the work. O(n^3) time, O(n^2) space.\n");
}

/* ---- Optimal Binary Search Tree (DP): O(n^3) time, O(n^2) space ----
   Keys are sorted; freq[i] is how often key i is searched. Returns the minimum total search cost. */
int obstMin(const int freq[], int n) {
    static long long cost[14][14];
    long long pre[14] = {0};
    if (n < 1 || n > MAXN) return -1;
    for (int i = 0; i < n; i++) { if (freq[i] < 0 || freq[i] > 100000) return -1; pre[i + 1] = pre[i] + freq[i]; }
    for (int i = 0; i < n; i++) cost[i][i] = freq[i];
    for (int len = 2; len <= n; len++)
        for (int i = 0; i + len - 1 < n; i++) {
            int j = i + len - 1;
            long long best = LLONG_MAX;
            for (int r = i; r <= j; r++) {
                long long c = (r > i ? cost[i][r - 1] : 0) + (r < j ? cost[r + 1][j] : 0);
                if (c < best) best = c;
            }
            cost[i][j] = best + (pre[j + 1] - pre[i]);        /* every key moves one level deeper */
        }
    return (int)cost[0][n - 1];
}
void demoOBST(void) {
    int f[3] = {34, 8, 50};
    printf("Keys k1 < k2 < k3 searched with frequencies 34, 8, 50\n");
    printf("Minimum total search cost = %d (root k3, then k1, then k2)\n", obstMin(f, 3));
    printf("Frequent keys go near the root. O(n^3) time, O(n^2) space.\n");
}

/* ---- Multi-stop delivery route (TSP). Distances between stops come from Floyd-Warshall. ---- */
static int D[MAXTSP][MAXTSP], tk;
static int tBest, tPath[MAXTSP], tBestPath[MAXTSP];
static long tLeaves;

static void brute(int pos, int used, int cost) {
    if (pos == tk) {
        int tot = cost + D[tPath[tk - 1]][0];
        tLeaves++;
        if (tot < tBest) { tBest = tot; memcpy(tBestPath, tPath, sizeof tPath); }
        return;
    }
    for (int c = 1; c < tk; c++) if (!(used & (1 << c))) {
        tPath[pos] = c;
        brute(pos + 1, used | (1 << c), cost + D[tPath[pos - 1]][c]);
    }
}

static int heldKarp(void) {                          /* O(n^2 2^n) time, O(n 2^n) space */
    static int dp[1 << MAXTSP][MAXTSP];
    for (int m = 0; m < (1 << tk); m++) for (int i = 0; i < tk; i++) dp[m][i] = INF * 4;
    dp[1][0] = 0;
    for (int m = 1; m < (1 << tk); m++) if (m & 1)
        for (int last = 0; last < tk; last++) if ((m & (1 << last)) && dp[m][last] < INF * 4)
            for (int nx = 1; nx < tk; nx++) if (!(m & (1 << nx))) {
                int c = dp[m][last] + D[last][nx];
                if (c < dp[m | (1 << nx)][nx]) dp[m | (1 << nx)][nx] = c;
            }
    int best = INT_MAX, full = (1 << tk) - 1;
    for (int last = 1; last < tk; last++)
        if (dp[full][last] < INF * 4 && dp[full][last] + D[last][0] < best) best = dp[full][last] + D[last][0];
    return best;
}

typedef struct { int path[MAXTSP], len, mask, cost, bound; } TNode;
static int tspBound(const TNode *t) {                /* each city still to be left costs at least its cheapest allowed exit */
    int lb = t->cost, last = t->path[t->len - 1];
    for (int u = 0; u < tk; u++) {
        if (u != last && (t->mask & (1 << u))) continue;
        int mn = INT_MAX;
        for (int v = 0; v < tk; v++) {
            if (v == u) continue;
            if (v != 0 && (t->mask & (1 << v))) continue;
            if (D[u][v] < mn) mn = D[u][v];
        }
        if (mn < INT_MAX) lb += mn;
    }
    return lb;
}
static int tspBB(long *expanded, long *generated) {  /* least-cost branch and bound */
    static TNode pool[10000];
    static int live[10000];
    int nl = 0, np = 0, best = INT_MAX;
    TNode root = {{0}, 1, 1, 0, 0};
    root.bound = tspBound(&root);
    pool[np] = root; live[nl++] = np++;
    *expanded = 0; *generated = 1;
    while (nl > 0) {
        int bi = 0;
        for (int i = 1; i < nl; i++) if (pool[live[i]].bound < pool[live[bi]].bound) bi = i;
        TNode cur = pool[live[bi]];
        live[bi] = live[--nl];
        if (cur.bound >= best) break;                /* every other live node has a bound >= this one */
        (*expanded)++;
        for (int c = 1; c < tk; c++) if (!(cur.mask & (1 << c))) {
            TNode ch = cur;
            ch.cost += D[cur.path[cur.len - 1]][c];
            ch.path[ch.len++] = c; ch.mask |= 1 << c;
            (*generated)++;
            if (ch.len == tk) { int tot = ch.cost + D[c][0]; if (tot < best) best = tot; }
            else { ch.bound = tspBound(&ch); if (ch.bound < best && np < 10000) { pool[np] = ch; live[nl++] = np++; } }
        }
    }
    return best;
}

/* stops[0] is the depot. result = {brute force, Held-Karp, Branch and Bound}. Returns 0, or
   -1 invalid input, -2 some stop unreachable, -3 negative cycle. */
int tspSolveAll(const Graph *g, const int stops[], int k, int verbose, int result[3]) {
    static int fw[MAXV][MAXV];
    if (k < 2 || k > MAXTSP) return -1;
    for (int i = 0; i < k; i++) {
        if (stops[i] < 0 || stops[i] >= g->n) return -1;
        for (int j = 0; j < i; j++) if (stops[j] == stops[i]) return -1;
    }
    if (floydWarshall(g, fw)) return -3;
    tk = k;
    for (int i = 0; i < k; i++) for (int j = 0; j < k; j++) {
        D[i][j] = fw[stops[i]][stops[j]];
        if (D[i][j] >= INF) return -2;
    }
    if (verbose) {
        printf("Distance matrix between stops (Floyd-Warshall):\n        ");
        for (int j = 0; j < k; j++) printf("%7s", ds.nodeName[stops[j]]);
        printf("\n");
        for (int i = 0; i < k; i++) {
            printf("%-8s", ds.nodeName[stops[i]]);
            for (int j = 0; j < k; j++) printf("%7d", D[i][j]);
            printf("\n");
        }
    }
    tBest = INT_MAX; tLeaves = 0; tPath[0] = 0;
    brute(1, 1, 0);
    result[0] = tBest;
    result[1] = heldKarp();
    long ex, gen;
    result[2] = tspBB(&ex, &gen);
    if (verbose) {
        printf("Brute force (backtracking, no pruning): best %d, tours examined %ld, route ", result[0], tLeaves);
        for (int i = 0; i < k; i++) printf("%s -> ", ds.nodeName[stops[tBestPath[i]]]);
        printf("%s\n", ds.nodeName[stops[0]]);
        printf("Held-Karp DP                           : best %d\n", result[1]);
        printf("Branch and Bound (least-cost)          : best %d, nodes expanded %ld, generated %ld\n", result[2], ex, gen);
    }
    return 0;
}
void demoTSP(void) {
    int stops[5], res[3];
    const char *nm[5] = {"W1", "CustX", "CustY", "HubA", "HubC"};
    for (int i = 0; i < 5; i++) { stops[i] = nodeByName(nm[i]); if (stops[i] < 0) { printf("The TSP demo needs the built-in dataset (reset the data first).\n"); return; } }
    printf("One agent leaves W1 and must visit CustX, CustY, HubA, HubC, then return.\n");
    int r = tspSolveAll(&ds.roads, stops, 5, 1, res);
    if (r) { printf("Could not solve (code %d).\n", r); return; }
    printf("Brute force O(n!), Held-Karp O(n^2 2^n), B&B worst case O(n!) but prunes. TSP is NP-hard.\n");
}

/* ---- N-Queens (backtracking): worst case O(n!), O(n) space ---- */
static int nq, ncol[12], nsols, nshow;
static int safe(int r, int c) { for (int i = 0; i < r; i++) if (ncol[i] == c || abs(ncol[i] - c) == r - i) return 0; return 1; }
static void place(int r) {
    if (r == nq) {
        if (++nsols == 1 && nshow) {
            printf("First solution:\n");
            for (int i = 0; i < nq; i++) { for (int j = 0; j < nq; j++) printf("%c ", ncol[i] == j ? 'Q' : '.'); printf("\n"); }
        }
        return;
    }
    for (int c = 0; c < nq; c++) if (safe(r, c)) { ncol[r] = c; place(r + 1); }
}
int nQueensCount(int n, int showFirst) {
    if (n < 1 || n > 12) return -1;
    nq = n; nsols = 0; nshow = showFirst;
    place(0);
    return nsols;
}
void demoNQueens(void) {
    for (int n = 4; n <= 8; n += 2) printf("%d-Queens: %d solutions\n", n, nQueensCount(n, n == 6));
    printf("One queen per row; a partial board is abandoned as soon as it has a conflict.\n");
}

/* ---- Sum of Subsets (backtracking): which parcels exactly fill a vehicle. O(2^n), O(n) space ---- */
static const int *sw; static int sn, starget, spick[16], scount, sverbose;
static void subsetRec(int i, int sum, int rem) {
    if (sum == starget) {
        scount++;
        if (sverbose) { printf("  {"); for (int j = 0; j < sn; j++) if (spick[j]) printf(" %d", sw[j]); printf(" }\n"); }
        return;
    }
    if (i == sn || sum > starget || sum + rem < starget) return;        /* prune */
    spick[i] = 1; subsetRec(i + 1, sum + sw[i], rem - sw[i]);
    spick[i] = 0; subsetRec(i + 1, sum, rem - sw[i]);
}
int subsetSumCount(const int w[], int n, int target, int verbose) {
    int total = 0;
    if (n < 1 || n > 16 || target < 0) return -1;
    for (int i = 0; i < n; i++) { if (w[i] < 1) return -1; total += w[i]; }
    sw = w; sn = n; starget = target; scount = 0; sverbose = verbose;
    memset(spick, 0, sizeof spick);
    subsetRec(0, 0, total);
    return scount;
}
void demoSubsetSum(void) {
    int w[7] = {10, 7, 5, 18, 12, 20, 15};
    printf("Parcel weights: 10 7 5 18 12 20 15; fill a vehicle to exactly 35 kg.\n");
    int c = subsetSumCount(w, 7, 35, 1);
    printf("%d exact combination(s). O(2^n) worst case; pruning cuts overweight and hopeless branches.\n", c);
}
