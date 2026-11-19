// oracle.h  (owner: Oracle owner)
// The oracle: a sorted array, kept sorted by binary-search insertion and deletion.
// It is slow (O(w) per update) but so simple that it is easy to trust.
// Same interface as Structure A and Structure B.

#ifndef ORACLE_H
#define ORACLE_H

#include <vector>
#include <cmath>
using namespace std;

class Oracle {
    int w;
    vector<double> sorted;
    vector<double> valueOfId;
    vector<char> active;
    vector<int> order;
    int head;

    // first position whose value is >= x
    int firstNotLess(double x) {
        int lo = 0;
        int hi = (int)sorted.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (sorted[mid] < x) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    bool removeId(int id) {
        if (id <= 0 || id >= (int)active.size()) {
            return false;
        }
        if (active[id] == 0) {
            return false;
        }
        active[id] = 0;
        int pos = firstNotLess(valueOfId[id]);
        sorted.erase(sorted.begin() + pos);
        return true;
    }

public:
    Oracle(int window) {
        w = window;
        head = 0;
        active.push_back(0);
        valueOfId.push_back(0);
    }

    int add(double value) {
        int id = (int)active.size();
        active.push_back(1);
        valueOfId.push_back(value);
        int pos = firstNotLess(value);
        sorted.insert(sorted.begin() + pos, value);
        order.push_back(id);
        if ((int)order.size() - head > w) {
            removeId(order[head]);
            head++;
        }
        return id;
    }

    bool retract(int id) {
        return removeId(id);
    }

    double get() {
        int n = (int)sorted.size();
        if (n == 0) {
            return NAN;
        }
        if (n % 2 == 0) {
            return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
        }
        return sorted[n / 2];
    }
};

#endif
