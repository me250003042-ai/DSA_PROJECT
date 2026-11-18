#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <unordered_map>
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
    FenTree():n(0){}

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
    vector<double> arr;
    unordered_map<double,int> h;
    vector<pair<double,int>> all;
    int start = 0;
    FenTree ft;
    int size = 0;
    
public:
    StructureB (int w, int n, vector<double> a){
        if(w <= 0){
            throw invalid_argument("Invalid window size");
        }
        this->win_size = w;
        this->p = n;
        sort(a.begin(),a.end());
        a.erase(unique(a.begin(), a.end()), a.end()); // to remove the duplicates from the array

        for(int i = 0;i<a.size();i++){
            all.push_back({a[i],0});
            h[a[i]] = i;
        }
        ft = FenTree(all.size());
    }

    void add(double x){
        if(h.find(x) == h.end()){
            throw invalid_argument("This element does not exists in the domain");
        }
        size++;
        arr.push_back(x);
        if(size > win_size){
            int left = 1 + h[arr[start]];
            int right = 1 + h[arr.back()];
            ft.add(left , -1);
            ft.add(right , 1);
            start++;
            while(all[h[arr[start]]].second > 0){
                start++;
                all[h[arr[start]]].second--;
            }
            size--;
        }else{
            int idx = 1 + h[arr.back()];
            ft.add(idx , 1);
        }
    }

    void retract(double x){
        if(h.find(x) == h.end()){
            throw invalid_argument("This element does not exists in the domain");
        }
        int idx = 1 + h[x];
        int val = ft.sum(idx) - (idx > 1 ? ft.sum(idx - 1) : 0);
        all[h[x]].second += val;
        size -= val;
        while(all[h[arr[start]]].second > 0){
            start++;
            all[h[arr[start]]].second--;
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
        int ans = 0;
        while(i>=0){
            j += (1<<i);
            if(j > ft.n){
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
            return all[ans-1].first;
        }
        return all[j-1].first;
    }
};
