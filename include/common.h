#ifndef COMMON_H
#define COMMON_H

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- compile-time capacities ---- */
#define MAXV      32      /* graph nodes */
#define MAXP      16      /* products */
#define MAXW       8      /* warehouses */
#define MAXI     128      /* inventory records */
#define MAXPIN    32      /* serviceable PIN codes */
#define MAXORD   200      /* order log */
#define NAMELEN   16
#define MAXD      64      /* deliveries in one schedule */
#define MAXN      12      /* knapsack items */
#define MAXCAP   200      /* knapsack capacity */
#define SN         8      /* largest Strassen matrix */
#define MAXTSP     8      /* largest TSP instance */
#define FLOWN      8      /* nodes in the flow demo */
#define MAXM       8      /* matching demo size */
#define MAXTOTAL 1000000  /* largest stock total accepted */

/* ---- model constants ---- */
#define INF        1000000
#define NOEDGE     INT_MAX   /* "no road"; lets a road of length 0 exist */
#define MAXWEIGHT  10000     /* |road weight| limit: keeps every path sum inside int */
#define RATE       1         /* delivery cost per distance unit */
#define SAFETY     2         /* safety stock per product per warehouse */
#define HORIZON    2         /* weeks of demand a warehouse should cover */
#define BENEFIT    5         /* value of one transferred unit: caps transfer cost */

typedef struct { int id; char name[NAMELEN]; } Product;               /* id is globally unique */
typedef struct { int id; char name[NAMELEN]; int node; int fee; } Warehouse;
typedef struct { int productId, warehouseId, total, damaged, reserved; } Inventory;
typedef struct { int pin, node; } PinEntry;
typedef struct { int id, start, end; } Delivery;                       /* minutes since midnight */
typedef struct { char name[8]; int w, v; } Item;                       /* knapsack parcel */
typedef struct { int n; int w[MAXV][MAXV]; } Graph;                    /* w[u][v] = NOEDGE if no road u->v */
typedef struct { int n, m; int *start, *to, *w; } Csr;                 /* sparse graph for the stress test */
typedef int Mat[SN][SN];

typedef enum { ALGO_NONE, ALGO_DIJKSTRA, ALGO_BELLMAN_FORD } RouteAlgo;
enum { ORD_OK, ORD_BAD_QTY, ORD_BAD_PIN, ORD_BAD_PRODUCT, ORD_INSUFFICIENT,
       ORD_NO_SINGLE_WH, ORD_UNREACHABLE, ORD_ROUTING_ERROR };
typedef struct { int id, pin, productId, qty, status, wh, dist, cost; RouteAlgo algo; } Order;

typedef struct {
    int nNodes, nProducts, nWarehouses, nInv, nPins;
    char nodeName[MAXV][NAMELEN];
    Graph roads;
    Product products[MAXP];          /* sorted by id */
    Warehouse warehouses[MAXW];
    Inventory inv[MAXI];             /* sorted by (productId, warehouseId) */
    PinEntry pins[MAXPIN];           /* sorted by pin */
    int weeklyDemand[MAXP][MAXW];    /* [product index][warehouse index] */
} Dataset;

extern Dataset ds;
extern Order orders[MAXORD];
extern int orderCount;

/* ---- main.c: validated input ---- */
int parseIntStrict(const char *s, long lo, long hi, long *out);
int parseDoubleStrict(const char *s, double lo, double hi, double *out);
int readLine(char *buf, size_t cap);
int readIntRange(const char *prompt, long lo, long hi, long *out);
int readDoubleRange(const char *prompt, double lo, double hi, double *out);

/* ---- data.c ---- */
int loadBuiltinData(void);
int loadDatasetFile(const char *path, char *err, size_t cap);

/* ---- inventory.c ---- */
int  sellable(const Inventory *r);
int  cmpInt(const void *a, const void *b);
int  cmpProductId(const void *a, const void *b);
int  cmpPinEntry(const void *a, const void *b);
int  cmpInventory(const void *a, const void *b);
int  mergeSort(void *base, int n, size_t sz, int (*cmp)(const void *, const void *));
int  quickSort(void *base, int n, size_t sz, int (*cmp)(const void *, const void *));
int  findPinNode(int pin, int *cmps);
int  findPinNodeLinear(int pin, int *cmps);
int  findProductBinary(int id);
int  findProductLinear(int id);
int  invRange(int pid, int *first);
int  invIndex(int pid, int wid);
int  recordDamage(int pid, int wid, int qty);
void printInventory(void);

