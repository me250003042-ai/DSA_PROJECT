// monitor.cpp  (application)
// A monitoring replay. A sensor sends readings; some are faulty spikes that
// are retracted one step later. After every step the program reports
//   * the windowed MEDIAN (Structure A, two heaps)
//   * the same windowed MEDIAN (Structure B, Fenwick tree)
// and raises an alert when the median goes above a threshold.
//
// Usage: ./monitor [N] [WINDOW] [THRESHOLD] [SEED]
//   default: N=1000000 readings, WINDOW=1000, THRESHOLD=110, SEED=1

#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include "../src/structA/medianA.h"
#include "../src/structB/medianB.h"
using namespace std;

int main(int argc, char **argv) {
    int n = 1000000;
    int window = 1000;
    int threshold = 110;
    unsigned seed = 1;
    if (argc > 1) { n = atoi(argv[1]); }
    if (argc > 2) { window = atoi(argv[2]); }
    if (argc > 3) { threshold = atoi(argv[3]); }
    if (argc > 4) { seed = (unsigned)atoi(argv[4]); }

    // ---- make the sensor data ----
    // normal reading: about 100 +- 20, with a slow drift upward in the middle
    // faulty reading: 5000 (about 1 in 50)
    mt19937 rng(seed);
    vector<double> reading(n);
    vector<char> faulty(n);
    for (int i = 0; i < n; i++) {
        int drift = 0;
        if (i > n / 2 && i < n / 2 + n / 10) {
            drift = 30;                       // a period where the sensor reads high
        }
        reading[i] = 100.5 + drift + (int)(rng() % 41) - 20;
        faulty[i] = 0;
        if (rng() % 50 == 0) {
            reading[i] = 5000;
            faulty[i] = 1;
        }
    }

    StructureA medianKeeper(window);
    StructureB fenwickMedianKeeper(window, reading);

    long alerts = 0;
    bool inAlert = false;
    long retracted = 0;
    int lastFaultyId = 0;
    double lastMedian = 0;
    double lastFenwickMedian = 0;

    for (int i = 0; i < n; i++) {
        int idA = medianKeeper.add(reading[i]);
        int idB = fenwickMedianKeeper.add(reading[i]);

        // a faulty reading is noticed one step later and retracted
        if (lastFaultyId > 0) {
            medianKeeper.retract(lastFaultyId);
            fenwickMedianKeeper.retract(lastFaultyId);
            retracted++;
            lastFaultyId = 0;
        }
        if (faulty[i] == 1) {
            lastFaultyId = idA;       // idA and idB are the same number
        }
        if (idA != idB) {
            cout << "ids differ, something is wrong\n";
            return 1;
        }

        lastMedian = medianKeeper.get();
        lastFenwickMedian = fenwickMedianKeeper.get();

        // alert rule: raise once when the median crosses the threshold
        if (lastMedian > threshold && !inAlert) {
            inAlert = true;
            alerts++;
            cout << "ALERT at step " << i << ": median " << lastMedian << " is above " << threshold << "\n";
        }
        if (lastMedian <= threshold && inAlert) {
            inAlert = false;
            cout << "alert over at step " << i << ": median " << lastMedian << "\n";
        }

        if (i < 10 || (i + 1) % (n / 5) == 0) {
            cout << "step " << i << ": heap median = " << lastMedian
                 << ", Fenwick median = " << lastFenwickMedian << "\n";
        }
    }

    cout << "readings: " << n << ", window: " << window << ", faulty readings retracted: " << retracted << "\n";
    cout << "alerts raised: " << alerts << "\n";
    return 0;
}
