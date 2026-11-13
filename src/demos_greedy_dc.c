#include "common.h"

/* ---- Maximum Subarray (Divide and Conquer): T(n) = 2T(n/2) + n -> O(n log n), O(log n) stack ---- */
typedef struct { int lo, hi, sum; } Sub;

static Sub crossing(const int a[], int lo, int mid, int hi) {
    int ls = INT_MIN, s = 0, li = mid, rs = INT_MIN, ri = mid + 1;
    for (int i = mid; i >= lo; i--) { s += a[i]; if (s > ls) { ls = s; li = i; } }
    s = 0;
    for (int i = mid + 1; i <= hi; i++) { s += a[i]; if (s > rs) { rs = s; ri = i; } }
    return (Sub){li, ri, ls + rs};
}
static Sub maxSub(const int a[], int lo, int hi) {
    if (lo == hi) return (Sub){lo, hi, a[lo]};
    int mid = lo + (hi - lo) / 2;
    Sub l = maxSub(a, lo, mid), r = maxSub(a, mid + 1, hi), c = crossing(a, lo, mid, hi);
    if (l.sum >= r.sum && l.sum >= c.sum) return l;
    return (r.sum >= c.sum) ? r : c;
}
/* Returns the best sum (INT_MIN if n < 1) and the index range. */
int maxSubarray(const int a[], int n, int *lo, int *hi) {
    if (n < 1) return INT_MIN;
    Sub s = maxSub(a, 0, n - 1);
    if (lo) *lo = s.lo;
    if (hi) *hi = s.hi;
    return s.sum;
}
void demoMaxSubarray(void) {
    int a[16] = {13, -3, -25, 20, -3, -16, -23, 18, 20, -7, 12, -5, -22, 15, -4, 7}, lo, hi;
    int s = maxSubarray(a, 16, &lo, &hi);
    printf("Daily net sales change of a product: ");
    for (int i = 0; i < 16; i++) printf("%d ", a[i]);
    printf("\nBest consecutive period: day %d to day %d, total gain %d\n", lo, hi, s);
    printf("O(n log n) time, O(log n) recursion stack. (Kadane's scan would be O(n).)\n");
}

/* ---- Powering a number (Divide and Conquer): T(n) = T(n/2) + 1 -> O(log n) ---- */
long long powerDC(long long x, int n) {
    if (n < 0) return -1;
    if (n == 0) return 1;
    long long h = powerDC(x, n / 2);
    return (n % 2) ? h * h * x : h * h;
}
long long modPowerDC(long long x, long long n, long long m) {
    if (n < 0 || m < 1) return -1;
    if (n == 0) return 1 % m;
    long long h = modPowerDC(x, n / 2, m);
    h = (h * h) % m;
    return (n % 2) ? (h * (((x % m) + m) % m)) % m : h;
}
void demoPower(void) {
    printf("2^10 = %lld, 3^13 = %lld\n", powerDC(2, 10), powerDC(3, 13));
    printf("7^128 mod 13 = %lld (the modular version avoids overflow; used by Rabin-Karp and CRT)\n", modPowerDC(7, 128, 13));
    printf("O(log n) multiplications instead of n-1; O(log n) stack space.\n");
}

/* ---- Celebrity Problem: O(n) elimination, O(1) space. knows[i*n+j] = 1 if i knows j. ---- */
int celebrityFind(const int *knows, int n) {
    if (n < 1) return -1;
    int c = 0;
    for (int i = 1; i < n; i++) if (knows[c * n + i]) c = i;     /* if c knows i, c cannot be the celebrity */
    for (int i = 0; i < n; i++) if (i != c && (knows[c * n + i] || !knows[i * n + c])) return -1;
    return c;
}
void demoCelebrity(void) {
    int K[16] = {0, 1, 1, 0,  0, 0, 1, 0,  0, 0, 0, 0,  1, 0, 1, 0};
    int c = celebrityFind(K, 4);
    if (c >= 0) printf("Person %d is the celebrity (known by all, knows nobody).\n", c);
    else printf("No celebrity exists.\n");
    printf("Each question removes one candidate: O(n) 'knows' checks, O(1) space.\n");
}

/* ---- Strassen matrix multiplication (n a power of two, n <= SN): T(n) = 7T(n/2) + O(n^2) ---- */
static void madd(int h, Mat x, Mat y, Mat z) { for (int i = 0; i < h; i++) for (int j = 0; j < h; j++) z[i][j] = x[i][j] + y[i][j]; }
static void msub(int h, Mat x, Mat y, Mat z) { for (int i = 0; i < h; i++) for (int j = 0; j < h; j++) z[i][j] = x[i][j] - y[i][j]; }

