#include "common.h"

/* ---------------- graph construction and checks ---------------- */

void graphInit(Graph *g, int n) {
    g->n = n;
    for (int i = 0; i < MAXV; i++) for (int j = 0; j < MAXV; j++) g->w[i][j] = NOEDGE;
}

int graphHasNegative(const Graph *g) {
    for (int u = 0; u < g->n; u++)
        for (int v = 0; v < g->n; v++) if (g->w[u][v] != NOEDGE && g->w[u][v] < 0) return 1;
    return 0;
}

/* Negative-cycle test: Bellman-Ford from a virtual source joined to every node (all distances
   start at 0). If the n-th pass still improves something, a negative cycle exists. O(VE). */
int graphHasNegativeCycle(const Graph *g) {
    int dist[MAXV] = {0};
    for (int pass = 0; pass < g->n; pass++) {
        int changed = 0;
        for (int u = 0; u < g->n; u++)
            for (int v = 0; v < g->n; v++) {
                int w = g->w[u][v];
                if (w != NOEDGE && dist[u] + w < dist[v]) { dist[v] = dist[u] + w; changed = 1; }
            }
        if (!changed) return 0;
    }
    return 1;
}

/* Returns 0 ok, 1 bad node, 2 weight out of range, 3 would create a negative cycle (graph unchanged). */
int graphAddArc(Graph *g, int u, int v, int w) {
    if (u < 0 || v < 0 || u >= g->n || v >= g->n || u == v) return 1;
    if (w < -MAXWEIGHT || w > MAXWEIGHT) return 2;
    int old = g->w[u][v];
    g->w[u][v] = w;
    if (w < 0 && graphHasNegativeCycle(g)) { g->w[u][v] = old; return 3; }
    return 0;
}

/* Two-way road. A negative two-way road is itself a negative cycle (a->b->a), so it is rejected (4). */
int graphAddRoad(Graph *g, int a, int b, int w) {
    if (a < 0 || b < 0 || a >= g->n || b >= g->n || a == b) return 1;
    if (w < -MAXWEIGHT || w > MAXWEIGHT) return 2;
    if (w < 0) return 4;
    graphAddArc(g, a, b, w);
    graphAddArc(g, b, a, w);
    return 0;
}

int graphIsUndirectedNonNegative(const Graph *g) {
    for (int u = 0; u < g->n; u++)
        for (int v = 0; v < g->n; v++) {
            if (g->w[u][v] != g->w[v][u]) return 0;
            if (g->w[u][v] != NOEDGE && g->w[u][v] < 0) return 0;
        }
    return 1;
}

void graphTranspose(const Graph *g, Graph *t) {
    graphInit(t, g->n);
    for (int u = 0; u < g->n; u++) for (int v = 0; v < g->n; v++) t->w[v][u] = g->w[u][v];
}

const char *algoName(RouteAlgo a) {
    return a == ALGO_DIJKSTRA ? "Dijkstra" : a == ALGO_BELLMAN_FORD ? "Bellman-Ford" : "none";
}

int nodeByName(const char *name) {
    for (int i = 0; i < ds.nNodes; i++) if (strcmp(ds.nodeName[i], name) == 0) return i;
    return -1;
}

/* ---------------- shortest-path algorithms ---------------- */

/* Dijkstra, array version: O(V^2) time, O(V) auxiliary space. Requires NON-NEGATIVE weights. */
void dijkstra(const Graph *g, int src, int dist[], int parent[]) {
    int visited[MAXV] = {0}, n = g->n;
    for (int i = 0; i < n; i++) { dist[i] = INF; parent[i] = -1; }
    dist[src] = 0;
    for (int round = 0; round < n; round++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] >= INF) break;
        visited[u] = 1;
        for (int v = 0; v < n; v++) {
            int w = g->w[u][v];
            if (w != NOEDGE && !visited[v] && dist[u] + w < dist[v]) { dist[v] = dist[u] + w; parent[v] = u; }
        }
    }
}

/* Bellman-Ford: O(VE) time, O(V) space. Allows negative weights. Returns 1 if a negative cycle is
   reachable from src (distances are then meaningless), else 0. */
