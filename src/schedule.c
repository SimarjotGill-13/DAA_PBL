#include "common.h"

/* ---------------- Delivery scheduling ---------------- */

static int cmp3(long x, long y) { return (x > y) - (x < y); }
static int cmpEnd(const void *a, const void *b)   { return cmp3(((const Delivery *)a)->end, ((const Delivery *)b)->end); }
static int cmpStart(const void *a, const void *b) { return cmp3(((const Delivery *)a)->start, ((const Delivery *)b)->start); }

static void pt(int m) { printf("%02d:%02d", m / 60, m % 60); }
static void pd(const Delivery *d) { printf("D%d ", d->id); pt(d->start); printf("-"); pt(d->end); }

/* Activity Selection (greedy, earliest finish first): ONE agent, most compatible deliveries.
   Touching intervals (end == next start) are compatible. O(n log n) time, O(n) space.
   Returns the number chosen, or -1 on invalid input. */
int activitySelection(const Delivery in[], int n, Delivery chosen[]) {
    if (n < 0 || n > MAXD) return -1;
    Delivery d[MAXD];
    if (n > 0) memcpy(d, in, sizeof(Delivery) * (size_t)n);
    if (mergeSort(d, n, sizeof(Delivery), cmpEnd)) return -1;
    int cnt = 0, lastEnd = INT_MIN;
    for (int i = 0; i < n; i++)
        if (d[i].start >= lastEnd) { chosen[cnt++] = d[i]; lastEnd = d[i].end; }
    return cnt;
}

/* Interval Partitioning (greedy, earliest start first): serve ALL deliveries with the fewest agents.
   O(n log n + n*k) time, O(n) space. Returns the number of agents, or -1 on invalid input. */
int intervalPartition(const Delivery in[], int n, Delivery sorted[], int agentOf[]) {
    if (n < 0 || n > MAXD) return -1;
    if (n > 0) memcpy(sorted, in, sizeof(Delivery) * (size_t)n);
    if (mergeSort(sorted, n, sizeof(Delivery), cmpStart)) return -1;
    int agentEnd[MAXD], k = 0;
    for (int i = 0; i < n; i++) {
        int a;
        for (a = 0; a < k; a++) if (agentEnd[a] <= sorted[i].start) break;     /* a free agent */
        if (a == k) k++;                                                        /* none free: open a new one */
        agentEnd[a] = sorted[i].end;
        agentOf[i] = a;
    }
    return k;
}

#define ND 8
static const Delivery sample[ND] = {
    {1, 540, 630}, {2, 570, 660}, {3, 630, 720}, {4, 600, 690}, {5, 660, 750}, {6, 720, 780}, {7, 540, 600}, {8, 690, 810}};

void demoScheduling(void) {
    Delivery ch[MAXD], so[MAXD];
    int agentOf[MAXD];
    printf("Delivery time windows:\n");
    for (int i = 0; i < ND; i++) { printf("  "); pd(&sample[i]); printf("\n"); }

    int c = activitySelection(sample, ND, ch);
    printf("\n[Activity Selection] ONE agent, choose the most compatible deliveries: %d\n", c);
    for (int i = 0; i < c; i++) { printf("  "); pd(&ch[i]); printf("\n"); }
    printf("  (the others are dropped: one agent cannot do overlapping jobs)\n");

    int k = intervalPartition(sample, ND, so, agentOf);
    printf("\n[Interval Partitioning] serve ALL deliveries, minimum agents: %d\n", k);
    for (int a = 0; a < k; a++) {
        printf("  Agent %d:", a + 1);
        for (int i = 0; i < ND; i++) if (agentOf[i] == a) { printf("  "); pd(&so[i]); }
        printf("\n");
    }
    int depth = 0;
    for (int i = 0; i < ND; i++) {
        int cnt = 0;
        for (int j = 0; j < ND; j++) if (sample[j].start <= sample[i].start && sample[i].start < sample[j].end) cnt++;
        if (cnt > depth) depth = cnt;
    }
    printf("  Maximum overlap at one moment (depth) = %d, so %d agents is the true minimum.\n", depth, k);
    printf("Difference: Activity Selection picks compatible jobs for ONE resource; Interval Partitioning\n"
           "assigns ALL overlapping jobs across MANY resources.\n");
}

/* ---------------- Knapsack: loading one vehicle ---------------- */

static int validItems(const Item it[], int n, int cap) {
    if (n < 1 || n > MAXN || cap < 0 || cap > MAXCAP) return 0;
    for (int i = 0; i < n; i++) if (it[i].w < 1 || it[i].v < 0) return 0;
    return 1;
}

static int cmpRatio(const void *a, const void *b) {            /* descending value/weight, no overflow */
    const Item *x = a, *y = b;
    long long l = (long long)y->v * x->w, r = (long long)x->v * y->w;
    return (l > r) - (l < r);
}

/* Fractional: greedy by value/weight, O(n log n). Valid only if items can be split. -1 if invalid. */
double knapFractional(const Item it[], int n, int cap) {
    if (!validItems(it, n, cap)) return -1;
    Item s[MAXN];
    memcpy(s, it, sizeof(Item) * (size_t)n);
    if (mergeSort(s, n, sizeof(Item), cmpRatio)) return -1;
    double val = 0;
    for (int i = 0; i < n && cap > 0; i++) {
        if (s[i].w <= cap) { val += s[i].v; cap -= s[i].w; }
        else { val += (double)s[i].v * cap / s[i].w; cap = 0; }
    }
    return val;
}

/* 0/1 Dynamic Programming: O(nW) time, O(nW) space for the table (O(W) if only the value is needed).
   Returns the best value (-1 if invalid) and writes the chosen parcels into chosen. */
