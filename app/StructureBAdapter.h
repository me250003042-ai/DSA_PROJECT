#ifndef STRUCTURE_B_ADAPTER_H
#define STRUCTURE_B_ADAPTER_H

#include "IOrderStatistic.h"
#include "../src/structB/medianB.h"

#include <deque>
#include <unordered_set>
#include <vector>

class StructureBAdapter : public IOrderStatistic {
private:
    StructureB structure;
    int windowSize;
    std::deque<int> arrivals;
    std::unordered_set<int> activeIDs;

public:
    StructureBAdapter(int w, std::vector<double> allValues)
        : structure(w, allValues), windowSize(w) {}

    int add(double value) override {
        int id = structure.add(value);

        arrivals.push_back(id);
        activeIDs.insert(id);

        if ((int)arrivals.size() > windowSize) {
            int expiredID = arrivals.front();
            arrivals.pop_front();
            activeIDs.erase(expiredID);
        }

        return id;
    }

    bool retract(int id) override {
        if (activeIDs.find(id) == activeIDs.end())
            return false;

        if (!structure.retract(id))
            return false;

        activeIDs.erase(id);
        return true;
    }

    double getMedian() override {
        return structure.get();
    }

    int getSize() const override {
        return (int)activeIDs.size();
    }
};

#endif
