// adversary/generate_adversary.cpp  (owner: Adversary / D5 - Stavan Ingaldar, 250041037)
// Runs the adversarial workloads directly against StructureA and StructureB and writes
// results/adversary.csv (or prints one CSV line when called with CLI arguments).

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include "adversary.hpp"
#include "../src/structA/medianA.h"
#include "../src/structB/medianB.h"
using namespace std;

long readStatusKb(string key) {
    ifstream f("/proc/self/status");
    string line;
    while (getline(f, line)) {
        if (line.compare(0, key.size(), key) == 0) {
            return atol(line.c_str() + key.size());
        }
    }
    return 0;
}

void runOneAdversary(const string& advName, const string& which, int w, int retractPct,
                     int n, int percentile, unsigned seed, double purgeFactor, ostream& out) {
    adversary::StreamInput in;
    if (advName == "adv1_expiry_funnel") {
        in = adversary::make_expiry_funnel(n, w);
    } else if (advName == "adv2_buried_retract") {
        in = adversary::make_buried_retractions(n, w, retractPct);
    } else if (advName == "adv3_pinned_memory") {
        in = adversary::make_pinned_memory(n, w);
    } else {
        in = adversary::make_fenwick_blowup(n, w, retractPct, seed);
    }

    long baseKb = readStatusKb("VmRSS:");

#ifdef COUNT_OPS
    g_heap_compares = 0; g_heap_swaps = 0; g_hash_probes = 0;
    g_purge_moves = 0; g_fen_steps = 0; g_bin_steps = 0;
#endif

    auto t0 = chrono::steady_clock::now();
    StructureA *a = 0;
    StructureB *b = 0;
    if (which == "A") {
        a = new StructureA(w, percentile);
        a->setPurgeFactor(purgeFactor);
    } else {
        b = new StructureB(w, percentile, in.values);
    }
    auto t1 = chrono::steady_clock::now();

    double checksum = 0;
    long retractDone = 0;
    for (int i = 0; i < n; i++) {
        if (which == "A") {
            a->add(in.values[i]);
            if (in.retractId[i] > 0) {
                if (a->retract(in.retractId[i])) retractDone++;
            }
            checksum += a->get();
        } else {
            b->add(in.values[i]);
            if (in.retractId[i] > 0) {
                if (b->retract(in.retractId[i])) retractDone++;
            }
            checksum += b->get();
        }
    }
    auto t2 = chrono::steady_clock::now();

    long peakKb = readStatusKb("VmHWM:");
    double buildMs = chrono::duration<double, milli>(t1 - t0).count();
    double runMs = chrono::duration<double, milli>(t2 - t1).count();

    out.precision(12);
    out << advName << "," << which << "," << w << "," << retractPct << "," << n << ","
        << percentile << "," << seed << "," << purgeFactor << ","
        << buildMs << "," << runMs << "," << (runMs * 1e6 / n) << ","
        << (peakKb - baseKb) << "," << checksum << "," << retractDone;
#ifdef COUNT_OPS
    out << "," << g_heap_compares / (double)n << "," << g_heap_swaps / (double)n
        << "," << g_hash_probes / (double)n << "," << g_purge_moves / (double)n
        << "," << g_fen_steps / (double)n << "," << g_bin_steps / (double)n;
#endif
    out << "\n";

    delete a;
    delete b;
}

int main(int argc, char **argv) {
    int n = (argc > 1) ? atoi(argv[1]) : 1000000;
    ofstream f("results/adversary.csv");
    ostream& out = f.is_open() ? f : cout;

    out << "adversary,structure,w,retract_pct,N,percentile,seed,purge_factor,"
        << "build_ms,run_ms,ns_per_step,peak_kb,checksum,retractions_done";
#ifdef COUNT_OPS
    out << ",heap_compares,heap_swaps,hash_probes,purge_moves,fen_steps,bin_steps";
#endif
    out << "\n";

    // 1. Funnel without purge (purgeFactor = 1e9 disables purge -> shows Theta(N) dead entries)
    //    vs Funnel with purge (purgeFactor = 2.0 -> bounds dead entries by live entries)
    runOneAdversary("adv1_expiry_funnel", "A", 10000, 0, n, 50, 1, 1000000.0, out);
    runOneAdversary("adv1_expiry_funnel", "A", 10000, 0, n, 50, 1, 2.0, out);

    // 2. Buried retractions without purge vs with purge (purgeFactor = 2.0)
    runOneAdversary("adv2_buried_retract", "A", 10000, 90, n, 50, 1, 1000000.0, out);
    runOneAdversary("adv2_buried_retract", "A", 10000, 90, n, 50, 1, 2.0, out);

    // 3. Pinned maximum dead entries (hovers at D <= L without triggering purge)
    runOneAdversary("adv3_pinned_memory", "A", 10000, 50, n, 50, 1, 2.0, out);

    // 4. Fenwick compressed domain blowup (U = N = 10^6 vs tiny live count w = 100, r = 90%)
    //    Compare Structure A (fast, tiny memory) vs Structure B (large U = 10^6 domain)
    runOneAdversary("adv4_fenwick_blowup", "A", 100, 90, n, 50, 1, 2.0, out);
    runOneAdversary("adv4_fenwick_blowup", "B", 100, 90, n, 50, 1, 2.0, out);

    cout << "wrote results/adversary.csv\n";
    return 0;
}
