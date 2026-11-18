#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <random>
#include <cstdlib>
#include <cmath>
#include "../Structure_A/structureA.h"
#include "../Structure_B/structureB.h"
using namespace std;

double memoryKb(bool peak) {
    PROCESS_MEMORY_COUNTERS info = {};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) {
        cerr << "Cannot read process memory: " << GetLastError() << "\n";
        exit(1);
    }
    return (peak ? info.PeakWorkingSetSize : info.WorkingSetSize) / 1024.0;
}

LONGLONG ticks() {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

int main(int argc, char **argv) {
    if (argc < 8) {
        cout << "usage: ./bench STRUCT W RETRACT_PCT RANGE N SEED PURGE_FACTOR\n";
        return 1;
    }

    string which = argv[1];
    int w = atoi(argv[2]);
    int retractPct = atoi(argv[3]);
    int range = atoi(argv[4]);
    int n = atoi(argv[5]);
    unsigned seed = (unsigned)atoi(argv[6]);
    double purgeFactor = atof(argv[7]);
    if ((which != "A" && which != "B") || w <= 0 || range <= 0 || n <= 0 ||
        retractPct < 0 || retractPct > 100 ||
        !isfinite(purgeFactor) || purgeFactor <= 1) {
        cerr << "Invalid setting: use A/B, positive sizes, retract 0..100, purge factor > 1.\n";
        return 1;
    }

    mt19937 rng(seed);
    vector<double> values(n);
    vector<int> retractId(n);
    for (int i = 0; i < n; i++) {
        values[i] = (double)(rng() % range) + 0.5;
        retractId[i] = 0;
        if ((int)(rng() % 100) < retractPct) {
            int back = (int)(rng() % w);
            int id = (i + 1) - back;
            if (id < 1) {
                id = 1;
            }
            retractId[i] = id;
        }
    }

    double baseKb = memoryKb(false);
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LONGLONG t0 = ticks();

    StructureA *a = 0;
    StructureB *b = 0;
    if (which == "A") {
        a = new StructureA(w);
        a->setPurgeFactor(purgeFactor);
    } else {
        b = new StructureB(w, values);
    }
    LONGLONG t1 = ticks();

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
            double answer = a->get();
            if (!isnan(answer)) checksum += answer;
        } else {
            b->add(values[i]);
            if (retractId[i] > 0) {
                if (b->retract(retractId[i])) {
                    retractDone++;
                }
            }
            double answer = b->get();
            if (!isnan(answer)) checksum += answer;
        }
    }
    LONGLONG t2 = ticks();

    double peakKb = memoryKb(true);

    double buildMs = (t1 - t0) * 1000.0 / frequency.QuadPart;
    double runMs = (t2 - t1) * 1000.0 / frequency.QuadPart;
    int distinctValues = which == "B" ? b->distinctValues() : 0;

    cout.precision(12);
    cout << which << "," << w << "," << retractPct << "," << range << "," << n << ","
         << seed << "," << purgeFactor << ","
         << buildMs << "," << runMs << "," << (runMs * 1e6 / n) << ","
         << (peakKb - baseKb) << "," << checksum << "," << retractDone << "," << distinctValues;
#ifdef COUNT_OPS
    cout << "," << g_heap_compares / (double)n << "," << g_heap_swaps / (double)n
         << "," << g_hash_probes / (double)n << "," << g_purge_moves / (double)n
         << "," << g_fen_steps / (double)n << "," << g_bin_steps / (double)n;
#endif
    cout << "\n";
    delete a;
    delete b;
    return 0;
}
