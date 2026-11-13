#include "common.h"

/* ---- Maximum flow: Ford-Fulkerson with DFS augmenting paths. O(E * f) for integer capacities ---- */
static int res[FLOWN][FLOWN];
static int dfsFlow(int n, int u, int t, int f, int seen[]) {
    if (u == t) return f;
    seen[u] = 1;
    for (int v = 0; v < n; v++)
        if (!seen[v] && res[u][v] > 0) {
            int g = dfsFlow(n, v, t, f < res[u][v] ? f : res[u][v], seen);
            if (g > 0) { res[u][v] -= g; res[v][u] += g; return g; }
        }
    return 0;
}
/* Returns the maximum flow from s to t (-1 if invalid) and the flow on each edge (cap - residual). */
int maxFlow(int n, int cap[FLOWN][FLOWN], int s, int t, int flow[FLOWN][FLOWN]) {
    if (n < 2 || n > FLOWN || s < 0 || t < 0 || s >= n || t >= n || s == t) return -1;
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) { if (cap[i][j] < 0) return -1; res[i][j] = cap[i][j]; }
    long total = 0;
    int seen[FLOWN], f;
    for (;;) {
        memset(seen, 0, sizeof seen);
        f = dfsFlow(n, s, t, INT_MAX, seen);
        if (f <= 0) break;
        total += f;
    }
    if (flow) for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) flow[i][j] = cap[i][j] > res[i][j] ? cap[i][j] - res[i][j] : 0;
    return (int)total;
}
void demoMaxFlow(void) {
    static const char *nm[FLOWN] = {"S", "W1", "W2", "W3", "CustX", "CustY", "T"};
    int cap[FLOWN][FLOWN] = {{0}}, flow[FLOWN][FLOWN];
    cap[0][1] = 10; cap[0][3] = 5;                                  /* stock of product 101 (W2 has none) */
    cap[1][4] = 6; cap[1][5] = 3; cap[2][4] = 4; cap[2][5] = 4; cap[3][4] = 2; cap[3][5] = 5;   /* daily dispatch capacity */
    cap[4][6] = 8; cap[5][6] = 7;                                   /* customer-area demand */
    int f = maxFlow(7, cap, 0, 6, flow);
    printf("Supply (stock) 15, demand 15 (CustX 8, CustY 7). Maximum flow = %d units\n", f);
    for (int u = 1; u <= 3; u++) for (int v = 4; v <= 5; v++)
        if (cap[u][v]) printf("  %s -> %s : %d of %d\n", nm[u], nm[v], flow[u][v], cap[u][v]);
    printf("The bottleneck is dispatch capacity, not stock. Residual edges let the algorithm undo earlier choices.\n");
}

/* ---- Maximum bipartite matching (augmenting paths = Ford-Fulkerson on a unit network): O(V*E) ---- */
static int (*gElig)[MAXM], gNr, *gMatchR;
static int tryMatch(int u, int seen[]) {
    for (int a = 0; a < gNr; a++)
        if (gElig[u][a] && !seen[a]) {
            seen[a] = 1;
            if (gMatchR[a] < 0 || tryMatch(gMatchR[a], seen)) { gMatchR[a] = u; return 1; }
        }
    return 0;
}
/* elig[left][right] = 1 if allowed. Returns the matching size (-1 if invalid); matchR[right] = left or -1. */
int bipartiteMatching(int nl, int nr, int elig[MAXM][MAXM], int matchR[MAXM]) {
    if (nl < 0 || nr < 0 || nl > MAXM || nr > MAXM) return -1;
    gElig = elig; gNr = nr; gMatchR = matchR;
    for (int a = 0; a < nr; a++) matchR[a] = -1;
    int cnt = 0, seen[MAXM];
    for (int u = 0; u < nl; u++) { memset(seen, 0, sizeof seen); cnt += tryMatch(u, seen); }
    return cnt;
}
void demoMatching(void) {
    int elig[MAXM][MAXM] = {{0}}, matchR[MAXM];
    elig[0][0] = elig[0][1] = 1; elig[1][0] = 1; elig[2][1] = elig[2][2] = 1; elig[3][2] = 1;
    printf("Eligibility (order -> agents that can take it): O1:{A1,A2} O2:{A1} O3:{A2,A3} O4:{A3}\n");
    int c = bipartiteMatching(4, 3, elig, matchR);
    for (int a = 0; a < 3; a++) if (matchR[a] >= 0) printf("  Agent A%d <- Order O%d\n", a + 1, matchR[a] + 1);
    printf("Maximum matching size = %d (an agent takes at most one order here).\n", c);
}

