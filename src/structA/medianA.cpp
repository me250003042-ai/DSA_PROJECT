// Small demo of Structure A.  Build: g++ -std=c++17 src/structA/medianA.cpp -o demoA
#include <iostream>
using namespace std;
#include "medianA.h"

int main() {
    StructureA m(4);

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
