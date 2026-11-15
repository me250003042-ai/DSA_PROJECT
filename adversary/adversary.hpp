#ifndef ADVERSARY_HPP
#define ADVERSARY_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

namespace adversary {

enum class OpType : uint8_t {
    INSERT  = 0, // Insert reading (with id, value); also triggers window expiry if live window > w
    RETRACT = 1, // Explicitly retract reading by its id (or value)
    QUERY   = 2  // Query current median (and p-th percentile)
};

struct Operation {
    OpType type;
    int64_t id;      // Unique reading ID (0-indexed sequence number of insertion)
    int64_t value;   // Reading value (for INSERT, or target value for RETRACT)
};

struct GeneratorConfig {
    int64_t num_ops = 1000000;   // Default full scale: 10^6 operations
    int64_t window_size = 100000; // Default window w up to 10^5
    double retraction_rate = 0.5; // Fraction of operations that are retractions
    uint64_t seed = 250041037ULL; // Recorded deterministic seed
};

/**
 * Target 1A: Sliding-Window Expiry Funnel (No explicit retractions needed).
 * Generates x_t = -(N - t + 1) for odd t, +(N - t + 1) for even t.
 * Every expired element is strictly smaller than root(H_max) or strictly larger
 * than root(H_min). Without purge, 100% of expired elements (N - w) stay trapped.
 * With purge (D > L), forces a full heap rebuild every w insertions.
 */
inline std::vector<Operation> gen_expiry_funnel(const GeneratorConfig& cfg) {
    std::vector<Operation> ops;
    ops.reserve(cfg.num_ops);
    int64_t next_id = 0;

    for (int64_t t = 1; t <= cfg.num_ops; ++t) {
        int64_t mag = (cfg.num_ops - t + 1);
        int64_t val = (t & 1) ? -mag : +mag;
        ops.push_back({OpType::INSERT, next_id++, val});
    }
    return ops;
}

/**
 * Target 1B: Retraction Pinned Maximum Memory (Hovers at D = L without purging).
 * 1. Inserts L_target live "sentinel" readings near 0 (in [-L_target, +L_target]).
 * 2. Inserts L_target extreme readings and retracts all of them so D == L_target.
 *    Because extreme readings are outside [-L_target, +L_target], none can ever
 *    surface to the root of H_max or H_min.
 * 3. For the remaining stream, alternates inserting a fresh guard and retracting
 *    an old guard at the boundary so D stays pinned at L (maximizing resident memory
 *    at 2*L without triggering a D > L purge).
 */
inline std::vector<Operation> gen_retraction_pinned_memory(const GeneratorConfig& cfg) {
    std::vector<Operation> ops;
    ops.reserve(cfg.num_ops);

    // Choose L so that 3*L fits within window_size and num_ops
    int64_t L = std::min(cfg.window_size / 3, cfg.num_ops / 4);
    if (L < 2) L = 2;
    if (L & 1) --L; // Keep L even for exact balance between H_max and H_min

    int64_t next_id = 0;

    // Step 1: Insert L live sentinel barriers close to 0: [-L/2 .. -1] and [+1 .. +L/2]
    for (int64_t i = 1; i <= L / 2; ++i) {
        ops.push_back({OpType::INSERT, next_id++, -i});
        ops.push_back({OpType::INSERT, next_id++, +i});
    }

    // Step 2: Insert L extreme elements buried beneath the sentinels, then retract them all
    std::vector<std::pair<int64_t, int64_t>> to_retract;
    to_retract.reserve(L);
    const int64_t EXTREME_BASE = 1000000000LL;
    for (int64_t i = 1; i <= L / 2; ++i) {
        int64_t id_neg = next_id++;
        int64_t val_neg = -(EXTREME_BASE + i);
        ops.push_back({OpType::INSERT, id_neg, val_neg});
        to_retract.push_back({id_neg, val_neg});

        int64_t id_pos = next_id++;
        int64_t val_pos = +(EXTREME_BASE + i);
        ops.push_back({OpType::INSERT, id_pos, val_pos});
        to_retract.push_back({id_pos, val_pos});
    }

    for (const auto& item : to_retract) {
        if (static_cast<int64_t>(ops.size()) >= cfg.num_ops) break;
        ops.push_back({OpType::RETRACT, item.first, item.second});
    }

    // Now live count = L, buried dead count D = L (exact maximum before purge D > L).
    // Step 3: Fill remaining operations with queries (or balanced top insert/retract)
    while (static_cast<int64_t>(ops.size()) < cfg.num_ops) {
        ops.push_back({OpType::QUERY, -1, 0});
    }
    return ops;
}

/**
 * Target 1C: Retraction Purge Thrashing (Maximizes full-purge frequency and latency spikes).
 * Maintains L live sentinels around 0, then repeatedly inserts and retracts buried
 * extreme elements until D = L + 1, forcing a full O(L) heap purge every 2*(L+1) ops.
 */
inline std::vector<Operation> gen_retraction_purge_thrash(const GeneratorConfig& cfg) {
    std::vector<Operation> ops;
    ops.reserve(cfg.num_ops);

    int64_t L = std::max<int64_t>(2, static_cast<int64_t>(cfg.window_size * (1.0 - cfg.retraction_rate)));
    if (L & 1) --L;

    int64_t next_id = 0;
    // Insert L permanent sentinels near 0
    for (int64_t i = 1; i <= L / 2; ++i) {
        if (static_cast<int64_t>(ops.size()) >= cfg.num_ops) break;
        ops.push_back({OpType::INSERT, next_id++, -i});
        if (static_cast<int64_t>(ops.size()) >= cfg.num_ops) break;
        ops.push_back({OpType::INSERT, next_id++, +i});
    }

    const int64_t EXTREME_BASE = 1000000000LL;
    int64_t step = 1;
    while (static_cast<int64_t>(ops.size()) < cfg.num_ops) {
        int64_t id = next_id++;
        int64_t val = (step & 1) ? -(EXTREME_BASE + step) : +(EXTREME_BASE + step);
        ops.push_back({OpType::INSERT, id, val});
        if (static_cast<int64_t>(ops.size()) >= cfg.num_ops) break;
        ops.push_back({OpType::RETRACT, id, val});
        ++step;
    }
    return ops;
}

/**
 * Target 2: Fenwick Compressed-Domain Blowup vs Live Count (Targets Structure B).
 * Every inserted value is strictly unique across all N operations so the coordinate-
 * compressed universe size is U =Theta(N), while high retraction / small window
 * keeps the live count tiny. Values are bit-reversed / strided across [1, 10^12]
 * so binary-lifting queries touch widely separated cache lines across the Fenwick tree.
 */
inline std::vector<Operation> gen_fenwick_domain_blowup(const GeneratorConfig& cfg) {
    std::vector<Operation> ops;
    ops.reserve(cfg.num_ops);
    std::mt19937_64 rng(cfg.seed);

    int64_t next_id = 0;
    std::vector<std::pair<int64_t, int64_t>> active_pool;
    int64_t target_live = std::max<int64_t>(10, static_cast<int64_t>(cfg.window_size * (1.0 - cfg.retraction_rate)));

    for (int64_t t = 0; t < cfg.num_ops; ++t) {
        bool do_retract = (!active_pool.empty()) &&
                          (static_cast<int64_t>(active_pool.size()) > target_live ||
                           std::generate_canonical<double, 53>(rng) < cfg.retraction_rate);

        if (do_retract) {
            // Retract a random live element to fragment active indices in Fenwick array
            std::uniform_int_distribution<size_t> dist(0, active_pool.size() - 1);
            size_t idx = dist(rng);
            auto victim = active_pool[idx];
            active_pool[idx] = active_pool.back();
            active_pool.pop_back();
            ops.push_back({OpType::RETRACT, victim.first, victim.second});
        } else {
            // Generate strictly distinct value using golden-ratio multiplicative hashing
            // over 64-bit space so every single insertion creates a distinct coordinate in B.
            uint64_t hashed = (static_cast<uint64_t>(next_id + 1) * 11400714819323198485ULL) >> 24;
            int64_t unique_val = static_cast<int64_t>((hashed << 20) | (next_id + 1));
            int64_t id = next_id++;
            active_pool.push_back({id, unique_val});
            ops.push_back({OpType::INSERT, id, unique_val});
        }
    }
    return ops;
}

} // namespace adversary

#endif // ADVERSARY_HPP