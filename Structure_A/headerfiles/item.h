
#pragma once

struct Item {
    double value;
    long long id;

    Item() {
        value = 0;
        id = -1;
    }

    Item(double v, long long i) {
        value = v;
        id = i;
    }
};