int standardMultiply(int n, Mat a, Mat b, Mat c, long *mults) {
    if (n < 1 || n > SN) return -1;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            c[i][j] = 0;
            for (int k = 0; k < n; k++) { c[i][j] += a[i][k] * b[k][j]; if (mults) (*mults)++; }
        }
    return 0;
}

int strassenMultiply(int n, Mat a, Mat b, Mat c, long *mults) {
    if (n < 1 || n > SN || (n & (n - 1))) return -1;
    if (n == 1) { c[0][0] = a[0][0] * b[0][0]; if (mults) (*mults)++; return 0; }
    int h = n / 2;
    Mat a11, a12, a21, a22, b11, b12, b21, b22, m1, m2, m3, m4, m5, m6, m7, t1, t2;
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            a11[i][j] = a[i][j];     a12[i][j] = a[i][j + h];     a21[i][j] = a[i + h][j];     a22[i][j] = a[i + h][j + h];
            b11[i][j] = b[i][j];     b12[i][j] = b[i][j + h];     b21[i][j] = b[i + h][j];     b22[i][j] = b[i + h][j + h];
        }
    madd(h, a11, a22, t1); madd(h, b11, b22, t2); strassenMultiply(h, t1, t2, m1, mults);   /* M1 = (A11+A22)(B11+B22) */
    madd(h, a21, a22, t1);                         strassenMultiply(h, t1, b11, m2, mults);  /* M2 = (A21+A22)B11 */
    msub(h, b12, b22, t2);                         strassenMultiply(h, a11, t2, m3, mults);  /* M3 = A11(B12-B22) */
    msub(h, b21, b11, t2);                         strassenMultiply(h, a22, t2, m4, mults);  /* M4 = A22(B21-B11) */
    madd(h, a11, a12, t1);                         strassenMultiply(h, t1, b22, m5, mults);  /* M5 = (A11+A12)B22 */
    msub(h, a21, a11, t1); madd(h, b11, b12, t2); strassenMultiply(h, t1, t2, m6, mults);   /* M6 = (A21-A11)(B11+B12) */
    msub(h, a12, a22, t1); madd(h, b21, b22, t2); strassenMultiply(h, t1, t2, m7, mults);   /* M7 = (A12-A22)(B21+B22) */
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            c[i][j]         = m1[i][j] + m4[i][j] - m5[i][j] + m7[i][j];
            c[i][j + h]     = m3[i][j] + m5[i][j];
            c[i + h][j]     = m2[i][j] + m4[i][j];
            c[i + h][j + h] = m1[i][j] - m2[i][j] + m3[i][j] + m6[i][j];
        }
    return 0;
}
void demoStrassen(void) {
    Mat a, b, c1, c2;
    for (int n = 2; n <= 8; n *= 2) {
        long ms = 0, mt = 0;
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) { a[i][j] = (i * 3 + j) % 7 - 3; b[i][j] = (i + j * 2) % 5 - 2; }
        strassenMultiply(n, a, b, c1, &ms);
        standardMultiply(n, a, b, c2, &mt);
        int same = 1;
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) if (c1[i][j] != c2[i][j]) same = 0;
        printf("n = %d: Strassen %ld multiplications, standard %ld, results equal: %s\n", n, ms, mt, same ? "yes" : "NO");
    }
    printf("T(n) = 7T(n/2) + O(n^2) -> O(n^2.81) against O(n^3). Not needed by the order workflow (demo only).\n");
}