int knapDP(const Item it[], int n, int cap, char chosen[], size_t chosenCap) {
    static int dp[MAXN + 1][MAXCAP + 1];
    if (!validItems(it, n, cap)) return -1;
    for (int w = 0; w <= cap; w++) dp[0][w] = 0;
    for (int i = 1; i <= n; i++)
        for (int w = 0; w <= cap; w++) {
            dp[i][w] = dp[i - 1][w];                                           /* skip item i */
            if (it[i - 1].w <= w && dp[i - 1][w - it[i - 1].w] + it[i - 1].v > dp[i][w])
                dp[i][w] = dp[i - 1][w - it[i - 1].w] + it[i - 1].v;           /* take item i */
        }
    if (chosen && chosenCap) {
        chosen[0] = '\0';
        int w = cap;
        for (int i = n; i >= 1; i--)
            if (dp[i][w] != dp[i - 1][w]) {
                if (strlen(chosen) + strlen(it[i - 1].name) + 2 < chosenCap) { strcat(chosen, it[i - 1].name); strcat(chosen, " "); }
                w -= it[i - 1].w;
            }
    }
    return dp[n][cap];
}

/* 0/1 Backtracking: include/exclude recursion, prunes only overweight branches. O(2^n). */
static const Item *btItems; static int btN, btCap, btBest; static long btCalls;
static void bt(int i, int w, int v) {
    btCalls++;
    if (w > btCap) return;
    if (i == btN) { if (v > btBest) btBest = v; return; }
    bt(i + 1, w + btItems[i].w, v + btItems[i].v);
    bt(i + 1, w, v);
}
int knapBacktrack(const Item it[], int n, int cap, long *calls) {
    if (!validItems(it, n, cap)) return -1;
    btItems = it; btN = n; btCap = cap; btBest = 0; btCalls = 0;
    bt(0, 0, 0);
    if (calls) *calls = btCalls;
    return btBest;
}

/* 0/1 Branch and Bound. mode 0 = FIFO (queue), 1 = LIFO (stack), 2 = Least-Cost (best bound first).
   Bound = profit so far + fractional fill of the remaining items (items sorted by ratio). */
typedef struct { int level, profit, weight; double bound; } KNode;
static double kbound(const KNode *u, const Item s[], int n, int cap) {
    if (u->weight > cap) return 0;
    double b = u->profit;
    int w = u->weight, j = u->level + 1;
    while (j < n && w + s[j].w <= cap) { w += s[j].w; b += s[j].v; j++; }
    if (j < n) b += (double)(cap - w) * s[j].v / s[j].w;
    return b;
}
int knapBB(const Item it[], int n, int cap, int mode, long *expanded) {
    static KNode pool[1 << (MAXN + 2)];
    static int live[1 << (MAXN + 2)];
    if (!validItems(it, n, cap) || mode < 0 || mode > 2) return -1;
    Item s[MAXN];
    memcpy(s, it, sizeof(Item) * (size_t)n);
    if (mergeSort(s, n, sizeof(Item), cmpRatio)) return -1;
    int nl = 0, np = 0, best = 0;
    long ex = 0;
    pool[np] = (KNode){-1, 0, 0, 0};
    pool[np].bound = kbound(&pool[np], s, n, cap);
    live[nl++] = np++;
    while (nl > 0) {
        int pick = (mode == 1) ? nl - 1 : 0;
        if (mode == 2) for (int i = 1; i < nl; i++) if (pool[live[i]].bound > pool[live[pick]].bound) pick = i;
        KNode u = pool[live[pick]];
        for (int i = pick; i < nl - 1; i++) live[i] = live[i + 1];
        nl--;
        if (u.bound <= best) continue;                                         /* prune */
        ex++;
        int l = u.level + 1;
        KNode in = {l, u.profit + s[l].v, u.weight + s[l].w, 0};
        if (in.weight <= cap) {
            if (in.profit > best) best = in.profit;
            in.bound = kbound(&in, s, n, cap);
            if (l < n - 1 && in.bound > best) { pool[np] = in; live[nl++] = np++; }
        }
        KNode out = {l, u.profit, u.weight, 0};
        out.bound = kbound(&out, s, n, cap);
        if (l < n - 1 && out.bound > best) { pool[np] = out; live[nl++] = np++; }
    }
    if (expanded) *expanded = ex;
    return best;
}

void demoKnapsack(void) {
    static const Item items[4] = {{"P1", 10, 60}, {"P2", 20, 100}, {"P3", 30, 120}, {"P4", 15, 75}};
    const int cap = 50;
    printf("Parcels (weight kg / priority value) for one vehicle, capacity %d kg:\n", cap);
    for (int i = 0; i < 4; i++) printf("  %s: %d kg, value %d\n", items[i].name, items[i].w, items[i].v);
    printf("\nFractional (greedy by value/weight): value %.1f  <- NOT valid: parcels are indivisible\n", knapFractional(items, 4, cap));
    char ch[32];
    int v = knapDP(items, 4, cap, ch, sizeof ch);
    printf("0/1 Dynamic Programming   : value %d, parcels: %s\n", v, ch);
    long calls;
    v = knapBacktrack(items, 4, cap, &calls);
    printf("0/1 Backtracking          : value %d, recursive calls %ld\n", v, calls);
    const char *nm[3] = {"FIFO", "LIFO", "Least-Cost"};
    for (int m = 0; m < 3; m++) {
        long ex;
        v = knapBB(items, 4, cap, m, &ex);
        printf("0/1 Branch&Bound %-10s: value %d, nodes expanded %ld\n", nm[m], v, ex);
    }
    printf("DP: O(nW) pseudo-polynomial. Backtracking/B&B: O(2^n) worst case; B&B prunes with a bound.\n");
}
