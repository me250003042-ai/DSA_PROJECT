#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#define ll long long
using namespace std;

class FenTree{
public:
    int n;
    vector<ll> bit;

    void update(int i,ll x){
        while(i<=n){
            bit[i] += x;
            i += (i & (-i));
        }
    }

    FenTree(int n){
        this->n = n;
        bit.resize(n+1);
    }

    ll sum(int i){
        ll ans = 0;
        while(i>0){
            ans += bit[i];
            i -= (i & (-i));
        }
        return ans;
    }  
};

int main(){
    int n,win_size;
    cin>>n>>win_size;
    vector<double> a(n+1);  
    bool compression_required = false; // this will be true if the data needs compression

    for(int i = 1;i<=n;i++){
        cin>>a[i];
        if((a[i] > 1e5 || a[i] != (ll)a[i]) || a[i] < 0){
            compression_required = true;
        }
    }

    if(compression_required){
        vector<double> b = a;
        sort(b.begin() + 1,b.end());

        FenTree ft = FenTree(n);

        for(int i = 1;i<=win_size;i++){
            int idx = lower_bound(b.begin() + 1,b.end(), a[i]) - b.begin();
            ft.update(idx,1);
        }

        for(int z = 1;z <= n-win_size+1;z++){
            if(z > 1){
                int left = lower_bound(b.begin() + 1,b.end(), a[z-1]) - b.begin();
                int right = lower_bound(b.begin() + 1,b.end(), a[z-1+win_size]) - b.begin();
                ft.update(left,-1);
                ft.update(right, 1);
            }
            // code of binary lifting for finding median of each window

            int i = log2(n);
            int j = 0;
            int sum = 0;
            int target = win_size - win_size/2; // this i have set for lower median

            while(i>=0){
                j += (1<<i);
                if(j > n){
                    j -= (1<<i);
                    i--;
                    continue;
                }
                sum += ft.bit[j];
                // cout<<i<<' '<<j<<' '<<sum<<endl;

                if(sum > target){
                    sum -= (ft.bit[j]);
                    j -= (1<<i);
                }
                
                if(sum == target)break;
                i--;
            }
            cout<<b[j]<<endl;
        }

    }else{
        FenTree ft = FenTree(n);

        for(int i = 1;i<=win_size;i++){
            ft.update(a[i],1);
        }

        for(int z = 1;z <= n-win_size+1;z++){
            if(z > 1){
                ft.update(a[z-1],-1);
                ft.update(a[z-1-win_size], 1);
            }
            // code of binary lifting for finding median of each window

            int i = log2(n);
            int j = 0;
            int sum = 0;
            int target = win_size - win_size/2; // this i have set for lower median

            while(i>=0){
                j += (1<<i);
                if(j > n){
                    j -= (1<<i);
                    i--;
                    continue;
                }
                sum += ft.bit[j];
                // cout<<i<<' '<<j<<' '<<sum<<endl;

                if(sum > target){
                    sum -= (ft.bit[j]);
                    j -= (1<<i);
                }
                
                if(sum == target)break;
                i--;
            }
            cout<<a[j]<<endl;
        }

    }
    return 0;
}