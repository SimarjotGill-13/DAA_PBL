#include "common.h"
#include <ctype.h>
#include <errno.h>

/* ---------------- validated input ---------------- */

static int onlySpace(const char *s) { while (*s) { if (!isspace((unsigned char)*s)) return 0; s++; } return 1; }

/* Accepts optional spaces, an optional sign and digits only. Rejects empty text, trailing characters,
   overflow and values outside [lo, hi]. Returns 1 if valid. */
int parseIntStrict(const char *s, long lo, long hi, long *out) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || errno == ERANGE || !onlySpace(end) || v < lo || v > hi) return 0;
    *out = v;
    return 1;
}

int parseDoubleStrict(const char *s, double lo, double hi, double *out) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;
    errno = 0;
    double v = strtod(s, &end);
    if (end == s || errno == ERANGE || !onlySpace(end) || !(v >= lo && v <= hi)) return 0;
    *out = v;
    return 1;
}

/* Reads one line. 1 = ok, 0 = line longer than the buffer (the rest is discarded), -1 = end of input. */
int readLine(char *buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) return -1;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') return 1;
    int c, tooLong = 0;
    while ((c = getchar()) != '\n' && c != EOF) tooLong = 1;
    return tooLong ? 0 : 1;
}

/* 1 = ok, 0 = invalid (message printed), -1 = end of input */
int readIntRange(const char *prompt, long lo, long hi, long *out) {
    char buf[64];
    printf("%s", prompt);
    int r = readLine(buf, sizeof buf);
    if (r == -1) return -1;
    if (r == 0) { printf("Input too long.\n"); return 0; }
    if (!parseIntStrict(buf, lo, hi, out)) { printf("Enter a whole number between %ld and %ld.\n", lo, hi); return 0; }
    return 1;
}

int readDoubleRange(const char *prompt, double lo, double hi, double *out) {
    char buf[64];
    printf("%s", prompt);
    int r = readLine(buf, sizeof buf);
    if (r == -1) return -1;
    if (r == 0) { printf("Input too long.\n"); return 0; }
    if (!parseDoubleStrict(buf, lo, hi, out)) { printf("Enter a number between %g and %g.\n", lo, hi); return 0; }
    return 1;
}

/* ---------------- menu actions ---------------- */

static void placeOrderMenu(void) {
    long pin, pid, qty;
    printf("Serviceable PINs:");
    for (int i = 0; i < ds.nPins; i++) printf(" %d(%s)", ds.pins[i].pin, ds.nodeName[ds.pins[i].node]);
    printf("\nProducts:");
    for (int i = 0; i < ds.nProducts; i++) printf(" %d(%s)", ds.products[i].id, ds.products[i].name);
    printf("\n");
    if (readIntRange("PIN code: ", 1, 999999999, &pin) != 1 || readIntRange("Product ID: ", 1, 999999999, &pid) != 1 ||
        readIntRange("Quantity: ", 1, MAXTOTAL, &qty) != 1) return;
    placeOrder((int)pin, (int)pid, (int)qty, 1, NULL);
}

static void damageMenu(void) {
    long pid, wid, qty;
    if (readIntRange("Product ID: ", 1, 999999999, &pid) != 1 || readIntRange("Warehouse ID: ", 1, 999999999, &wid) != 1 ||
        readIntRange("Damaged units found in inspection: ", 1, MAXTOTAL, &qty) != 1) return;
    recordDamage((int)pid, (int)wid, (int)qty);
}

static void routeMenu(void) {
    long src;
    printf("Nodes:");
    for (int i = 0; i < ds.nNodes; i++) printf(" %d=%s", i, ds.nodeName[i]);
    printf("\n");
    if (readIntRange("Source node: ", 0, ds.nNodes - 1, &src) != 1) return;
    compareRouting((int)src);
}

static void roadMenu(void) {
    long a, b, w, kind;
    printf("Nodes:");
    for (int i = 0; i < ds.nNodes; i++) printf(" %d=%s", i, ds.nodeName[i]);
    printf("\nA negative weight (a rebate or credit) is allowed only on a ONE-WAY road and only if it creates no negative cycle.\n");
    if (readIntRange("From node: ", 0, ds.nNodes - 1, &a) != 1 || readIntRange("To node: ", 0, ds.nNodes - 1, &b) != 1 ||
        readIntRange("Weight (-10000..10000): ", -MAXWEIGHT, MAXWEIGHT, &w) != 1 ||
        readIntRange("1 = one-way, 2 = two-way: ", 1, 2, &kind) != 1) return;
    int r = (kind == 1) ? graphAddArc(&ds.roads, (int)a, (int)b, (int)w) : graphAddRoad(&ds.roads, (int)a, (int)b, (int)w);
    static const char *msg[5] = {"Road saved.", "Invalid nodes (a road needs two different nodes).", "Weight out of range.",
                                 "Rejected: this road would create a negative cycle, so shortest routes would be undefined.",
                                 "Rejected: a two-way road cannot be negative (it is a negative cycle by itself). Use a one-way road."};
    printf("%s\n", msg[r]);
    if (r == 0) printf("Routing will now use %s.\n", graphHasNegative(&ds.roads) ? "Bellman-Ford (a negative road exists)" : "Dijkstra (all roads non-negative)");
}

