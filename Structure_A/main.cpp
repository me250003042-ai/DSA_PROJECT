
#include <iostream>
#include "./structureA.h"
#include "Counters.h"

using namespace std;

int main() {
    int w;

    cout << "Enter window size: ";
    cin >> w;

    try {
        RunningMedian rm(w);

        while (true) {
            cout << "\n1. Insert reading\n";
            cout << "2. Retract reading\n";
            cout << "3. Find median\n";
            cout << "4. Find percentile\n";
            cout << "5. Display active count\n";
            cout << "6. Exit\n";

            int choice;
            cout << "\nEnter choice: ";
            cin >> choice;

            try {
                if (choice == 1) {
                    double value;

                    cout << "Enter reading: ";
                    cin >> value;

                    cout << "Generated ID: "
                         << rm.insert(value) << '\n';
                }
                else if (choice == 2) {
                    long long id;

                    cout << "Enter ID: ";
                    cin >> id;

                    cout << (rm.retract(id)
                        ? "Reading retracted\n"
                        : "Invalid ID\n");
                }
                else if (choice == 3) {
                    if (rm.size() == 0)
                        cout << "No readings available\n";
                    else
                        cout << "Median: "
                             << rm.findMedian() << '\n';
                }
                else if (choice == 4) {
                    double p;

                    cout << "Enter percentile (0.9 = 90th): ";
                    cin >> p;

                    cout << "Percentile: "
                         << rm.findPercentile(p) << '\n';
                }
                else if (choice == 5) {
                    cout << "Active readings: "
                         << rm.size() << '\n';
                }
                else if (choice == 6) {
                    break;
                }
#ifdef COUNT_OPS
                else if (choice == 7) {
                    cout << "\nBenchmark Counters\n";

                    cout << "Heap comparisons: "
                         << g_heap_compares << '\n';

                    cout << "Heap swaps: "
                         << g_heap_swaps << '\n';

                    cout << "Hash probes: "
                         << g_hash_probes << '\n';

                    cout << "Purge entries scanned: "
                         << g_purge_moves << '\n';
                }
#endif
                else {
                    cout << "Invalid choice\n";
                }
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << '\n';
            }
        }
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << '\n';
    }

    return 0;
}
