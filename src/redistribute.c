#include "common.h"

typedef struct { int w, deficit; } Need;

static int cmpNeedDesc(const void *a, const void *b) {         /* largest deficit first, then lower index */
    const Need *x = a, *y = b;
    int c = (y->deficit > x->deficit) - (y->deficit < x->deficit);
    return c ? c : (x->w > y->w) - (x->w < y->w);
}

/* Recommends stock transfers; NEVER changes inventory.
   surplus = sellable - safety - demand*horizon       (stock a source can spare)
   deficit = demand*horizon + safety - sellable       (stock a destination lacks)
   sellable already excludes damaged and reserved units.
   Greedy: serve the largest deficit first; for each, try the cheapest source (shortest distance,
   ties -> larger surplus). A transfer is recommended only if cost <= units x BENEFIT.
   Returns the number of recommendations, or -1 on failure. */
int recommendTransfers(int verbose) {
    int count = 0;
    if (verbose) printf("Safety stock %d, horizon %d weeks, value per unit moved %d\n\n", SAFETY, HORIZON, BENEFIT);
    for (int pi = 0; pi < ds.nProducts; pi++) {
        int pid = ds.products[pi].id, sur[MAXW], def[MAXW];
        Need need[MAXW];
        int nn = 0;
        for (int w = 0; w < ds.nWarehouses; w++) {
            int k = invIndex(pid, ds.warehouses[w].id);
            int sell = (k < 0) ? 0 : sellable(&ds.inv[k]);
            int req = ds.weeklyDemand[pi][w] * HORIZON + SAFETY;
            sur[w] = sell > req ? sell - req : 0;
            def[w] = req > sell ? req - sell : 0;
            if (def[w] > 0) need[nn++] = (Need){w, def[w]};
        }
        if (mergeSort(need, nn, sizeof(Need), cmpNeedDesc)) return -1;
        for (int q = 0; q < nn; q++) {
            int d = need[q].w, dist[MAXV], par[MAXV], used[MAXW] = {0};
            if (distancesToNode(ds.warehouses[d].node, dist, par, NULL)) continue;
            while (def[d] > 0) {
                int s = -1;
                for (int w = 0; w < ds.nWarehouses; w++) {
                    if (w == d || used[w] || sur[w] == 0 || dist[ds.warehouses[w].node] >= INF) continue;
                    int cw = dist[ds.warehouses[w].node], cs = s < 0 ? 0 : dist[ds.warehouses[s].node];
                    if (s < 0 || cw < cs || (cw == cs && sur[w] > sur[s])) s = w;
                }
                if (s < 0) break;
                used[s] = 1;
                int qty = sur[s] < def[d] ? sur[s] : def[d];
                int cost = dist[ds.warehouses[s].node] * RATE;
                if (cost < 0) cost = 0;
                if (cost <= qty * BENEFIT) {
                    if (verbose) printf("RECOMMEND: move %d x %s from %s to %s (transfer cost %d <= benefit %d)\n", qty,
                                        ds.products[pi].name, ds.warehouses[s].name, ds.warehouses[d].name, cost, qty * BENEFIT);
                    sur[s] -= qty; def[d] -= qty; count++;
                } else if (verbose) {
                    printf("skip: %s -> %s for %s (cost %d > benefit %d)\n", ds.warehouses[s].name, ds.warehouses[d].name,
                           ds.products[pi].name, cost, qty * BENEFIT);
                }
            }
            if (verbose && def[d] > 0)
                printf("note: %s still short by %d unit(s) of %s after recommendations\n", ds.warehouses[d].name, def[d], ds.products[pi].name);
        }
    }
    if (verbose) printf("\n%d recommendation(s). Nothing was executed; stock is unchanged.\n", count);
    return count;
}
