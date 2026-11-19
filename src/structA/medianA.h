#pragma once
#include "teamA.h"
#include <cmath>
class StructureA : public RunningMedian {
public:
    explicit StructureA(int w) : RunningMedian(w) {}
    int add(double value) { return static_cast<int>(RunningMedian::add(value)); }
    double get() { return size() == 0 ? NAN : RunningMedian::get(); }
};