int bellmanFord(const Graph *g, int src, int dist[], int parent[]) {
    int n = g->n;
    for (int i = 0; i < n; i++) { dist[i] = INF; parent[i] = -1; }
    dist[src] = 0;
    for (int pass = 1; pass < n; pass++) {
        int changed = 0;
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++) {
                int w = g->w[u][v];
                if (w != NOEDGE && dist[u] < INF && dist[u] + w < dist[v]) { dist[v] = dist[u] + w; parent[v] = u; changed = 1; }
            }
        if (!changed) break;
    }
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) {
            int w = g->w[u][v];
            if (w != NOEDGE && dist[u] < INF && dist[u] + w < dist[v]) return 1;
        }
    return 0;
}

/* Floyd-Warshall: O(V^3) time, O(V^2) space. Returns 1 if a negative cycle exists. */
int floydWarshall(const Graph *g, int d[MAXV][MAXV]) {
    int n = g->n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) d[i][j] = (i == j) ? 0 : (g->w[i][j] == NOEDGE ? INF : g->w[i][j]);
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (d[i][k] < INF && d[k][j] < INF && d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
        for (int i = 0; i < n; i++) if (d[i][i] < 0) return 1;   /* stop before values can overflow */
    }
    return 0;
}

/* Algorithm selection by data: no negative road -> Dijkstra (faster); otherwise Bellman-Ford.
   Returns 1 if a negative cycle makes shortest paths undefined. */
int shortestPaths(const Graph *g, int src, int dist[], int parent[], RouteAlgo *used) {
    if (!graphHasNegative(g)) {
        dijkstra(g, src, dist, parent);
        if (used) *used = ALGO_DIJKSTRA;
        return 0;
    }
    if (used) *used = ALGO_BELLMAN_FORD;
    return bellmanFord(g, src, dist, parent);
}

/* Distances FROM every node TO target (a delivery goes warehouse -> customer, and roads may be
   one-way). Run the chosen algorithm once from the customer on the reversed graph. parent[v] is
   then the next node on v's shortest route to target. */
int distancesToNode(int target, int dist[], int parent[], RouteAlgo *used) {
    Graph rev;
    graphTranspose(&ds.roads, &rev);
    return shortestPaths(&rev, target, dist, parent, used);
}

void printPath(const int parent[], int v) {
    if (parent[v] != -1) { printPath(parent, parent[v]); printf(" -> "); }
    printf("%s", ds.nodeName[v]);
}

void printRouteToTarget(const int parent[], int from) {
    for (int v = from; v != -1; v = parent[v]) {
        printf("%s", ds.nodeName[v]);
        if (parent[v] != -1) printf(" -> ");
    }
}

static void cell(char *buf, size_t cap, int x, int unreachable) {
    if (unreachable) snprintf(buf, cap, "inf"); else snprintf(buf, cap, "%d", x);
}

void compareRouting(int src) {
    int dd[MAXV], dp[MAXV], bf[MAXV], bp[MAXV], fw[MAXV][MAXV], ok = 1, n = ds.roads.n;
    int neg = graphHasNegative(&ds.roads);
    if (!neg) dijkstra(&ds.roads, src, dd, dp);
    int cyc = bellmanFord(&ds.roads, src, bf, bp);
    int fcyc = floydWarshall(&ds.roads, fw);
    printf("Source: %s%s\n", ds.nodeName[src], neg ? "   (graph has a negative road: Dijkstra is not valid)" : "");
    if (cyc || fcyc) { printf("A negative cycle is reachable: shortest distances are not defined.\n"); return; }
    printf("%-8s %9s %13s %15s   Route (Bellman-Ford tree)\n", "Node", "Dijkstra", "Bellman-Ford", "Floyd-Warshall");
    for (int i = 0; i < n; i++) {
        char a[16], b[16], c[16];
        if (neg) snprintf(a, sizeof a, "n/a"); else cell(a, sizeof a, dd[i], dd[i] >= INF);
        cell(b, sizeof b, bf[i], bf[i] >= INF);
        cell(c, sizeof c, fw[src][i], fw[src][i] >= INF);
        if ((!neg && dd[i] != bf[i]) || bf[i] != fw[src][i]) ok = 0;
        printf("%-8s %9s %13s %15s   ", ds.nodeName[i], a, b, c);
        if (bf[i] < INF) printPath(bp, i); else printf("unreachable");
        printf("\n");
    }
    printf("Algorithms agree: %s\n", ok ? "YES" : "NO");
    printf("Dijkstra O(V^2) [O(V) space] | Bellman-Ford O(VE) [O(V)] | Floyd-Warshall O(V^3) [O(V^2)]\n");
    printf("The order workflow uses Dijkstra when every road is non-negative, Bellman-Ford otherwise.\n");
}