static void loadMenu(void) {
    char path[256], err[160];
    printf("Dataset files are in the data/ folder, for example data/extended_dataset.txt\nFile path: ");
    int r = readLine(path, sizeof path);
    if (r != 1) { printf("%s\n", r == 0 ? "Path too long." : "No input."); return; }
    size_t l = strlen(path);
    while (l > 0 && isspace((unsigned char)path[l - 1])) path[--l] = '\0';
    if (l == 0) { printf("No path given.\n"); return; }
    if (loadDatasetFile(path, err, sizeof err) == 0)
        printf("Loaded: %d nodes, %d products, %d warehouses, %d inventory records, %d PIN codes. Order log cleared.\n",
               ds.nNodes, ds.nProducts, ds.nWarehouses, ds.nInv, ds.nPins);
    else printf("Not loaded (current data kept): %s\n", err);
}

static void extrasMenu(void) {
    for (;;) {
        long c;
        printf("\n--- Supplementary algorithm demos ---\n"
               " Divide and Conquer : 1 Max Subarray   2 Powering   3 Celebrity   4 Strassen\n"
               " Greedy             : 5 Huffman        6 Prim vs Kruskal\n"
               " Dynamic Programming: 7 LCS   8 Matrix Chain   9 Optimal BST   10 TSP (DP, B&B, brute force)\n"
               " Backtracking       : 11 N-Queens   12 Sum of Subsets\n"
               " Flow and Matching  : 13 Max Flow   14 Bipartite Matching\n"
               " Strings and Numbers: 15 String Matching (4 algorithms)   16 Chinese Remainder Theorem\n"
               " 0 Back\n");
        int r = readIntRange("Choice: ", 0, 16, &c);
        if (r == -1) return;
        if (r == 0) continue;
        printf("\n");
        switch (c) {
            case 1: demoMaxSubarray(); break;   case 2: demoPower(); break;      case 3: demoCelebrity(); break;
            case 4: demoStrassen(); break;      case 5: demoHuffman(); break;    case 6: demoMST(); break;
            case 7: demoLCS(); break;           case 8: demoMatrixChain(); break; case 9: demoOBST(); break;
            case 10: demoTSP(); break;          case 11: demoNQueens(); break;   case 12: demoSubsetSum(); break;
            case 13: demoMaxFlow(); break;      case 14: demoMatching(); break;  case 15: demoStrings(); break;
            case 16: demoCRT(); break;
            default: return;
        }
    }
}

static void analysisMenu(void) {
    for (;;) {
        long c;
        printf("\n--- Complexity and analysis ---\n 1 Sort and search comparison   2 Recurrences (iteration, tree, substitution)\n"
               " 3 Master Theorem solver        4 Theory notes (asymptotics, P/NP, paradigms)   0 Back\n");
        int r = readIntRange("Choice: ", 0, 4, &c);
        if (r == -1) return;
        if (r == 0) continue;
        printf("\n");
        if (c == 1) demoComplexity(); else if (c == 2) demoRecurrences();
        else if (c == 3) demoMasterTheorem(); else if (c == 4) showTheory(); else return;
    }
}

static void printMenu(void) {
    printf("\n===== E-COMMERCE ORDER FULFILMENT AND DELIVERY OPTIMIZER =====\n"
           " ORDERS AND INVENTORY      1 Place order         2 View inventory      3 Record damaged stock\n"
           "                           4 View order log      5 Redistribution advice\n"
           " ROUTING                   6 Compare routing algorithms (Dijkstra / Bellman-Ford / Floyd-Warshall)\n"
           "                           7 Add or change a road (validated)   8 Negative-weight demo\n"
           " SCHEDULING AND LOADING    9 Delivery scheduling   10 Vehicle load planning (Knapsack)\n"
           " DATA                     11 Load dataset from file   12 Reset to built-in dataset\n"
           " ALGORITHMS AND ANALYSIS  13 Supplementary algorithm demos   14 Complexity and analysis\n"
           " TESTING                  15 Run automated tests    16 Demonstration scenarios\n"
           "  0 Exit\n");
}

int main(int argc, char **argv) {
    if (loadBuiltinData() != 0) return 1;
    if (argc > 1 && strcmp(argv[1], "--test") == 0) return runTests() == 0 ? 0 : 1;
    if (argc > 1 && strcmp(argv[1], "--scenarios") == 0) { runScenarios(); return 0; }
    for (;;) {
        long c;
        printMenu();
        int r = readIntRange("Choice: ", 0, 16, &c);
        if (r == -1) break;
        if (r == 0) continue;
        printf("\n");
        switch (c) {
            case 1: placeOrderMenu(); break;     case 2: printInventory(); break;     case 3: damageMenu(); break;
            case 4: printOrders(); break;        case 5: recommendTransfers(1); break; case 6: routeMenu(); break;
            case 7: roadMenu(); break;           case 8: demoNegativeWeights(); break; case 9: demoScheduling(); break;
            case 10: demoKnapsack(); break;      case 11: loadMenu(); break;
            case 12: if (loadBuiltinData() == 0) printf("Built-in dataset restored. Order log cleared.\n"); break;
            case 13: extrasMenu(); break;        case 14: analysisMenu(); break;
            case 15: runTests(); break;          case 16: runScenarios(); break;
            default: printf("Goodbye.\n"); return 0;
        }
    }
    return 0;
}
