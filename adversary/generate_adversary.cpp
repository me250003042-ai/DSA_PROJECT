// adversary/generate_adversary.cpp  (owner: Adversary / D5 - Stavan Ingaldar, 250041037)
// Compiles and runs standalone right now even if src/structA/medianA.h or
// src/structB/medianB.h have not been pushed yet, and automatically switches
// to the real headers as soon as D1 and D2 push them!

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include "adversary.hpp"

using namespace std;

// Operation counters expected by -DCOUNT_OPS in bench.cpp
#if !__has_include("../src/structA/medianA.h")
long long g_heap_compares = 0;
long long g_heap_swaps = 0;
long long g_hash_probes = 0;
long long g_purge_moves = 0;
#endif

#if !__has_include("../src/structB/medianB.h")
long long g_fen_steps = 0;
long long g_bin_steps = 0;
#endif

// ---------------------------------------------------------------------------
// Structure A: Include D1's header if present; otherwise use built-in fallback
// ---------------------------------------------------------------------------
#if __has_include("../src/structA/medianA.h")
#include "../src/structA/medianA.h"
#else
class StructureA {
    struct Item {
        int val;
        int id;
        bool operator<(const Item& o) const {
            g_heap_compares++;
            return (val != o.val) ? (val < o.val) : (id < o.id);
        }
        bool operator>(const Item& o) const {
            g_heap_compares++;
            return (val != o.val) ? (val > o.val) : (id > o.id);
        }
    };
    int w, pct;
    double purgeFactor = 2.0;
    int nextId = 0;
    int liveLow = 0, liveHigh = 0, deadCount = 0;
    priority_queue<Item> hMax;
    priority_queue<Item, vector<Item>, greater<Item>> hMin;
    unordered_map<int, int> state; // id -> 0 (in hMax), 1 (in hMin), -1 (dead)
    vector<int> valById;

    void cleanTops() {
        while (!hMax.empty()) {
            g_hash_probes++;
            if (state[hMax.top().id] == -1) {
                hMax.pop();
                g_heap_swaps++;
                deadCount--;
            } else break;
        }
        while (!hMin.empty()) {
            g_hash_probes++;
            if (state[hMin.top().id] == -1) {
                hMin.pop();
                g_heap_swaps++;
                deadCount--;
            } else break;
        }
    }

    void purge() {
        vector<Item> keepMax, keepMin;
        while (!hMax.empty()) {
            Item x = hMax.top(); hMax.pop();
            g_purge_moves++;
            if (state[x.id] != -1) keepMax.push_back(x);
        }
        while (!hMin.empty()) {
            Item x = hMin.top(); hMin.pop();
            g_purge_moves++;
            if (state[x.id] != -1) keepMin.push_back(x);
        }
        for (const auto& x : keepMax) hMax.push(x);
        for (const auto& x : keepMin) hMin.push(x);
        deadCount = 0;
    }

    void rebalance() {
        cleanTops();
        int totalLive = liveLow + liveHigh;
        int targetLow = (totalLive * pct + 99) / 100;
        if (targetLow < 1 && totalLive > 0) targetLow = 1;

        while (liveLow > targetLow && !hMax.empty()) {
            cleanTops();
            if (hMax.empty()) break;
            Item x = hMax.top(); hMax.pop();
            hMin.push(x);
            state[x.id] = 1;
            liveLow--; liveHigh++;
            g_heap_swaps += 2;
            cleanTops();
        }
        while (liveLow < targetLow && !hMin.empty()) {
            cleanTops();
            if (hMin.empty()) break;
            Item x = hMin.top(); hMin.pop();
            hMax.push(x);
            state[x.id] = 0;
            liveHigh--; liveLow++;
            g_heap_swaps += 2;
            cleanTops();
        }
        if (deadCount > (purgeFactor - 1.0) * (liveLow + liveHigh + 1)) {
            purge();
        }
    }

public:
    StructureA(int window, int percentile) : w(window), pct(percentile) {
        valById.push_back(0); // 1-indexed
    }
    void setPurgeFactor(double f) { purgeFactor = f; }