/* ---- String matching: Naive, Rabin-Karp, Finite Automaton, KMP. Each returns the match count, -1 if the pattern is empty or longer than 31 ---- */
static int okPattern(const char *P) { size_t m = strlen(P); return m >= 1 && m <= 31; }

int naiveSearch(const char *T, const char *P, int verbose, long *cmps) {
    if (!okPattern(P)) return -1;
    int n = (int)strlen(T), m = (int)strlen(P), cnt = 0;
    long c = 0;
    for (int s = 0; s <= n - m; s++) {
        int j = 0;
        while (j < m) { c++; if (T[s + j] != P[j]) break; j++; }
        if (j == m) { if (verbose) printf(" %d", s); cnt++; }
    }
    if (cmps) *cmps = c;
    return cnt;
}

int rabinKarpSearch(const char *T, const char *P, int verbose, long *cmps, int *spurious) {
    if (!okPattern(P)) return -1;
    int n = (int)strlen(T), m = (int)strlen(P), d = 256, q = 101, h = 1, p = 0, t = 0, cnt = 0, sp = 0;
    long c = 0;
    if (n >= m) {
        for (int i = 0; i < m - 1; i++) h = (h * d) % q;
        for (int i = 0; i < m; i++) { p = (d * p + (unsigned char)P[i]) % q; t = (d * t + (unsigned char)T[i]) % q; }
        for (int s = 0; s <= n - m; s++) {
            if (p == t) {                                    /* equal hash: verify character by character */
                int j = 0;
                while (j < m) { c++; if (T[s + j] != P[j]) break; j++; }
                if (j == m) { if (verbose) printf(" %d", s); cnt++; } else sp++;
            }
            if (s < n - m) {
                t = (d * (t - (unsigned char)T[s] * h) + (unsigned char)T[s + m]) % q;
                if (t < 0) t += q;
            }
        }
    }
    if (cmps) *cmps = c;
    if (spurious) *spurious = sp;
    return cnt;
}

static int isSuffix(const char *P, int k, int q, int a) {    /* is P[0..k) a suffix of P[0..q) + a ? */
    if (k == 0) return 1;
    if ((unsigned char)P[k - 1] != a) return 0;
    for (int i = 0; i < k - 1; i++) if (P[i] != P[q - (k - 1) + i]) return 0;
    return 1;
}
int automatonSearch(const char *T, const char *P, int verbose) {
    static int tf[32][256];
    if (!okPattern(P)) return -1;
    int n = (int)strlen(T), m = (int)strlen(P), cnt = 0, q = 0;
    for (int s = 0; s <= m; s++)
        for (int a = 0; a < 256; a++) {
            int k = (s + 1 < m) ? s + 1 : m;
            while (k > 0 && !isSuffix(P, k, s, a)) k--;
            tf[s][a] = k;
        }
    for (int i = 0; i < n; i++) {
        q = tf[q][(unsigned char)T[i]];
        if (q == m) { if (verbose) printf(" %d", i - m + 1); cnt++; }
    }
    return cnt;
}

