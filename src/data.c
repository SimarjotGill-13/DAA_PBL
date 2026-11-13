#include "common.h"
#include <ctype.h>

Dataset ds;
Order orders[MAXORD];
int orderCount;

/* ---------------- validation and finalisation ---------------- */

static int bad(char *err, size_t cap, const char *msg, int a) {
    if (err && cap) snprintf(err, cap, msg, a);
    return -1;
}

/* Checks every rule the algorithms rely on. Returns 0 if the dataset is usable. */
static int validate(const Dataset *d, char *err, size_t cap) {
    if (d->nNodes < 1 || d->nNodes > MAXV) return bad(err, cap, "node count must be 1..%d", MAXV);
    if (d->nProducts < 1)   return bad(err, cap, "at least %d product is required", 1);
    if (d->nWarehouses < 1) return bad(err, cap, "at least %d warehouse is required", 1);
    if (d->nPins < 1)       return bad(err, cap, "at least %d PIN code is required", 1);
    for (int i = 0; i < d->nProducts; i++) {
        if (d->products[i].id <= 0) return bad(err, cap, "product id %d must be positive", d->products[i].id);
        for (int j = 0; j < i; j++)
            if (d->products[j].id == d->products[i].id) return bad(err, cap, "duplicate product id %d", d->products[i].id);
    }
    for (int i = 0; i < d->nWarehouses; i++) {
        const Warehouse *w = &d->warehouses[i];
        if (w->id <= 0) return bad(err, cap, "warehouse id %d must be positive", w->id);
        if (w->node < 0 || w->node >= d->nNodes) return bad(err, cap, "warehouse %d has an invalid node", w->id);
        if (w->fee < 0 || w->fee > MAXWEIGHT) return bad(err, cap, "warehouse %d has an invalid fee", w->id);
        for (int j = 0; j < i; j++)
            if (d->warehouses[j].id == w->id) return bad(err, cap, "duplicate warehouse id %d", w->id);
    }
    for (int i = 0; i < d->nInv; i++) {
        const Inventory *r = &d->inv[i];
        int pOk = 0, wOk = 0;
        for (int p = 0; p < d->nProducts; p++) if (d->products[p].id == r->productId) pOk = 1;
        for (int w = 0; w < d->nWarehouses; w++) if (d->warehouses[w].id == r->warehouseId) wOk = 1;
        if (!pOk) return bad(err, cap, "inventory refers to unknown product %d", r->productId);
        if (!wOk) return bad(err, cap, "inventory refers to unknown warehouse %d", r->warehouseId);
        if (r->total < 0 || r->total > MAXTOTAL || r->damaged < 0 || r->reserved < 0 || r->damaged + r->reserved > r->total)
            return bad(err, cap, "inventory for product %d has inconsistent stock numbers", r->productId);
        for (int j = 0; j < i; j++)
            if (d->inv[j].productId == r->productId && d->inv[j].warehouseId == r->warehouseId)
                return bad(err, cap, "duplicate inventory record for product %d", r->productId);
    }
    for (int i = 0; i < d->nPins; i++) {
        if (d->pins[i].pin < 100000 || d->pins[i].pin > 999999) return bad(err, cap, "PIN %d must have 6 digits", d->pins[i].pin);
        if (d->pins[i].node < 0 || d->pins[i].node >= d->nNodes) return bad(err, cap, "PIN %d has an invalid node", d->pins[i].pin);
        for (int j = 0; j < i; j++)
            if (d->pins[j].pin == d->pins[i].pin) return bad(err, cap, "duplicate PIN %d", d->pins[i].pin);
    }
    if (d->roads.n != d->nNodes) return bad(err, cap, "road graph size mismatch (%d)", d->roads.n);
    if (graphHasNegativeCycle(&d->roads)) return bad(err, cap, "road network contains a negative cycle (%d)", 0);
    return 0;
}

/* Sorts the arrays that the algorithms binary-search, then fills the demand matrix. */
static int finalize(Dataset *d, int dem[][3], int ndem, char *err, size_t cap) {
    if (mergeSort(d->products, d->nProducts, sizeof(Product), cmpProductId) ||
        mergeSort(d->pins, d->nPins, sizeof(PinEntry), cmpPinEntry) ||
        mergeSort(d->inv, d->nInv, sizeof(Inventory), cmpInventory))
        return bad(err, cap, "out of memory while sorting (%d)", 0);
    memset(d->weeklyDemand, 0, sizeof d->weeklyDemand);
    for (int k = 0; k < ndem; k++) {
        int pi = -1, wi = -1;
        for (int p = 0; p < d->nProducts; p++) if (d->products[p].id == dem[k][0]) pi = p;
        for (int w = 0; w < d->nWarehouses; w++) if (d->warehouses[w].id == dem[k][1]) wi = w;
        if (pi < 0 || wi < 0) return bad(err, cap, "demand refers to unknown product or warehouse (%d)", dem[k][0]);
        if (dem[k][2] < 0 || dem[k][2] > MAXTOTAL) return bad(err, cap, "demand for product %d is out of range", dem[k][0]);
        d->weeklyDemand[pi][wi] = dem[k][2];
    }
    return 0;
}

