#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "IOrderStatistic.h"
#include "StructureAAdapter.h"
#include "StructureBAdapter.h"

using namespace std;

int main(int argc, char **argv) {
    int n = 1000000;
    int window = 1000;
    int threshold = 110;
    unsigned seed = 1;

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) window = atoi(argv[2]);
    if (argc > 3) threshold = atoi(argv[3]);
    if (argc > 4) seed = (unsigned)atoi(argv[4]);

    if (n <= 0 || window <= 0) {
        cout << "Invalid number of readings or window size\n";
        return 1;
    }

    // Generate sensor readings.
    mt19937 rng(seed);
    vector<double> reading(n);
    vector<char> faulty(n);

    for (int i = 0; i < n; i++) {
        int drift = 0;

        if (i > n / 2 && i < n / 2 + n / 10)
            drift = 30;

        reading[i] = 100.5 + drift + (int)(rng() % 41) - 20;

        faulty[i] = 0;

        if (rng() % 50 == 0) {
            reading[i] = 5000;
            faulty[i] = 1;
        }
    }

    // Create both implementations through their adapters.
    StructureAAdapter adapterA(window);
    StructureBAdapter adapterB(window, reading);

    // Common interface references.
    IOrderStatistic &A = adapterA;
    IOrderStatistic &B = adapterB;

    long alerts = 0;
    long retracted = 0;
    bool inAlert = false;
    int lastFaultyId = 0;

    for (int i = 0; i < n; i++) {

        // Insert the same reading into both structures.
        int idA = A.add(reading[i]);
        int idB = B.add(reading[i]);

        if (idA != idB) {
            cout << "ERROR: Reading IDs differ\n";
            return 1;
        }

        // Retract the previous faulty reading.
        if (lastFaultyId > 0) {
            bool resultA = A.retract(lastFaultyId);
            bool resultB = B.retract(lastFaultyId);

            if (resultA != resultB) {
                cout << "ERROR: Retraction results differ\n";
                return 1;
            }

            if (resultA)
                retracted++;

            lastFaultyId = 0;
        }

        if (faulty[i])
            lastFaultyId = idA;

        // Compare active reading counts.
        if (A.getSize() != B.getSize()) {
            cout << "ERROR: Active sizes differ at step " << i << "\n";
            return 1;
        }

        // Compare medians.
        double medianA = A.getMedian();
        double medianB = B.getMedian();

        if (isnan(medianA) != isnan(medianB) ||
            (!isnan(medianA) && fabs(medianA - medianB) > 1e-9)) {
            cout << "ERROR: Medians differ at step " << i << "\n";
            cout << "Structure A: " << medianA << "\n";
            cout << "Structure B: " << medianB << "\n";
            return 1;
        }

        // Raise an alert when the median crosses the threshold.
        if (medianA > threshold && !inAlert) {
            inAlert = true;
            alerts++;

            cout << "ALERT at step " << i
                 << ": median " << medianA
                 << " is above " << threshold << "\n";
        }

        if (medianA <= threshold && inAlert) {
            inAlert = false;

            cout << "Alert over at step " << i
                 << ": median " << medianA << "\n";
        }

        int interval = max(1, n / 5);

        if (i < 10 || (i + 1) % interval == 0) {
            cout << "Step " << i
                 << ": Heap median = " << medianA
                 << ", Fenwick median = " << medianB << "\n";
        }
    }

    cout << "\nMonitoring completed successfully.\n";
    cout << "Readings: " << n << "\n";
    cout << "Window: " << window << "\n";
    cout << "Faulty readings retracted: " << retracted << "\n";
    cout << "Alerts raised: " << alerts << "\n";

    return 0;
}
