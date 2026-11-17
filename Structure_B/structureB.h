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
    int p;
    vector<double> all;
    vector<pair<double,bool>> arr;
    int start = 0;
    FenTree ft;
    int size = 0;
    
public:
    StructureB (int w, int n, vector<double> a){
        if(w == 0){
            throw invalid_argument("Window of size zero is not allowed.");
        }
        this->win_size = w;
        this->p = n;
        this->all = a;
        sort(all.begin(),all.end());
        ft = FenTree(all.size());
    }

    void add(double x){
        size++;
        arr.push_back({x,true});
        if(size > win_size){
            int left = 1 + lower_bound(all.begin(),all.end(),arr[start].first) - all.begin();
            int right = 1 + lower_bound(all.begin(),all.end(),arr.back().first) - all.begin();
            ft.add(left , -1);
            ft.add(right , 1);
            start++;
            while(!arr[start].second){
                start++;
            }
            size--;
        }else{
            int idx = 1 + lower_bound(all.begin(),all.end(),arr.back().first) - all.begin();
            ft.add(idx , 1);
        }
    }

    void retract(double x){
        int idx = 1 + lower_bound(all.begin(),all.end(),x) - all.begin();
        int val = ft.sum(idx) - (idx > 1 ? ft.sum(idx - 1) : 0);
        for(int i = start;i<arr.size();i++){
            if(arr[i].first == x){
                arr[i].second = false;
            }
        }
        size -= val;
        while(!arr[start].second && start < arr.size()-1){
            start++;
        }
        ft.add(idx , -val);
    }

    double get(){
        if(size == 0){
            throw invalid_argument("No elements present");
        }
        
        // median is found out using binary lifting
        int i = log2(ft.n);
        int j = 0;
        int sum = 0;
        int target = size - size/2; // this i have set for lower median
        int ans = j;
        while(i>=0){
            j += (1<<i);
            if(j > n){
                j -= (1<<i);
                i--;
                continue;
            }
            sum += ft.bit[j];

            if(sum >= target){
                ans = j;
                sum -= (ft.bit[j]);
                j -= (1<<i);
            }
            i--;
        }
        if(sum < target){
            return all[ans-1];
        }
        return all[j-1];
    }
};
