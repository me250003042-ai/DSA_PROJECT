// bench.cpp  (owner: Analyst)
// Runs ONE experiment setting on ONE structure and prints one CSV line.
//
// Usage: ./bench STRUCT W RETRACT_PCT RANGE N PERCENTILE SEED PURGE_FACTOR
//   STRUCT = A or B,  W = window,  RETRACT_PCT = chance (0..100) of one retraction per step
//   RANGE  = readings are random in [0, RANGE),  N = number of readings
//
// One step = add a reading, maybe retract an earlier one, then ask for the percentile.
// Compile with -DCOUNT_OPS to also print the operation counters (slower, so not used for timing).

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include "../src/structA/medianA.h"
#include "../src/structB/medianB.h"
using namespace std;

// read a number such as "VmRSS:  1234 kB" from /proc/self/status (Linux)
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

int main(int argc, char **argv) {
    if (argc < 9) {
        cout << "usage: ./bench STRUCT W RETRACT_PCT RANGE N PERCENTILE SEED PURGE_FACTOR\n";
        return 1;
    }
    string which = argv[1];
    int w = atoi(argv[2]);
    int retractPct = atoi(argv[3]);
    int range = atoi(argv[4]);
    int n = atoi(argv[5]);
    int percentile = atoi(argv[6]);
    unsigned seed = (unsigned)atoi(argv[7]);
    double purgeFactor = atof(argv[8]);

    // ---- make the input (not timed) ----
    mt19937 rng(seed);
    vector<int> values(n);
    vector<int> retractId(n);          // 0 = no retraction in this step
    for (int i = 0; i < n; i++) {
        values[i] = (int)(rng() % range);
        retractId[i] = 0;
        if ((int)(rng() % 100) < retractPct) {
            int back = (int)(rng() % w);           // one of the last w readings
            int id = (i + 1) - back;               // reading i has id i+1
            if (id < 1) {
                id = 1;
            }
            retractId[i] = id;
        }
    }

    long baseKb = readStatusKb("VmRSS:");

    // ---- build ----
    auto t0 = chrono::steady_clock::now();
    StructureA *a = 0;
    StructureB *b = 0;
    if (which == "A") {
        a = new StructureA(w, percentile);
        a->setPurgeFactor(purgeFactor);
    } else {
        b = new StructureB(w, percentile, values);
    }
    auto t1 = chrono::steady_clock::now();

    // ---- replay (timed) ----
    double checksum = 0;
    long retractDone = 0;
    for (int i = 0; i < n; i++) {
        if (which == "A") {
            a->add(values[i]);
            if (retractId[i] > 0) {
                if (a->retract(retractId[i])) {
                    retractDone++;
                }
            }
            checksum += a->get();
        } else {
            b->add(values[i]);
            if (retractId[i] > 0) {
                if (b->retract(retractId[i])) {
                    retractDone++;
                }
            }
            checksum += b->get();
        }
    }
    auto t2 = chrono::steady_clock::now();

    long peakKb = readStatusKb("VmHWM:");
    double buildMs = chrono::duration<double, milli>(t1 - t0).count();
    double runMs = chrono::duration<double, milli>(t2 - t1).count();

    cout.precision(12);
    cout << which << "," << w << "," << retractPct << "," << range << "," << n << ","
         << percentile << "," << seed << "," << purgeFactor << ","
         << buildMs << "," << runMs << "," << (runMs * 1e6 / n) << ","
         << (peakKb - baseKb) << "," << checksum << "," << retractDone;
#ifdef COUNT_OPS
    cout << "," << g_heap_compares / (double)n << "," << g_heap_swaps / (double)n
         << "," << g_hash_probes / (double)n << "," << g_purge_moves / (double)n
         << "," << g_fen_steps / (double)n << "," << g_bin_steps / (double)n;
#endif
    cout << "\n";
    return 0;
}