/* ---------------- negative-weight demonstration ---------------- */

void demoNegativeWeights(void) {
    static const char *nm[4] = {"A", "B", "C", "D"};
    Graph g;
    int d1[MAXV], p1[MAXV], d2[MAXV], p2[MAXV];
    graphInit(&g, 4);
    graphAddArc(&g, 0, 1, 4); graphAddArc(&g, 0, 2, 5); graphAddArc(&g, 2, 1, -3); graphAddArc(&g, 1, 3, 2);
    printf("Part 1. Small directed graph (weight = NET cost of a leg; a promo credit makes it negative)\n");
    printf("  A->B 4, A->C 5, C->B -3, B->D 2.  True shortest from A: B = 2 (via C), D = 4.\n");
    dijkstra(&g, 0, d1, p1);
    int cyc = bellmanFord(&g, 0, d2, p2);
    printf("  Dijkstra     :"); for (int i = 0; i < 4; i++) printf(" %s=%d", nm[i], d1[i]);
    printf("   <- WRONG: B was finalised at 4 before the cheaper route through C was seen\n");
    printf("  Bellman-Ford :"); for (int i = 0; i < 4; i++) printf(" %s=%d", nm[i], d2[i]);
    printf("   negative cycle: %s\n", cyc ? "YES" : "no");

    g.w[3][2] = -5;                                   /* set directly: graphAddArc would reject it */
    cyc = bellmanFord(&g, 0, d2, p2);
    printf("\nPart 2. Add D->C -5. The cycle C->B->D->C costs -3 + 2 - 5 = -6 and can be repeated forever.\n");
    printf("  Bellman-Ford negative cycle: %s -> shortest distances are NOT defined.\n", cyc ? "YES" : "no");
    printf("  graphAddArc() refuses such an edge, so a loaded or edited road network never contains one.\n");
    printf("  A negative two-way road is already a negative cycle, so negative roads must be one-way.\n");

    printf("\nPart 3. The order workflow adapts to the data\n");
    int hc = nodeByName("HubC"), ha = nodeByName("HubA");
    if (hc < 0 || ha < 0 || nodeByName("CustX") < 0) { printf("  (needs the built-in dataset: reset the data first)\n"); return; }
    Dataset saved = ds;
    int savedCount = orderCount;
    Order o;
    printf("  Normal roads. Order PIN 100001, product 101, qty 2:\n");
    placeOrder(100001, 101, 2, 0, &o);
    printf("    routing = %s, chosen warehouse W%d, cost %d\n", algoName(o.algo), o.wh, o.cost);
    ds = saved; orderCount = savedCount;
    int r = graphAddArc(&ds.roads, hc, ha, -1);       /* one-way express corridor HubC -> HubA with a rebate */
    printf("  Add one-way road HubC->HubA of weight -1 (rebate), accepted: %s\n", r == 0 ? "yes" : "no");
    placeOrder(100001, 101, 2, 0, &o);
    printf("    routing = %s, chosen warehouse W%d, cost %d\n", algoName(o.algo), o.wh, o.cost);
    int cn = nodeByName("CustX"), dist[MAXV], par[MAXV], dist2[MAXV];
    distancesToNode(cn, dist, par, NULL);
    Graph rev;
    graphTranspose(&ds.roads, &rev);
    dijkstra(&rev, cn, dist2, par);
    printf("    distance W3->CustX: Bellman-Ford %d, plain Dijkstra %d (%s)\n", dist[2], dist2[2],
           dist[2] == dist2[2] ? "same here, but Dijkstra is not guaranteed on negative roads" : "Dijkstra is wrong");
    ds = saved; orderCount = savedCount;
    printf("  (data restored)\n");
    printf("Real distance is never negative, but a net cost can be (discount, credit, subsidy).\n");
    printf("Dijkstra O(V^2) vs Bellman-Ford O(VE): the extra pass is what detects a negative cycle.\n");
}

/* ---------------- sparse graphs for the stress test ---------------- */

static unsigned lcg(unsigned *s) { *s = *s * 1664525u + 1013904223u; return *s >> 8; }

void csrFree(Csr *c) { free(c->start); free(c->to); free(c->w); c->start = c->to = c->w = NULL; c->n = c->m = 0; }

/* Random connected undirected graph with m >= n edges (a ring plus random chords); weights 1..maxW.
   Returns 0 ok, -1 on invalid arguments or allocation failure. */