/* ---------------- built-in dataset ---------------- */
/* 9 nodes: W1 W2 W3 (warehouses), HubA HubB HubC (hubs), CustX CustY (customers), CustZ (no road). */

int loadBuiltinData(void) {
    static Dataset d;
    static const char *names[9] = {"W1", "W2", "W3", "HubA", "HubB", "HubC", "CustX", "CustY", "CustZ"};
    static const Product P[3] = {{103, "Charger"}, {101, "Phone"}, {102, "Earbuds"}};      /* unsorted on purpose */
    static const Warehouse W[3] = {{1, "W1", 0, 5}, {2, "W2", 1, 2}, {3, "W3", 2, 4}};
    static const Inventory I[9] = {{101, 1, 10, 0, 0}, {101, 2, 0, 0, 0}, {101, 3, 5, 0, 0},
                                   {102, 1, 1, 0, 0},  {102, 2, 8, 0, 0}, {102, 3, 0, 0, 0},
                                   {103, 1, 6, 2, 0},  {103, 2, 6, 0, 0}, {103, 3, 6, 0, 0}};
    static const PinEntry N[3] = {{100003, 8}, {100001, 6}, {100002, 7}};
    static int dem[9][3] = {{101, 1, 2}, {101, 2, 6}, {101, 3, 1}, {102, 1, 1}, {102, 2, 1},
                            {102, 3, 3}, {103, 1, 2}, {103, 2, 2}, {103, 3, 2}};
    char err[96];
    memset(&d, 0, sizeof d);
    d.nNodes = 9; d.nProducts = 3; d.nWarehouses = 3; d.nInv = 9; d.nPins = 3;
    for (int i = 0; i < 9; i++) snprintf(d.nodeName[i], NAMELEN, "%s", names[i]);
    memcpy(d.products, P, sizeof P); memcpy(d.warehouses, W, sizeof W);
    memcpy(d.inv, I, sizeof I);      memcpy(d.pins, N, sizeof N);
    graphInit(&d.roads, 9);
    graphAddRoad(&d.roads, 0, 3, 4); graphAddRoad(&d.roads, 1, 4, 6); graphAddRoad(&d.roads, 2, 5, 5);
    graphAddRoad(&d.roads, 3, 4, 3); graphAddRoad(&d.roads, 4, 5, 2);
    graphAddRoad(&d.roads, 3, 6, 2); graphAddRoad(&d.roads, 4, 6, 5);
    graphAddRoad(&d.roads, 5, 7, 3); graphAddRoad(&d.roads, 4, 7, 7);
    if (finalize(&d, dem, 9, err, sizeof err) || validate(&d, err, sizeof err)) {
        fprintf(stderr, "Built-in dataset error: %s\n", err);
        return -1;
    }
    ds = d;
    orderCount = 0;
    memset(orders, 0, sizeof orders);
    return 0;
}

/* ---------------- dataset file loader ---------------- */
/* Format (one record per line, '#' starts a comment):
     nodes N | node I NAME | road A B W (two-way) | arc A B W (one-way, W may be negative)
     product ID NAME | warehouse ID NAME NODE FEE | inventory PID WID TOTAL DAMAGED RESERVED
     pin PIN NODE | demand PID WID WEEKLY                                                    */

static int onlySpace(const char *s) { while (*s) { if (!isspace((unsigned char)*s)) return 0; s++; } return 1; }

int loadDatasetFile(const char *path, char *err, size_t cap) {
    static Dataset t;
    static int dem[MAXI][3];
    char line[256], kw[16];
    int ndem = 0, lineNo = 0, haveNodes = 0, pos = 0;
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, cap, "cannot open '%s'", path); return -1; }
    memset(&t, 0, sizeof t);
    graphInit(&t.roads, 0);
