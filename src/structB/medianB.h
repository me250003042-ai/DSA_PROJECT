#pragma once
#include "teamB.h"
class StructureB : public TeamStructureB {
public:
    StructureB(int w, std::vector<double> values) : TeamStructureB(w, values) {}
    int add(double value) { return static_cast<int>(TeamStructureB::add(value)) + 1; }
    bool retract(int id) { return id > 0 && TeamStructureB::retract(id - 1); }
};
