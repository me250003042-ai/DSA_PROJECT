
#include <iostream>
#include "./oracle.h"

int main() {

    Oracle oracle;

    // Insert readings
    oracle.insert(1, 10);
    oracle.insert(2, 20);
    oracle.insert(3, 30);
    oracle.insert(4, 40);
    oracle.insert(5, 50);

    std::cout << "Total readings: "
              << oracle.size() << '\n';

    // Median
    auto median = oracle.getMedian();

    if (median.has_value()) {
        std::cout << "Median: "
                  << median.value() << '\n';
    }

    // P90
    auto p90 = oracle.getPercentile(90);

    if (p90.has_value()) {
        std::cout << "P90: "
                  << p90.value() << '\n';
    }

    // Display sorted readings
    std::cout << "\nSorted readings:\n";

    for (const auto& entry : oracle.getSortedReadings()) {
        std::cout << "ID: " << entry.second
                  << " Value: " << entry.first << '\n';
    }

    // Retract ID 3
    std::cout << "\nRetracting ID 3...\n";

    if (oracle.erase(3)) {
        std::cout << "Retraction successful\n";
    } else {
        std::cout << "Retraction failed\n";
    }

    // Updated median
    median = oracle.getMedian();

    if (median.has_value()) {
        std::cout << "Updated median: "
                  << median.value() << '\n';
    }

    // Updated P90
    p90 = oracle.getPercentile(90);

    if (p90.has_value()) {
        std::cout << "Updated P90: "
                  << p90.value() << '\n';
    }

    // Duplicate retraction test
    std::cout << "\nRetracting ID 3 again...\n";

    if (!oracle.erase(3)) {
        std::cout << "Duplicate retraction rejected\n";
    }

    return 0;
}
