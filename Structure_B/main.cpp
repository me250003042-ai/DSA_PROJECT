#include <iostream>
#include "./structureB.h"
using namespace std;

int main(){
    vector<double> a = {-2.25,-9.5,0.5,100.75,400};
    StructureB x = StructureB(4,a);
    int id = x.add(-2.25);
    id = x.add(-9.5);
    // cout<<x.get();
    id = x.add(.5);
    id = x.add(100.75);
    // x.retract(id);
    id = x.add(400);
    cout<<x.get();
    return 0;
}