int csrBuildRandom(Csr *c, int n, int m, unsigned seed, int maxW) {
    c->n = n; c->m = 0; c->start = c->to = c->w = NULL;
    if (n < 3 || m < n || maxW < 1) return -1;
    int arcs = 2 * m;
    int *eu = malloc(sizeof(int) * (size_t)m), *ev = malloc(sizeof(int) * (size_t)m), *ew = malloc(sizeof(int) * (size_t)m);
    c->start = calloc((size_t)n + 1, sizeof(int));
    c->to = malloc(sizeof(int) * (size_t)arcs);
    c->w = malloc(sizeof(int) * (size_t)arcs);
    if (!eu || !ev || !ew || !c->start || !c->to || !c->w) { free(eu); free(ev); free(ew); csrFree(c); return -1; }
    for (int i = 0; i < m; i++) {
        if (i < n) { eu[i] = i; ev[i] = (i + 1) % n; }
        else { eu[i] = (int)(lcg(&seed) % (unsigned)n); do { ev[i] = (int)(lcg(&seed) % (unsigned)n); } while (ev[i] == eu[i]); }
        ew[i] = 1 + (int)(lcg(&seed) % (unsigned)maxW);
        c->start[eu[i] + 1]++; c->start[ev[i] + 1]++;
    }
    for (int i = 0; i < n; i++) c->start[i + 1] += c->start[i];
    int *fill = malloc(sizeof(int) * (size_t)n);
    if (!fill) { free(eu); free(ev); free(ew); csrFree(c); return -1; }
    memcpy(fill, c->start, sizeof(int) * (size_t)n);
    for (int i = 0; i < m; i++) {
        c->to[fill[eu[i]]] = ev[i]; c->w[fill[eu[i]]++] = ew[i];
        c->to[fill[ev[i]]] = eu[i]; c->w[fill[ev[i]]++] = ew[i];
    }
    free(fill); free(eu); free(ev); free(ew);
    c->m = m;
    return 0;
}

/* Dijkstra with a linear scan for the minimum on an adjacency list: O(V^2 + E). */
int dijkstraCsrArray(const Csr *c, int src, int *dist) {
    char *vis = calloc((size_t)c->n, 1);
    if (!vis) return -1;
    for (int i = 0; i < c->n; i++) dist[i] = INT_MAX;
    dist[src] = 0;
    for (int r = 0; r < c->n; r++) {
        int u = -1;
        for (int i = 0; i < c->n; i++) if (!vis[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INT_MAX) break;
        vis[u] = 1;
        for (int e = c->start[u]; e < c->start[u + 1]; e++)
            if (dist[u] + c->w[e] < dist[c->to[e]]) dist[c->to[e]] = dist[u] + c->w[e];
    }
    free(vis);
    return 0;
}

typedef struct { int d, v; } HeapItem;
static void hpush(HeapItem *h, int *sz, HeapItem x) {
    int i = (*sz)++;
    while (i > 0) { int p = (i - 1) / 2; if (h[p].d <= x.d) break; h[i] = h[p]; i = p; }
    h[i] = x;
}
static HeapItem hpop(HeapItem *h, int *sz) {
    HeapItem top = h[0], last = h[--(*sz)];
    int i = 0;
    for (;;) {
        int l = 2 * i + 1;
        if (l >= *sz) break;
        int r = l + 1, c = (r < *sz && h[r].d < h[l].d) ? r : l;
        if (h[c].d >= last.d) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = last;
    return top;
}

/* Dijkstra with a binary min-heap (lazy deletion): O((V + E) log V). Best for large sparse graphs. */
int dijkstraCsrHeap(const Csr *c, int src, int *dist) {
    HeapItem *h = malloc(sizeof(HeapItem) * ((size_t)c->start[c->n] + 1));
    if (!h) return -1;
    int sz = 0;
    for (int i = 0; i < c->n; i++) dist[i] = INT_MAX;
    dist[src] = 0;
    hpush(h, &sz, (HeapItem){0, src});
    while (sz > 0) {
        HeapItem t = hpop(h, &sz);
        if (t.d > dist[t.v]) continue;                /* stale entry */
        for (int e = c->start[t.v]; e < c->start[t.v + 1]; e++)
            if (t.d + c->w[e] < dist[c->to[e]]) { dist[c->to[e]] = t.d + c->w[e]; hpush(h, &sz, (HeapItem){dist[c->to[e]], c->to[e]}); }
    }
    free(h);
    return 0;
}
