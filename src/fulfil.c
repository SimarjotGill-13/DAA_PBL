#include "common.h"

const char *statusText(int s) {
    switch (s) {
        case ORD_OK:            return "SUCCESS";
        case ORD_BAD_QTY:       return "Invalid quantity (must be at least 1)";
        case ORD_BAD_PIN:       return "PIN code is not serviceable";
        case ORD_BAD_PRODUCT:   return "Invalid product ID";
        case ORD_INSUFFICIENT:  return "Insufficient sellable stock across all warehouses";
        case ORD_NO_SINGLE_WH:  return "Stock exists, but no single warehouse can fulfil the full quantity";
        case ORD_UNREACHABLE:   return "Destination unreachable from every eligible warehouse";
        case ORD_ROUTING_ERROR: return "Road network has a negative cycle, so routes are undefined";
    }
    return "Unknown";
}

static int findWh(int id) {
    for (int i = 0; i < ds.nWarehouses; i++) if (ds.warehouses[i].id == id) return i;
    return -1;
}

static int finish(Order *o, int status, int verbose, Order *out) {
    o->status = status;
    if (orderCount < MAXORD) orders[orderCount++] = *o;
    if (out) *out = *o;
    if (verbose && status != ORD_OK) printf("Order #%d FAILED: %s\n", o->id, statusText(status));
    return status;
}

/* Policy: first come, first served; ONE warehouse fulfils the whole order.
   Stock is reserved only after a warehouse has been chosen.
   Step algorithms: Binary Search (PIN, product, inventory range) -> sellable filter ->
   Dijkstra or Bellman-Ford chosen by the road data -> linear minimum scan with tie-break. */
int placeOrder(int pin, int pid, int qty, int verbose, Order *out) {
    Order o = {orderCount + 1, pin, pid, qty, ORD_OK, -1, 0, 0, ALGO_NONE};
    int cmps = 0;
    if (verbose) printf("\n--- Order #%d: PIN %d, product %d, quantity %d ---\n", o.id, pin, pid, qty);
    if (qty <= 0) return finish(&o, ORD_BAD_QTY, verbose, out);

    int node = findPinNode(pin, &cmps);
    if (verbose) printf("1. Serviceability (binary search, %d comparisons): %s\n", cmps, node >= 0 ? "serviceable" : "NOT serviceable");
    if (node < 0) return finish(&o, ORD_BAD_PIN, verbose, out);

    int pi = findProductBinary(pid);
    if (verbose) printf("2. Product lookup (binary search): %s\n", pi >= 0 ? ds.products[pi].name : "not found");
    if (pi < 0) return finish(&o, ORD_BAD_PRODUCT, verbose, out);

    int first, count = invRange(pid, &first), cand[MAXW], nc = 0;
    long totalSell = 0;
    if (verbose) printf("3. Sellable = total - damaged - reserved:\n");
    for (int k = 0; k < count; k++) {
        const Inventory *r = &ds.inv[first + k];
        int s = sellable(r);
        totalSell += s;
        if (s >= qty && nc < MAXW) cand[nc++] = first + k;
        if (verbose) printf("   %s: %d - %d - %d = %d  %s\n", ds.warehouses[findWh(r->warehouseId)].name, r->total,
                            r->damaged, r->reserved, s, s >= qty ? "(eligible)" : "(rejected: too little)");
    }
    if (totalSell < qty) return finish(&o, ORD_INSUFFICIENT, verbose, out);
    if (nc == 0)         return finish(&o, ORD_NO_SINGLE_WH, verbose, out);

    int dist[MAXV], parent[MAXV], bi = -1, bw = -1, bd = 0, bc = 0;
    RouteAlgo algo = ALGO_NONE;
    if (distancesToNode(node, dist, parent, &algo)) return finish(&o, ORD_ROUTING_ERROR, verbose, out);
    o.algo = algo;
    if (verbose) printf("4. Routing with %s (%s); cost = distance x %d + handling fee:\n", algoName(algo),
                        algo == ALGO_DIJKSTRA ? "all roads non-negative" : "a negative road is present", RATE);
    for (int c = 0; c < nc; c++) {
        int w = findWh(ds.inv[cand[c]].warehouseId), d = dist[ds.warehouses[w].node];
        if (d >= INF) { if (verbose) printf("   %s: unreachable\n", ds.warehouses[w].name); continue; }
        int cost = d * RATE + ds.warehouses[w].fee;
        if (cost < 0) cost = 0;                          /* a net cost is never below zero */
        if (verbose) printf("   %s: %d x %d + %d = %d\n", ds.warehouses[w].name, d, RATE, ds.warehouses[w].fee, cost);
        /* tie-break: lower cost, then shorter distance, then lower warehouse ID */
        if (bi == -1 || cost < bc || (cost == bc && (d < bd || (d == bd && ds.warehouses[w].id < ds.warehouses[bw].id)))) {
            bi = cand[c]; bw = w; bd = d; bc = cost;
        }
    }
    if (bi == -1) return finish(&o, ORD_UNREACHABLE, verbose, out);

    ds.inv[bi].reserved += qty;                          /* reserve only after successful allocation */
    o.wh = ds.warehouses[bw].id; o.dist = bd; o.cost = bc;
    if (verbose) {
        printf("5. SELECTED warehouse %s, cost %d\n   Route: ", ds.warehouses[bw].name, bc);
        printRouteToTarget(parent, ds.warehouses[bw].node);
        printf("  (distance %d)\n", bd);
        printf("   Updated stock at %s: total %d, damaged %d, reserved %d, sellable %d\n", ds.warehouses[bw].name,
               ds.inv[bi].total, ds.inv[bi].damaged, ds.inv[bi].reserved, sellable(&ds.inv[bi]));
        printf("Order #%d SUCCESS\n", o.id);
    }
    return finish(&o, ORD_OK, 0, out);
}

void printOrders(void) {
    if (!orderCount) { printf("No orders yet.\n"); return; }
    printf("%-4s %-8s %-7s %-4s %-6s %-5s %s\n", "ID", "PIN", "Product", "Qty", "WH", "Cost", "Result");
    for (int i = 0; i < orderCount; i++) {
        const Order *o = &orders[i];
        printf("%-4d %-8d %-7d %-4d ", o->id, o->pin, o->productId, o->qty);
        if (o->status == ORD_OK) printf("W%-5d %-5d ", o->wh, o->cost); else printf("%-6s %-5s ", "-", "-");
        printf("%s\n", statusText(o->status));
    }
}
