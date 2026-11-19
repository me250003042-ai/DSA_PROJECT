// Small demo of Structure B.  Build: g++ -std=c++17 src/structB/medianB.cpp -o demoB
#include <iostream>
#include "medianB.h"

int main() {
    // all values that will ever be added (needed for compression)
    vector<double> all;
    all.push_back(5);
    all.push_back(2);
    all.push_back(8);
    all.push_back(1);
    all.push_back(7);
    all.push_back(3);

    StructureB m(4, all);

    m.add(5);
    m.add(2);
    int id3 = m.add(8);
    m.add(1);
    cout << m.get() << endl;   // 3.5

    m.add(7);                  // 5 leaves the window
    cout << m.get() << endl;   // 4.5

    m.retract(id3);            // retract 8
    cout << m.get() << endl;   // 2

    m.add(3);                  // 2 leaves the window
    cout << m.get() << endl;   // 3
    return 0;
}
