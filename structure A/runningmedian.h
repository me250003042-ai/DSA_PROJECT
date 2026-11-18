
#pragma once

#include "Item.h"
#include "MaxHeap.h"
#include "MinHeap.h"
#include "HashTable.h"
#include "ExpiryQueue.h"

#include <stdexcept>
#include <vector>
#include <cmath>
#include <stdexcept>

class RunningMedian {
private:
    MaxHeap low;
    MinHeap high;
    HashTable table;
    ExpiryQueue windowQueue;

    int windowSize;
    int lowLive;
    int highLive;
    int deadItems;

    long long nextId;
    double percentile;

    int targetLow(double p) const {
    int total = lowLive + highLive;

    if (total == 0)
        return 0;

    return (int)ceil(p * total);
}

    bool markDead(long long id) {
        HashEntry* entry = table.find(id);

        if (entry == nullptr || !entry->active)
            return false;

        entry->active = false;

        if (entry->heapSide == 0)
            lowLive--;
        else
            highLive--;

        deadItems++;
        return true;
    }

    void cleanLow() {
        while (!low.empty()) {
            long long id = low.top().id;
            HashEntry* entry = table.find(id);

            if (entry != nullptr && entry->active)
                break;

            low.pop();
            deadItems--;
        }
    }

    void cleanHigh() {
        while (!high.empty()) {
            long long id = high.top().id;
            HashEntry* entry = table.find(id);

            if (entry != nullptr && entry->active)
                break;

            high.pop();
            deadItems--;
        }
    }

   void rebalance(double p) {
    cleanLow();
    cleanHigh();

    int target = targetLow(p);

    while (lowLive > target) {
        Item x = low.pop();

        high.push(x);
        table.find(x.id)->heapSide = 1;

        lowLive--;
        highLive++;

        cleanLow();
    }

    while (lowLive < target) {
        cleanHigh();

        Item x = high.pop();

        low.push(x);
        table.find(x.id)->heapSide = 0;

        highLive--;
        lowLive++;

        cleanHigh();
    }

    cleanLow();
    cleanHigh();
}

    void purgeIfNeeded() {
        int live = lowLive + highLive;

        if (deadItems <= live)
            return;

        std::vector<Item> keepLow;
        std::vector<Item> keepHigh;

        for (const Item& x : low.items()) {
            HashEntry* entry = table.find(x.id);

            if (entry != nullptr && entry->active)
                keepLow.push_back(x);
        }

        for (const Item& x : high.items()) {
            HashEntry* entry = table.find(x.id);

            if (entry != nullptr && entry->active)
                keepHigh.push_back(x);
        }

        low.build(keepLow);
        high.build(keepHigh);

        deadItems = 0;
    }

    void finish() {
    cleanLow();
    cleanHigh();

    rebalance(0.5);
    purgeIfNeeded();

    cleanLow();
    cleanHigh();
}

public:
    RunningMedian(int w, double p = 0.5)
        : windowQueue(w) {

        if (w <= 0 || p <= 0.0 || p > 1.0)
            throw std::invalid_argument("Invalid window or percentile");

        windowSize = w;
        percentile = p;

        lowLive = 0;
        highLive = 0;
        deadItems = 0;

        nextId = 0;
    }

    long long insert(double value) {
        if (windowQueue.full()) {
            long long oldId = windowQueue.pop();
            markDead(oldId);
        }

        long long id = nextId++;

        Item x(value, id);

        cleanLow();
        cleanHigh();

        if (low.empty() || value <= low.top().value) {
            low.push(x);
            table.insert(id, 0);
            lowLive++;
        }
        else {
            high.push(x);
            table.insert(id, 1);
            highLive++;
        }

        windowQueue.push(id);

        finish();

        return id;
    }

    bool retract(long long id) {
        if (!markDead(id))
            return false;

        finish();

        return true;
    }

    double findMedian() {
    if (size() == 0)
        throw std::runtime_error("No readings available");

    rebalance(0.5);

    return low.top().value;
}

double findPercentile(double p) {
    if (size() == 0)
        throw std::runtime_error("No readings available");

    if (p <= 0.0 || p > 1.0)
        throw std::invalid_argument("Invalid percentile");

    rebalance(p);

    return low.top().value;
}

    int size() const {
        return lowLive + highLive;
    }
};