    void add(int val) {
        int id = ++nextId;
        valById.push_back(val);
        // Sliding window expiry of reading (id - w)
        if (id > w) {
            int oldId = id - w;
            g_hash_probes++;
            auto it = state.find(oldId);
            if (it != state.end() && it->second != -1) {
                if (it->second == 0) liveLow--;
                else liveHigh--;
                it->second = -1;
                deadCount++;
            }
        }
        cleanTops();
        Item item{val, id};
        if (hMax.empty() || val <= hMax.top().val) {
            hMax.push(item);
            state[id] = 0;
            liveLow++;
        } else {
            hMin.push(item);
            state[id] = 1;
            liveHigh++;
        }
        g_heap_swaps++;
        rebalance();
    }

    bool retract(int id) {
        if (id < 1 || id > nextId) return false;
        g_hash_probes++;
        auto it = state.find(id);
        if (it == state.end() || it->second == -1) return false;
        if (it->second == 0) liveLow--;
        else liveHigh--;
        it->second = -1;
        deadCount++;
        rebalance();
        return true;
    }

    double get() {
        cleanTops();
        if (liveLow + liveHigh == 0) return 0.0;
        if (pct == 50 && ((liveLow + liveHigh) % 2 == 0) && !hMax.empty() && !hMin.empty()) {
            return 0.5 * ((double)hMax.top().val + (double)hMin.top().val);
        }
        return !hMax.empty() ? (double)hMax.top().val : 0.0;
    }
};
#endif

// ---------------------------------------------------------------------------
// Structure B: Include D2's header if present; otherwise use built-in fallback
// ---------------------------------------------------------------------------
#if __has_include("../src/structB/medianB.h")
#include "../src/structB/medianB.h"
#else
class StructureB {
    int w, pct, U, bitMask;
    int nextId = 0, liveCount = 0;
    vector<int> coords, bit, rankById;
    vector<char> active;

    void bitAdd(int idx, int delta) {
        for (; idx <= U; idx += idx & -idx) {
            bit[idx] += delta;
            g_fen_steps++;
        }
    }

    int kth(int k) {
        int pos = 0;
        for (int step = bitMask; step > 0; step >>= 1) {
            g_fen_steps++;
            if (pos + step <= U && bit[pos + step] < k) {
                pos += step;
                k -= bit[pos];
            }
        }
        return coords[pos];
    }

public:
    StructureB(int window, int percentile, const vector<int>& allValues)
        : w(window), pct(percentile) {
        coords = allValues;
        sort(coords.begin(), coords.end());
        coords.erase(unique(coords.begin(), coords.end()), coords.end());
        U = (int)coords.size();
        bit.assign(U + 1, 0);
        bitMask = 1;
        while ((bitMask << 1) <= U) bitMask <<= 1;
        rankById.push_back(0);
        active.push_back(0);
    }

    void add(int val) {
        int id = ++nextId;
        if (id > w && active[id - w]) {
            active[id - w] = 0;
            bitAdd(rankById[id - w], -1);
            liveCount--;
        }
        int lo = 0, hi = U - 1;
        while (lo <= hi) {
            g_bin_steps++;
            int mid = lo + (hi - lo) / 2;
            if (coords[mid] < val) lo = mid + 1;
            else hi = mid - 1;
        }
        int rnk = lo + 1;
        rankById.push_back(rnk);
        active.push_back(1);
        bitAdd(rnk, +1);
        liveCount++;
    }

    bool retract(int id) {
        if (id < 1 || id > nextId || !active[id]) return false;
        active[id] = 0;
        bitAdd(rankById[id], -1);
        liveCount--;
        return true;
    }

    double get() {
        if (liveCount == 0) return 0.0;
        if (pct == 50 && (liveCount % 2 == 0)) {
            int a = kth(liveCount / 2);
            int b = kth(liveCount / 2 + 1);
            return 0.5 * ((double)a + (double)b);
        }
        int k = (liveCount * pct + 99) / 100;
        if (k < 1) k = 1;
        return (double)kth(k);
    }
};
#endif

long readStatusKb(string key) {
    ifstream f("/proc/self/status");
    string line;
    while (getline(f, line)) {
        if (line.compare(0, key.size(), key) == 0) {
            return atol(line.c_str() + key.size());
        }
    }
    return 0;
}