int kmpSearch(const char *T, const char *P, int verbose, long *cmps) {
    if (!okPattern(P)) return -1;
    int n = (int)strlen(T), m = (int)strlen(P), lps[32], k = 0, cnt = 0;
    long c = 0;
    lps[0] = 0;
    for (int i = 1; i < m; i++) {
        while (k > 0 && P[i] != P[k]) k = lps[k - 1];
        if (P[i] == P[k]) k++;
        lps[i] = k;
    }
    k = 0;
    for (int i = 0; i < n; i++) {
        while (k > 0) { c++; if (T[i] == P[k]) break; k = lps[k - 1]; }
        if (k > 0) k++;
        else { c++; if (T[i] == P[0]) k = 1; }
        if (k == m) { if (verbose) printf(" %d", i - m + 1); cnt++; k = lps[k - 1]; }
    }
    if (cmps) *cmps = c;
    return cnt;
}

void demoStrings(void) {
    const char *T = "Phone,Charger,Earbuds,Phone,Earbuds,Charger,Earbuds", *P = "Earbuds";
    long c; int sp, r;
    printf("Text   : %s\nPattern: %s\n", T, P);
    printf("  Naive            positions:"); r = naiveSearch(T, P, 1, &c);      printf("   | matches %d, char comparisons %ld\n", r, c);
    printf("  Rabin-Karp       positions:"); r = rabinKarpSearch(T, P, 1, &c, &sp); printf("   | matches %d, spurious hits %d, char comparisons %ld\n", r, sp, c);
    printf("  Finite Automaton positions:"); r = automatonSearch(T, P, 1);      printf("   | matches %d (one table lookup per text character)\n", r);
    printf("  KMP              positions:"); r = kmpSearch(T, P, 1, &c);        printf("   | matches %d, char comparisons %ld\n", r, c);
    printf("Naive O(nm) worst | Rabin-Karp O(n+m) avg, O(nm) worst | Automaton O(n) matching, O(m^3 sigma) simple build | KMP O(n+m), O(m) space\n");
}

/* ---- Chinese Remainder Theorem: x = r[i] (mod m[i]) for pairwise-coprime moduli ---- */
static long extgcd(long a, long b, long *x, long *y) {
    if (!b) { *x = 1; *y = 0; return a; }
    long x1, y1, g = extgcd(b, a % b, &x1, &y1);
    *x = y1; *y = x1 - (a / b) * y1;
    return g;
}
/* Returns 0 and the smallest non-negative solution x modulo M; 1 if the input is invalid
   (a modulus below 2, moduli not pairwise coprime, or M too large). */
int crtSolve(const long r[], const long m[], int k, long *x, long *M) {
    long prod = 1, sum = 0;
    if (k < 1 || k > 8) return 1;
    for (int i = 0; i < k; i++) {
        if (m[i] < 2 || m[i] > 100000) return 1;
        for (int j = 0; j < i; j++) { long a, b; if (extgcd(m[i], m[j], &a, &b) != 1) return 1; }
        prod *= m[i];
        if (prod > 1000000000L) return 1;
    }
    for (int i = 0; i < k; i++) {
        long Mi = prod / m[i], inv, y;
        extgcd(Mi % m[i], m[i], &inv, &y);
        inv = ((inv % m[i]) + m[i]) % m[i];
        long ri = ((r[i] % m[i]) + m[i]) % m[i];
        sum = (sum + (ri * Mi) % prod * inv) % prod;
    }
    *x = sum; *M = prod;
    return 0;
}
void demoCRT(void) {
    long r[3] = {2, 3, 2}, m[3] = {3, 5, 7}, x, M;
    printf("Solve: x = 2 (mod 3), x = 3 (mod 5), x = 2 (mod 7); moduli pairwise coprime\n");
    if (crtSolve(r, m, 3, &x, &M)) { printf("Invalid input.\n"); return; }
    printf("x = %ld (mod %ld)\n", x, M);
    printf("Idea: a big number is stored as small remainders. Each term is r_i * M_i * (M_i^-1 mod m_i).\n");
    printf("Extended Euclid is O(log m) per inverse; total O(k log M).\n");
    long bad[2] = {4, 6};
    printf("Moduli 4 and 6 (not coprime): %s\n", crtSolve(r, bad, 2, &x, &M) ? "rejected (CRT does not apply)" : "accepted");
}
