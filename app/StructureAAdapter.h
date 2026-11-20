#ifndef STRUCTURE_A_ADAPTER_H
#define STRUCTURE_A_ADAPTER_H

#include "IOrderStatistic.h"
#include "../src/structA/medianA.h"

class StructureAAdapter : public IOrderStatistic {
private:
    StructureA structure;

public:
    explicit StructureAAdapter(int windowSize)
        : structure(windowSize) {}

    int add(double value) override {
        return structure.add(value);
    }

    bool retract(int id) override {
        return structure.retract(id);
    }

    double getMedian() override {
        return structure.get();
    }

    int getSize() const override {
        return structure.size();
    }
};

#endif
