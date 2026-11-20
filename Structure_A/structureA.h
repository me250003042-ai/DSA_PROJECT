
#pragma once

#include "./headerfiles/item.h"
#include "./headerfiles/maxheap.h"
#include "./headerfiles/minheap.h"
#include "./headerfiles/hashtable.h"
#include "./headerfiles/expiryqueue.h"

#include <stdexcept>
#include <vector>
#include <cmath>

class RunningMedian {
private:
    MaxHeap low;
    MinHeap high;
    HashTable table;
    ExpiryQueue windowQueue;

    int lowLive;
    int highLive;
    int deadItems;

    long long nextId;
    double purgeFactor;

    int targetLow(double p) const {
        int total = lowLive + highLive;

        if (total == 0)
            return 0;

        return (int)std::ceil(p * total);
    }

    bool markDead(long long id) {
        HashEntry* entry = table.find(id);

        if (entry == nullptr || !entry->active)
            return false;

        int side = entry->heapSide;

        if (!table.erase(id))
            return false;

        if (side == 0)
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

            if (entry != nullptr)
                table.remove(id);

            if (deadItems > 0)
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

            if (entry != nullptr)
                table.remove(id);

            if (deadItems > 0)
                deadItems--;
        }
    }

    void rebalance(double p) {
        cleanLow();
        cleanHigh();

        int target = targetLow(p);

        while (lowLive > target) {
            cleanLow();

            if (low.empty())
                break;

            Item x = low.pop();

            high.push(x);

            HashEntry* entry = table.find(x.id);

            if (entry != nullptr)
                entry->heapSide = 1;

            lowLive--;
            highLive++;
        }

        while (lowLive < target) {
            cleanHigh();

            if (high.empty())
                break;

            Item x = high.pop();

            low.push(x);

            HashEntry* entry = table.find(x.id);

            if (entry != nullptr)
                entry->heapSide = 0;

            highLive--;
            lowLive++;
        }

        cleanLow();
        cleanHigh();
    }

    void purgeIfNeeded() {
        int live = lowLive + highLive;
        int total = low.size() + high.size();

        if (total <= purgeFactor * live)
            return;

        std::vector<Item> keepLow;
        std::vector<Item> keepHigh;

        for (const Item& x : low.items()) {
            HashEntry* entry = table.find(x.id);

            if (entry != nullptr && entry->active) {
                keepLow.push_back(x);
            } else if (entry != nullptr) {
                table.remove(x.id);
            }
        }

        for (const Item& x : high.items()) {
            HashEntry* entry = table.find(x.id);

            if (entry != nullptr && entry->active) {
                keepHigh.push_back(x);
            } else if (entry != nullptr) {
                table.remove(x.id);
            }
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
    RunningMedian(int w) : windowQueue(w) {
        if (w <= 0)
            throw std::invalid_argument("Invalid window size");

        lowLive = 0;
        highLive = 0;
        deadItems = 0;

        nextId = 1;
        purgeFactor = 2.0;
    }

    long long add(double value) {
        if (windowQueue.full()) {
            long long oldId = windowQueue.pop();
            markDead(oldId);
        }

        long long id = nextId++;

        Item x(value, id);

        cleanLow();
        cleanHigh();

        if (low.empty() || value <= low.top().value) {
            if (!table.insert(id, 0))
                throw std::runtime_error("Hash table insertion failed");

            low.push(x);
            lowLive++;
        } else {
            if (!table.insert(id, 1))
                throw std::runtime_error("Hash table insertion failed");

            high.push(x);
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

    double get() {
        if (size() == 0)
            throw std::runtime_error("No readings available");

        rebalance(0.5);

        if (size() % 2 == 0) {
            return low.top().value / 2.0
                 + high.top().value / 2.0;
        }

        return low.top().value;
    }

    double findPercentile(double p) {
        if (size() == 0)
            throw std::runtime_error("No readings available");

        if (!(p > 0.0 && p <= 1.0))
            throw std::invalid_argument("Invalid percentile");

        rebalance(p);

        return low.top().value;
    }

    void setPurgeFactor(double factor) {
        if (!(factor > 0.0))
            throw std::invalid_argument("Invalid purge factor");

        purgeFactor = factor;

        purgeIfNeeded();
    }

    int size() const {
        return lowLive + highLive;
    }

    int heapItems() const {
        return low.size() + high.size();
    }
};
