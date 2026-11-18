#include <iostream>
#include "./structureB.h"
using namespace std;

int main(){
    vector<double> a = {-2.25,-9.5,0.5,100.75,400};
    StructureB x = StructureB(4 , 2 ,a);
    x.add(-2.25);
    x.add(-9.5);
    x.add(.5);
    x.add(100.75);
    x.retract(-2.25);
    x.add(400);
    cout<<x.get();
    return 0;
}