#define FAIL(...) do { int _n = snprintf(err, cap, "line %d: ", lineNo); if ((size_t)_n < cap) snprintf(err + _n, cap - (size_t)_n, __VA_ARGS__); fclose(f); return -1; } while (0)
    while (fgets(line, sizeof line, f)) {
        lineNo++;
        if (!strchr(line, '\n') && !feof(f)) FAIL("line is too long");
        char *hash = strchr(line, '#');
        if (hash) *hash = '\0';
        if (sscanf(line, "%15s%n", kw, &pos) != 1) continue;           /* blank line */
        const char *rest = line + pos;
        int a, b, c, e, g, n = 0;
        char name[NAMELEN];
        if (strcmp(kw, "nodes") == 0) {
            if (sscanf(rest, "%d %n", &a, &n) != 1 || !onlySpace(rest + n)) FAIL("expected: nodes N");
            if (haveNodes) FAIL("nodes given twice");
            if (a < 1 || a > MAXV) FAIL("node count must be 1..%d", MAXV);
            t.nNodes = a; haveNodes = 1; graphInit(&t.roads, a);
            for (int i = 0; i < a; i++) snprintf(t.nodeName[i], NAMELEN, "N%d", i);
            continue;
        }
        if (!haveNodes) FAIL("'nodes N' must come first");
        if (strcmp(kw, "node") == 0) {
            if (sscanf(rest, "%d %15s %n", &a, name, &n) != 2 || !onlySpace(rest + n)) FAIL("expected: node INDEX NAME");
            if (a < 0 || a >= t.nNodes) FAIL("node index out of range");
            snprintf(t.nodeName[a], NAMELEN, "%s", name);
        } else if (strcmp(kw, "road") == 0 || strcmp(kw, "arc") == 0) {
            if (sscanf(rest, "%d %d %d %n", &a, &b, &c, &n) != 3 || !onlySpace(rest + n)) FAIL("expected: %s FROM TO WEIGHT", kw);
            int r = (kw[0] == 'r') ? graphAddRoad(&t.roads, a, b, c) : graphAddArc(&t.roads, a, b, c);
            if (r == 1) FAIL("invalid node in road");
            if (r == 2) FAIL("weight must be within +/-%d", MAXWEIGHT);
            if (r == 3) FAIL("this road would create a negative cycle");
            if (r == 4) FAIL("a two-way road cannot be negative; use 'arc' for a one-way road");
        } else if (strcmp(kw, "product") == 0) {
            if (sscanf(rest, "%d %15s %n", &a, name, &n) != 2 || !onlySpace(rest + n)) FAIL("expected: product ID NAME");
            if (t.nProducts >= MAXP) FAIL("too many products (max %d)", MAXP);
            t.products[t.nProducts].id = a; snprintf(t.products[t.nProducts].name, NAMELEN, "%s", name); t.nProducts++;
        } else if (strcmp(kw, "warehouse") == 0) {
            if (sscanf(rest, "%d %15s %d %d %n", &a, name, &b, &c, &n) != 4 || !onlySpace(rest + n)) FAIL("expected: warehouse ID NAME NODE FEE");
            if (t.nWarehouses >= MAXW) FAIL("too many warehouses (max %d)", MAXW);
            Warehouse *w = &t.warehouses[t.nWarehouses++];
            w->id = a; w->node = b; w->fee = c; snprintf(w->name, NAMELEN, "%s", name);
        } else if (strcmp(kw, "inventory") == 0) {
            if (sscanf(rest, "%d %d %d %d %d %n", &a, &b, &c, &e, &g, &n) != 5 || !onlySpace(rest + n)) FAIL("expected: inventory PID WID TOTAL DAMAGED RESERVED");
            if (t.nInv >= MAXI) FAIL("too many inventory records (max %d)", MAXI);
            t.inv[t.nInv++] = (Inventory){a, b, c, e, g};
        } else if (strcmp(kw, "pin") == 0) {
            if (sscanf(rest, "%d %d %n", &a, &b, &n) != 2 || !onlySpace(rest + n)) FAIL("expected: pin PIN NODE");
            if (t.nPins >= MAXPIN) FAIL("too many PIN codes (max %d)", MAXPIN);
            t.pins[t.nPins++] = (PinEntry){a, b};
        } else if (strcmp(kw, "demand") == 0) {
            if (sscanf(rest, "%d %d %d %n", &a, &b, &c, &n) != 3 || !onlySpace(rest + n)) FAIL("expected: demand PID WID WEEKLY");
            if (ndem >= MAXI) FAIL("too many demand lines");
            dem[ndem][0] = a; dem[ndem][1] = b; dem[ndem][2] = c; ndem++;
        } else {
            FAIL("unknown keyword '%s'", kw);
        }
    }
#undef FAIL
    fclose(f);
    if (!haveNodes) { snprintf(err, cap, "file has no 'nodes' line"); return -1; }
    if (validate(&t, err, cap) || finalize(&t, dem, ndem, err, cap)) return -1;
    ds = t;                                   /* only replaced after everything checked out */
    orderCount = 0;
    memset(orders, 0, sizeof orders);
    return 0;
}
