#include "adversary.hpp"
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>

// Lightweight simulation of Structure A's lazy-deletion state for generating
// adversarial verification logs (D vs L with and without purge) for REPORT.md.
void run_dead_entry_audit(const std::vector<adversary::Operation>& ops,
                          int64_t window_size,
                          bool enable_purge,
                          const std::string& csv_path) {
    struct Entry {
        int64_t val;
        int64_t id;
        bool operator<(const Entry& o) const {
            return (val != o.val) ? (val < o.val) : (id < o.id);
        }
        bool operator>(const Entry& o) const {
            return (val != o.val) ? (val > o.val) : (id > o.id);
        }
    };

    std::priority_queue<Entry> h_max;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> h_min;
    std::unordered_map<int64_t, int> dead_ids;     // id -> 1 if dead
    std::unordered_map<int64_t, int> heap_side;    // id -> 0 for h_max, 1 for h_min
    std::deque<Entry> window_queue;

    int64_t live_low = 0, live_high = 0, dead_count = 0, purge_count = 0;

    auto clean_tops = [&]() {
        while (!h_max.empty() && dead_ids.count(h_max.top().id)) {
            dead_ids.erase(h_max.top().id);
            h_max.pop();
            --dead_count;
        }
        while (!h_min.empty() && dead_ids.count(h_min.top().id)) {
            dead_ids.erase(h_min.top().id);
            h_min.pop();
            --dead_count;
        }
    };

    auto purge_all = [&]() {
        std::vector<Entry> surv_max, surv_min;
        while (!h_max.empty()) {
            if (!dead_ids.count(h_max.top().id)) surv_max.push_back(h_max.top());
            h_max.pop();
        }
        while (!h_min.empty()) {
            if (!dead_ids.count(h_min.top().id)) surv_min.push_back(h_min.top());
            h_min.pop();
        }
        dead_ids.clear();
        for (const auto& e : surv_max) h_max.push(e);
        for (const auto& e : surv_min) h_min.push(e);
        dead_count = 0;
        ++purge_count;
    };

    auto rebalance = [&]() {
        clean_tops();
        while (live_low > live_high + 1) {
            clean_tops();
            Entry e = h_max.top(); h_max.pop();
            h_min.push(e);
            heap_side[e.id] = 1;
            --live_low; ++live_high;
            clean_tops();
        }
        while (live_high > live_low) {
            clean_tops();
            Entry e = h_min.top(); h_min.pop();
            h_max.push(e);
            heap_side[e.id] = 0;
            --live_high; ++live_low;
            clean_tops();
        }
    };

    std::ofstream out(csv_path);
    out << "step,live_entries,dead_entries,physical_heap_size,purge_count\n";

    int64_t step = 0;
    for (const auto& op : ops) {
        ++step;
        if (op.type == adversary::OpType::INSERT) {
            // Expire oldest if sliding window is full
            if (static_cast<int64_t>(window_queue.size()) >= window_size) {
                Entry old = window_queue.front();
                window_queue.pop_front();
                if (!dead_ids.count(old.id) && heap_side.count(old.id)) {
                    dead_ids[old.id] = 1;
                    if (heap_side[old.id] == 0) --live_low;
                    else --live_high;
                    ++dead_count;
                    heap_side.erase(old.id);
                }
            }
            Entry cur{op.value, op.id};
            window_queue.push_back(cur);
            clean_tops();
            if (h_max.empty() || cur.val <= h_max.top().val) {
                h_max.push(cur);
                heap_side[cur.id] = 0;
                ++live_low;
            } else {
                h_min.push(cur);
                heap_side[cur.id] = 1;
                ++live_high;
            }
            rebalance();
        } else if (op.type == adversary::OpType::RETRACT) {
            if (heap_side.count(op.id)) {
                dead_ids[op.id] = 1;
                if (heap_side[op.id] == 0) --live_low;
                else --live_high;
                ++dead_count;
                heap_side.erase(op.id);
                rebalance();
            }
        }

        int64_t live_total = live_low + live_high;
        if (enable_purge && dead_count > live_total) {
            purge_all();
        }

        if (step % 10000 == 0 || step == static_cast<int64_t>(ops.size())) {
            out << step << "," << live_total << "," << dead_count << ","
                << (h_max.size() + h_min.size()) << "," << purge_count << "\n";
        }
    }
    std::cout << "[Audit: " << csv_path << "] Final Live=" << (live_low + live_high)
              << " | Final Dead=" << dead_count
              << " | Total Purges=" << purge_count << "\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    adversary::GeneratorConfig cfg;
    cfg.num_ops = (argc > 1) ? std::stoll(argv[1]) : 1000000;
    cfg.window_size = (argc > 2) ? std::stoll(argv[2]) : 100000;
    cfg.seed = (argc > 3) ? std::stoull(argv[3]) : 250041037ULL;

    std::cout << "Generating adversarial suites (N=" << cfg.num_ops
              << ", w=" << cfg.window_size << ", seed=" << cfg.seed << ")...\n";

    auto funnel_ops = adversary::gen_expiry_funnel(cfg);
    run_dead_entry_audit(funnel_ops, cfg.window_size, false, "results/adv_funnel_no_purge.csv");
    run_dead_entry_audit(funnel_ops, cfg.window_size, true,  "results/adv_funnel_with_purge.csv");

    auto thrash_ops = adversary::gen_retraction_purge_thrash(cfg);
    run_dead_entry_audit(thrash_ops, cfg.window_size, true,  "results/adv_retraction_thrash.csv");

    auto pinned_ops = adversary::gen_retraction_pinned_memory(cfg);
    run_dead_entry_audit(pinned_ops, cfg.window_size, true,  "results/adv_retraction_pinned.csv");

    return 0;
}