/* ---- graph.c ---- */
void graphInit(Graph *g, int n);
int  graphAddArc(Graph *g, int u, int v, int w);
int  graphAddRoad(Graph *g, int a, int b, int w);
int  graphHasNegative(const Graph *g);
int  graphHasNegativeCycle(const Graph *g);
int  graphIsUndirectedNonNegative(const Graph *g);
void graphTranspose(const Graph *g, Graph *t);
void dijkstra(const Graph *g, int src, int dist[], int parent[]);
int  bellmanFord(const Graph *g, int src, int dist[], int parent[]);
int  floydWarshall(const Graph *g, int d[MAXV][MAXV]);
int  shortestPaths(const Graph *g, int src, int dist[], int parent[], RouteAlgo *used);
int  distancesToNode(int target, int dist[], int parent[], RouteAlgo *used);
const char *algoName(RouteAlgo a);
int  nodeByName(const char *name);
void printPath(const int parent[], int v);
void printRouteToTarget(const int parent[], int from);
void compareRouting(int src);
void demoNegativeWeights(void);
int  csrBuildRandom(Csr *c, int n, int m, unsigned seed, int maxW);
void csrFree(Csr *c);
int  dijkstraCsrArray(const Csr *c, int src, int *dist);
int  dijkstraCsrHeap(const Csr *c, int src, int *dist);

/* ---- fulfil.c ---- */
int  placeOrder(int pin, int pid, int qty, int verbose, Order *out);
const char *statusText(int s);
void printOrders(void);

/* ---- schedule.c ---- */
int    activitySelection(const Delivery in[], int n, Delivery chosen[]);
int    intervalPartition(const Delivery in[], int n, Delivery sorted[], int agentOf[]);
double knapFractional(const Item it[], int n, int cap);
int    knapDP(const Item it[], int n, int cap, char chosen[], size_t chosenCap);
int    knapBacktrack(const Item it[], int n, int cap, long *calls);
int    knapBB(const Item it[], int n, int cap, int mode, long *expanded);
void   demoScheduling(void);
void   demoKnapsack(void);

/* ---- redistribute.c ---- */
int recommendTransfers(int verbose);

/* ---- demos_greedy_dc.c ---- */
int       maxSubarray(const int a[], int n, int *lo, int *hi);
long long powerDC(long long x, int n);
long long modPowerDC(long long x, long long n, long long m);
int       celebrityFind(const int *knows, int n);
int       strassenMultiply(int n, Mat a, Mat b, Mat c, long *mults);
int       standardMultiply(int n, Mat a, Mat b, Mat c, long *mults);
int       huffmanBits(const char *text, int verbose);
int       mstPrim(const Graph *g, int verbose);
int       mstKruskal(const Graph *g, int verbose);
void demoMaxSubarray(void);
void demoPower(void);
void demoCelebrity(void);
void demoStrassen(void);
void demoHuffman(void);
void demoMST(void);

/* ---- demos_dp_bt.c ---- */
int  lcsLength(const char *a, const char *b, char *out, size_t cap);
int  matrixChainMin(const int p[], int n, char *paren, size_t cap);
int  obstMin(const int freq[], int n);
int  tspSolveAll(const Graph *g, const int stops[], int k, int verbose, int result[3]);
int  nQueensCount(int n, int showFirst);
int  subsetSumCount(const int w[], int n, int target, int verbose);
void demoLCS(void);
void demoMatrixChain(void);
void demoOBST(void);
void demoTSP(void);
void demoNQueens(void);
void demoSubsetSum(void);

/* ---- demos_flow_string.c ---- */
int  maxFlow(int n, int cap[FLOWN][FLOWN], int s, int t, int flow[FLOWN][FLOWN]);
int  bipartiteMatching(int nl, int nr, int elig[MAXM][MAXM], int matchR[MAXM]);
int  naiveSearch(const char *T, const char *P, int verbose, long *cmps);
int  rabinKarpSearch(const char *T, const char *P, int verbose, long *cmps, int *spurious);
int  automatonSearch(const char *T, const char *P, int verbose);
int  kmpSearch(const char *T, const char *P, int verbose, long *cmps);
int  crtSolve(const long r[], const long m[], int k, long *x, long *M);
void demoMaxFlow(void);
void demoMatching(void);
void demoStrings(void);
void demoCRT(void);

/* ---- complexity.c ---- */
int  masterCase(int a, int b, double c, double *exponent);
void demoComplexity(void);
void demoRecurrences(void);
void demoMasterTheorem(void);
void demoStress(void);
void showTheory(void);

/* ---- tests.c ---- */
int  runTests(void);
void runScenarios(void);

#endif
