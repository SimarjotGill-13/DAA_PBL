#include "common.h"

/* Sellable stock is always COMPUTED, never stored, so it cannot be subtracted twice. */
int sellable(const Inventory *r) { return r->total - r->damaged - r->reserved; }

/* Safe comparison: (x > y) - (x < y) never overflows, unlike x - y. */
static int cmp3(long x, long y) { return (x > y) - (x < y); }
int cmpInt(const void *a, const void *b)       { return cmp3(*(const int *)a, *(const int *)b); }
int cmpProductId(const void *a, const void *b) { return cmp3(((const Product *)a)->id, ((const Product *)b)->id); }
int cmpPinEntry(const void *a, const void *b)  { return cmp3(((const PinEntry *)a)->pin, ((const PinEntry *)b)->pin); }
int cmpInventory(const void *a, const void *b) {
    const Inventory *x = a, *y = b;
    int c = cmp3(x->productId, y->productId);
    return c ? c : cmp3(x->warehouseId, y->warehouseId);
}

/* Merge Sort: stable, O(n log n) in every case, O(n) auxiliary space. Returns 0, or -1 on bad
   arguments or allocation failure (the array may then be partly sorted). */
int mergeSort(void *base, int n, size_t sz, int (*cmp)(const void *, const void *)) {
    if (n < 0 || sz == 0 || !cmp || (!base && n > 0)) return -1;
    if (n < 2) return 0;
    char *a = base;
    int mid = n / 2;
    if (mergeSort(a, mid, sz, cmp) || mergeSort(a + (size_t)mid * sz, n - mid, sz, cmp)) return -1;
    char *tmp = malloc((size_t)n * sz);
    if (!tmp) return -1;
    int i = 0, j = mid, k = 0;
    while (i < mid && j < n) {
        if (cmp(a + (size_t)i * sz, a + (size_t)j * sz) <= 0) { memcpy(tmp + (size_t)k * sz, a + (size_t)i * sz, sz); i++; }
        else                                                  { memcpy(tmp + (size_t)k * sz, a + (size_t)j * sz, sz); j++; }
        k++;
    }
    while (i < mid) { memcpy(tmp + (size_t)k * sz, a + (size_t)i * sz, sz); i++; k++; }
    while (j < n)   { memcpy(tmp + (size_t)k * sz, a + (size_t)j * sz, sz); j++; k++; }
    memcpy(a, tmp, (size_t)n * sz);
    free(tmp);
    return 0;
}

static void swapBytes(char *x, char *y, size_t sz) { while (sz--) { char t = *x; *x++ = *y; *y++ = t; } }

/* Quick Sort (last element as pivot): average O(n log n), worst O(n^2) on sorted input,
   O(log n) average stack space. Returns 0, or -1 on bad arguments. */
int quickSort(void *base, int n, size_t sz, int (*cmp)(const void *, const void *)) {
    if (n < 0 || sz == 0 || !cmp || (!base && n > 0)) return -1;
    if (n < 2) return 0;
    char *a = base, *p = a + (size_t)(n - 1) * sz;
    int i = 0;
    for (int j = 0; j < n - 1; j++)
        if (cmp(a + (size_t)j * sz, p) < 0) { swapBytes(a + (size_t)i * sz, a + (size_t)j * sz, sz); i++; }
    swapBytes(a + (size_t)i * sz, p, sz);
    quickSort(a, i, sz, cmp);
    quickSort(a + (size_t)(i + 1) * sz, n - i - 1, sz, cmp);
    return 0;
}

/* Binary Search on the sorted PIN array: O(log n). Returns the graph node, or -1. */
int findPinNode(int pin, int *cmps) {
    int lo = 0, hi = ds.nPins - 1;
    *cmps = 0;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        (*cmps)++;
        if (ds.pins[mid].pin == pin) return ds.pins[mid].node;
        if (ds.pins[mid].pin < pin) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}
/* Linear Search: O(n), needs no sorting. */
int findPinNodeLinear(int pin, int *cmps) {
    *cmps = 0;
    for (int i = 0; i < ds.nPins; i++) { (*cmps)++; if (ds.pins[i].pin == pin) return ds.pins[i].node; }
    return -1;
}
int findProductBinary(int id) {
    int lo = 0, hi = ds.nProducts - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (ds.products[mid].id == id) return mid;
        if (ds.products[mid].id < id) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}
int findProductLinear(int id) {
    for (int i = 0; i < ds.nProducts; i++) if (ds.products[i].id == id) return i;
    return -1;
}

/* Inventory is sorted by (productId, warehouseId). Lower-bound binary search finds the first
   record of a product in O(log n); the records of one product are then contiguous. */
int invRange(int pid, int *first) {
    int lo = 0, hi = ds.nInv;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (ds.inv[mid].productId < pid) lo = mid + 1; else hi = mid;
    }
    int cnt = 0;
    while (lo + cnt < ds.nInv && ds.inv[lo + cnt].productId == pid) cnt++;
    *first = lo;
    return cnt;
}
int invIndex(int pid, int wid) {
    int first, cnt = invRange(pid, &first);
    for (int k = 0; k < cnt; k++) if (ds.inv[first + k].warehouseId == wid) return first + k;
    return -1;
}

/* The system RECORDS inspection results; it cannot detect damage itself. New damaged units must
   come from units that are neither damaged nor reserved. Returns 0 ok, 1 no record, 2 bad qty, 3 too many. */
int recordDamage(int pid, int wid, int qty) {
    int i = invIndex(pid, wid);
    if (i < 0)    { printf("No inventory record for product %d at warehouse %d.\n", pid, wid); return 1; }
    if (qty <= 0) { printf("Damaged quantity must be at least 1.\n"); return 2; }
    if (qty > sellable(&ds.inv[i])) {
        printf("Rejected: only %d sellable unit(s) can be marked damaged (reserved units are protected).\n", sellable(&ds.inv[i]));
        return 3;
    }
    ds.inv[i].damaged += qty;
    printf("Recorded %d damaged unit(s). Sellable is now %d.\n", qty, sellable(&ds.inv[i]));
    return 0;
}

void printInventory(void) {
    printf("%-7s %-12s %-6s %6s %8s %9s %9s\n", "ProdID", "Product", "WH", "Total", "Damaged", "Reserved", "Sellable");
    for (int i = 0; i < ds.nInv; i++) {
        const Inventory *r = &ds.inv[i];
        int pi = findProductBinary(r->productId);
        int wi = -1;
        for (int w = 0; w < ds.nWarehouses; w++) if (ds.warehouses[w].id == r->warehouseId) wi = w;
        printf("%-7d %-12s %-6s %6d %8d %9d %9d\n", r->productId, pi >= 0 ? ds.products[pi].name : "?",
               wi >= 0 ? ds.warehouses[wi].name : "?", r->total, r->damaged, r->reserved, sellable(r));
    }
}
