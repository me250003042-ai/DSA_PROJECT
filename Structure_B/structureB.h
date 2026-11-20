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
    FenTree ft;
    int win_size;
    vector<double> all;
    unordered_map<double,int> h;
    vector<pair<double,bool>> arr;
    int start = 0;
    int size = 0;
    
public:
    StructureB (int w, vector<double> a){
        if(w <= 0){
            throw invalid_argument("Invalid window size");
        }
        this->win_size = w;
        sort(a.begin(),a.end());
        a.erase(unique(a.begin(), a.end()), a.end()); // to remove the duplicates from the array

        for(int i = 0;i<a.size();i++){
            all.push_back(a[i]);
            h[a[i]] = i;
        }
        ft = FenTree(all.size());
    }

    ll add(double x){
        if(h.find(x) == h.end()){
            throw invalid_argument("This element does not exists in the domain");
        }
        size++;
        arr.push_back({x,true});
        if(size > win_size){
            int left = 1 + h[arr[start].first];
            int right = 1 + h[arr.back().first];
            ft.add(left , -1);
            ft.add(right , 1);
            arr[start].second = false;
            while(!arr[start].second){
                start++;
            }
            size--;
        }else{
            int idx = 1 + h[arr.back().first];
            ft.add(idx , 1);
        }

        ll id = arr.size() - 1;
        return id;
    }

    bool retract(ll id){
        if(id < start || id >= arr.size()){
            throw invalid_argument("This id is not in the domain");
        }
        if(!arr[id].second){
            return true;
        }
        int idx = 1 + h[arr[id].first];
        arr[id].second = false;
        size --;
        while(start < arr.size() && !arr[start].second){
            start++;
        }
        ft.add(idx , -1);
        return true;
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
        double lower = all[ans - 1];
        if(size %  2){
            return lower;
        }

        i = log2(ft.n);
        j = 0;
        sum = 0;
        target = size - size/2 + 1; // this i have set for upper median
        ans = 0;
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
        double upper = all[ans - 1];
        double avg = lower + (upper - lower)/2;
        return avg;
    }
};
