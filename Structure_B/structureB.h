#pragma once
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

    void add(int i,ll x){
        while(i<=n){
            bit[i] += x;
            i += (i & (-i));
        }
    }
    FenTree(){}

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

class StructureB{
    int win_size;
    int n;
    vector<double> all, arr;
    int start = 0;
    FenTree ft;
    
public:
    StructureB (int w, int n, vector<double> all){
        this->win_size = w;
        this->n = n;
        this->all = all;
        sort(all.begin(),all.end());
        ft = FenTree(all.size());
    }

    void add(double x){
        arr.push_back(x);
        if(arr.size() - start > win_size){
            int left = 1 + lower_bound(all.begin(),all.end(),arr[start]) - all.begin();
            int right = 1 + lower_bound(all.begin(),all.end(),arr.back()) - all.begin();
            ft.add(left , -1);
            ft.add(right , 1);
            start++;
        }else{
            int idx = 1 + lower_bound(all.begin(),all.end(),arr.back()) - all.begin();
            ft.add(idx , 1);
        }
    }

    void retract(double x){
        int idx = 1 + lower_bound(all.begin(),all.end(),x) - all.begin();
        int val = ft.sum(idx) - (idx > 1 ? ft.sum(idx - 1) : 0);
        ft.add(left , -val);
    }

    void get(){
        int i = log2(n);
        int j = 0;
        int sum = 0;
        int w = arr.size() - start;
        int target = w - w/2; // this i have set for lower median
        int ans = j;
        while(i>=0){
            j += (1<<i);
            if(j > n){
                j -= (1<<i);
                i--;
                continue;
            }
            sum += ft.bit[j];
            // cout<<i<<' '<<j<<' '<<sum<<endl;

            if(sum >= target){
                ans = j;
                sum -= (ft.bit[j]);
                j -= (1<<i);
            }
            i--;
        }
        if(sum < target){
            cout<<all[ans-1]<<endl;
        }else{
            cout<<all[j-1]<<endl;
        }
    }
};
