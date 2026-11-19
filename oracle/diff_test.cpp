// diff_test.cpp  (differential test)
// Runs the same random operations on Structure A, Structure B and the oracle
// and checks that all three give the same answer after every operation.
//
// Usage:  ./diff_test            (default: 300 small runs + 1 big run of 1,000,000 ops)
//         ./diff_test 50 100000  (50 small runs, big run with 100000 ops)
//
// Every run uses a seed, and a failure prints the seed and the step,
// so the failing case can be repeated exactly.

#include <iostream>
#include <vector>
#include <cstdlib>
#include <random>
#include <cmath>
#include "../src/structA/medianA.h"
#include "../src/structB/medianB.h"
#include "oracle.h"
using namespace std;

bool sameNumber(double x, double y) {
    if (std::isnan(x) && std::isnan(y)) {
        return true;
    }
    return x == y;
}

// Run one random test. Returns true if everything agreed.
bool runOne(unsigned seed, int steps, int window, int valueRange, bool deepCheck) {
    mt19937 rng(seed);

    // decide the whole stream first, because B needs all values for compression
    // type 0 = add, type 1 = retract
    vector<int> type(steps);
    vector<double> number(steps);    // the value to add, or the id to retract
    vector<double> allValues;
    int addedSoFar = 0;
    for (int i = 0; i < steps; i++) {
        int dice = (int)(rng() % 100);
        if (dice < 70 || addedSoFar == 0) {
            type[i] = 0;
            number[i] = (double)((int)(rng() % valueRange) - valueRange / 2) + 0.5;
            allValues.push_back(number[i]);
            addedSoFar++;
        } else {
            type[i] = 1;
            number[i] = (int)(rng() % (addedSoFar + 3));   // sometimes a wrong id (0 or too big)
        }
    }

    StructureA a(window);
    StructureB b(window, allValues);
    Oracle o(window);

    for (int i = 0; i < steps; i++) {
        if (type[i] == 0) {
            int ida = a.add(number[i]);
            int idb = b.add(number[i]);
            int ido = o.add(number[i]);
            if (ida != ido || idb != ido) {
                cout << "FAIL seed=" << seed << " step=" << i << " ids differ\n";
                return false;
            }
        } else {
            int id = (int)number[i];
            bool ra = a.retract(id);
            bool rb = b.retract(id);
            bool ro = o.retract(id);
            if (ra != ro || rb != ro) {
                cout << "FAIL seed=" << seed << " step=" << i << " retract result differs\n";
                return false;
            }
        }

        double ga = a.get();
        double gb = b.get();
        double go = o.get();
        if (!sameNumber(ga, go) || !sameNumber(gb, go)) {
            cout << "FAIL seed=" << seed << " step=" << i << " window=" << window
                 << " range=" << valueRange
                 << "  A=" << ga << " B=" << gb << " oracle=" << go << "\n";
            return false;
        }
        if (deepCheck && !b.selfCheck()) {
            cout << "FAIL seed=" << seed << " step=" << i << " Fenwick tree is not consistent\n";
            return false;
        }
    }
    return true;
}

// the small example from the demo: expected answers 3.5, 4.5, 2, 3
bool runExample() {
    vector<double> all;
    all.push_back(5);
    all.push_back(2);
    all.push_back(8);
    all.push_back(1);
    all.push_back(7);
    all.push_back(3);

    StructureA a(4);
    StructureB b(4, all);
    double expected[4] = {3.5, 4.5, 2, 3};
    double gotA[4];
    double gotB[4];

    a.add(5); a.add(2); int ida = a.add(8); a.add(1);
    b.add(5); b.add(2); int idb = b.add(8); b.add(1);
    gotA[0] = a.get();
    gotB[0] = b.get();
    a.add(7);
    b.add(7);
    gotA[1] = a.get();
    gotB[1] = b.get();
    a.retract(ida);
    b.retract(idb);
    gotA[2] = a.get();
    gotB[2] = b.get();
    a.add(3);
    b.add(3);
    gotA[3] = a.get();
    gotB[3] = b.get();

    for (int i = 0; i < 4; i++) {
        if (gotA[i] != expected[i] || gotB[i] != expected[i]) {
            cout << "FAIL example, answer number " << i << "\n";
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    int smallRuns = 300;
    int bigSteps = 1000000;
    if (argc > 1) {
        smallRuns = atoi(argv[1]);
    }
    if (argc > 2) {
        bigSteps = atoi(argv[2]);
    }

    bool allOk = true;

    if (runExample()) {
        cout << "example from demo ........ ok\n";
    } else {
        allOk = false;
    }

    int failed = 0;
    for (int run = 0; run < smallRuns; run++) {
        unsigned seed = 1000 + run;
        int window = 1 + (int)(seed % 40);
        int valueRange = 3 + (int)((seed * 7) % 60);      // small range -> many equal values
        if (!runOne(seed, 400, window, valueRange, true)) {
            failed++;
            allOk = false;
        }
    }
    cout << "small random runs ........ " << (smallRuns - failed) << " of " << smallRuns << " ok\n";

    if (bigSteps > 0) {
        bool ok = runOne(424242, bigSteps, 1000, 100000, false);
        cout << "big run (" << bigSteps << " ops, median) ... " << (ok ? "ok" : "FAILED") << "\n";
        if (!ok) {
            allOk = false;
        }
        ok = runOne(777, bigSteps / 10, 200, 1000, false);
        cout << "second big median run ... " << (ok ? "ok" : "FAILED") << "\n";
        if (!ok) {
            allOk = false;
        }
    }

    if (allOk) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    }
    cout << "SOME TESTS FAILED\n";
    return 1;
}