void runOneAdversary(const string& advName, const string& which, int w, int retractPct,
                     int n, int percentile, unsigned seed, double purgeFactor, ostream& out) {
    adversary::StreamInput in;
    if (advName == "adv1_expiry_funnel") {
        in = adversary::make_expiry_funnel(n, w);
    } else if (advName == "adv2_buried_retract") {
        in = adversary::make_buried_retractions(n, w, retractPct);
    } else if (advName == "adv3_pinned_memory") {
        in = adversary::make_pinned_memory(n, w);
    } else {
        in = adversary::make_fenwick_blowup(n, w, retractPct, seed);
    }

    long baseKb = readStatusKb("VmRSS:");

#ifdef COUNT_OPS
    g_heap_compares = 0; g_heap_swaps = 0; g_hash_probes = 0;
    g_purge_moves = 0; g_fen_steps = 0; g_bin_steps = 0;
#endif

    auto t0 = chrono::steady_clock::now();
    StructureA *a = 0;
    StructureB *b = 0;
    if (which == "A") {
        a = new StructureA(w, percentile);
        a->setPurgeFactor(purgeFactor);
    } else {
        b = new StructureB(w, percentile, in.values);
    }
    auto t1 = chrono::steady_clock::now();

    double checksum = 0;
    long retractDone = 0;
    for (int i = 0; i < n; i++) {
        if (which == "A") {
            a->add(in.values[i]);
            if (in.retractId[i] > 0) {
                if (a->retract(in.retractId[i])) retractDone++;
            }
            checksum += a->get();
        } else {
            b->add(in.values[i]);
            if (in.retractId[i] > 0) {
                if (b->retract(in.retractId[i])) retractDone++;
            }
            checksum += b->get();
        }
    }
    auto t2 = chrono::steady_clock::now();

    long peakKb = readStatusKb("VmHWM:");
    double buildMs = chrono::duration<double, milli>(t1 - t0).count();
    double runMs = chrono::duration<double, milli>(t2 - t1).count();

    out.precision(12);
    out << advName << "," << which << "," << w << "," << retractPct << "," << n << ","
        << percentile << "," << seed << "," << purgeFactor << ","
        << buildMs << "," << runMs << "," << (runMs * 1e6 / n) << ","
        << (peakKb - baseKb) << "," << checksum << "," << retractDone;
#ifdef COUNT_OPS
    out << "," << g_heap_compares / (double)n << "," << g_heap_swaps / (double)n
        << "," << g_hash_probes / (double)n << "," << g_purge_moves / (double)n
        << "," << g_fen_steps / (double)n << "," << g_bin_steps / (double)n;
#endif
    out << "\n";

    delete a;
    delete b;
}

int main(int argc, char **argv) {
    int n = (argc > 1) ? atoi(argv[1]) : 1000000;
    ofstream f("results/adversary.csv");
    ostream& out = f.is_open() ? f : cout;

    out << "adversary,structure,w,retract_pct,N,percentile,seed,purge_factor,"
        << "build_ms,run_ms,ns_per_step,peak_kb,checksum,retractions_done";
#ifdef COUNT_OPS
    out << ",heap_compares,heap_swaps,hash_probes,purge_moves,fen_steps,bin_steps";
#endif
    out << "\n";

    runOneAdversary("adv1_expiry_funnel", "A", 10000, 0, n, 50, 1, 1000000.0, out);
    runOneAdversary("adv1_expiry_funnel", "A", 10000, 0, n, 50, 1, 2.0, out);

    runOneAdversary("adv2_buried_retract", "A", 10000, 90, n, 50, 1, 1000000.0, out);
    runOneAdversary("adv2_buried_retract", "A", 10000, 90, n, 50, 1, 2.0, out);

    runOneAdversary("adv3_pinned_memory", "A", 10000, 50, n, 50, 1, 2.0, out);

    runOneAdversary("adv4_fenwick_blowup", "A", 100, 90, n, 50, 1, 2.0, out);
    runOneAdversary("adv4_fenwick_blowup", "B", 100, 90, n, 50, 1, 2.0, out);

    cout << "wrote results/adversary.csv\n";
    return 0;
}

