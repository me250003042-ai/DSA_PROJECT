#include <iostream>
#include <vector>
#define ll long long
using namespace std;

class fenTree{
    int n;
    vector<ll> bit;

public:

    void update(int i,ll x){
        while(i<=n){
            bit[i] += x;
            i += (i & (-i));
        }
    }

    fenTree(vector<ll> arr){
        n = arr.size() - 1;
        this->bit.resize(n + 1);
        for(int i = 1;i<=n;i++){
            update(i,arr[i]);
        }
    }

    ll sum(int i){
        ll ans = 0;
        while(i>0){
            ans += bit[i];
            i -= (i & (-i));
        }
    }  
};

int main(){
    
    return 0;
}