/* ---- Huffman coding (greedy): O(n^2) with a simple minimum search (O(n log n) with a heap) ---- */
typedef struct { int freq, left, right, ch; } HNode;
static HNode hn[512];
static char codeTab[256][32];
static void genCodes(int n, char *buf, int depth) {
    if (hn[n].left < 0) { buf[depth] = '\0'; strcpy(codeTab[hn[n].ch], depth ? buf : "0"); return; }
    buf[depth] = '0'; genCodes(hn[n].left, buf, depth + 1);
    buf[depth] = '1'; genCodes(hn[n].right, buf, depth + 1);
}
/* Returns the total number of bits of the encoded text, or -1 for empty text or too many symbols. */
int huffmanBits(const char *text, int verbose) {
    int freq[256] = {0}, alive[512] = {0}, nn = 0, len = 0;
    for (const char *p = text; *p; p++) { freq[(unsigned char)*p]++; len++; }
    if (len == 0) return -1;
    for (int c = 0; c < 256; c++) if (freq[c]) { hn[nn] = (HNode){freq[c], -1, -1, c}; alive[nn++] = 1; }
    if (nn > 30) return -1;                                       /* keeps every code under 32 bits */
    for (;;) {
        int a = -1, b = -1;
        for (int i = 0; i < nn; i++) if (alive[i]) {
            if (a < 0 || hn[i].freq < hn[a].freq) { b = a; a = i; }
            else if (b < 0 || hn[i].freq < hn[b].freq) b = i;
        }
        if (b < 0) break;
        hn[nn] = (HNode){hn[a].freq + hn[b].freq, a, b, -1};
        alive[a] = alive[b] = 0; alive[nn++] = 1;
    }
    char buf[32];
    genCodes(nn - 1, buf, 0);
    int bits = 0;
    if (verbose) printf("char freq code\n");
    for (int c = 0; c < 256; c++) if (freq[c]) {
        if (verbose) printf("  %c   %2d   %s\n", c == ' ' ? '_' : c, freq[c], codeTab[c]);
        bits += freq[c] * (int)strlen(codeTab[c]);
    }
    return bits;
}
void demoHuffman(void) {
    const char *text = "ORDER SHIPPED ORDER DELIVERED";
    printf("Text: \"%s\"\n", text);
    int bits = huffmanBits(text, 1), len = (int)strlen(text);
    printf("Huffman: %d bits vs fixed 8-bit: %d bits (saving %.0f%%)\n", bits, len * 8, 100.0 * (len * 8 - bits) / (len * 8));
}

/* ---- Minimum spanning trees on an undirected, non-negative road network ---- */
typedef struct { int u, v, w; } Edge;
static int cmpEdge(const void *a, const void *b) {
    int x = ((const Edge *)a)->w, y = ((const Edge *)b)->w;
    return (x > y) - (x < y);
}
static int uf[MAXV];
static int fnd(int x) { return uf[x] == x ? x : (uf[x] = fnd(uf[x])); }

/* Prim: O(V^2) with a matrix, O(V) space. Spans every component (a forest) so it matches Kruskal. -1 if invalid. */
int mstPrim(const Graph *g, int verbose) {
    if (!graphIsUndirectedNonNegative(g)) return -1;
    int n = g->n, key[MAXV], par[MAXV], in[MAXV] = {0}, total = 0;
    for (int i = 0; i < n; i++) { key[i] = INF; par[i] = -1; }
    for (int r = 0; r < n; r++) {
        int u = -1;
        for (int i = 0; i < n; i++) if (!in[i] && (u < 0 || key[i] < key[u])) u = i;
        if (key[u] >= INF) key[u] = 0;                            /* start of a new component */
        in[u] = 1; total += key[u];
        if (par[u] >= 0 && verbose) printf("  %s - %s (%d)\n", ds.nodeName[par[u]], ds.nodeName[u], key[u]);
        for (int v = 0; v < n; v++) { int w = g->w[u][v]; if (w != NOEDGE && !in[v] && w < key[v]) { key[v] = w; par[v] = u; } }
    }
    return total;
}
/* Kruskal: O(E log E) time, O(V + E) space. -1 if invalid. */
int mstKruskal(const Graph *g, int verbose) {
    if (!graphIsUndirectedNonNegative(g)) return -1;
    static Edge e[MAXV * MAXV];
    int ne = 0, total = 0;
    for (int i = 0; i < g->n; i++) for (int j = i + 1; j < g->n; j++) if (g->w[i][j] != NOEDGE) e[ne++] = (Edge){i, j, g->w[i][j]};
    if (mergeSort(e, ne, sizeof(Edge), cmpEdge)) return -1;
    for (int i = 0; i < g->n; i++) uf[i] = i;
    for (int i = 0; i < ne; i++) {
        int a = fnd(e[i].u), b = fnd(e[i].v);
        if (a != b) { uf[a] = b; total += e[i].w; if (verbose) printf("  %s - %s (%d)\n", ds.nodeName[e[i].u], ds.nodeName[e[i].v], e[i].w); }
    }
    return total;
}
void demoMST(void) {
    if (!graphIsUndirectedNonNegative(&ds.roads)) { printf("MST needs an undirected, non-negative road network; the current roads include one-way or negative roads.\n"); return; }
    printf("Prim (spans every component):\n");
    int p = mstPrim(&ds.roads, 1);
    printf("  Prim total weight = %d\nKruskal:\n", p);
    int k = mstKruskal(&ds.roads, 1);
    printf("  Kruskal total weight = %d (%s Prim)\n", k, k == p ? "equals" : "DIFFERS from");
    printf("Prim O(V^2) [matrix]; Kruskal O(E log E) with union-find. Isolated nodes are left out.\n");
}
