// adversary/adversary.hpp  (owner: Adversary / D5 - Stavan Ingaldar, 250041037)
// Worst-case input generators for Project 8 (Running median with retractions).
// Aligned directly with the (values[n], retractId[n]) 1-indexed step interface in bench.cpp.

#ifndef ADVERSARY_HPP
#define ADVERSARY_HPP

#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <cstdint>

namespace adversary {

struct StreamInput {
    std::vector<int> values;     // values[i] added at step i (gets 1-indexed id = i + 1)
    std::vector<int> retractId;  // 0 = no retraction; otherwise 1-indexed id (1 .. i+1) to retract
};

/**
 * Target 1A: Converging Expiry Funnel (Maximizes dead entries in Structure A via window expiry).
 * Odd steps insert -(N - i), even steps insert +(N - i).
 * Every reading that expires from the window of size w is strictly smaller than root(H_max)
 * or strictly larger than root(H_min). Without purge (purgeFactor >= N), 100% of expired
 * readings (N - w) stay trapped inside the heaps! With purge, forces maximal purge rebuilds.
 */
inline StreamInput make_expiry_funnel(int n, int w) {
    (void)w;
    StreamInput in;
    in.values.resize(n);
    in.retractId.assign(n, 0);
    for (int i = 0; i < n; i++) {
        int mag = n - i;
        in.values[i] = (i % 2 == 0) ? -mag : +mag;
    }
    return in;
}

/**
 * Target 1B: Buried Retraction Stream (Maximizes dead entries in Structure A via retractions).
 * Phase 1 (first guardCount steps): Inserts live "sentinel" readings near 0: [-1, +1, -2, +2, ...]
 *   which sit at the roots of H_max and H_min and are NEVER retracted inside the window.
 * Phase 2: Inserts extreme values far outside the sentinels (-(10^8 + i) or +(10^8 + i)) and
 *   immediately retracts them on the same step (retractId[i] = i + 1). Because they sit below
 *   the sentinel roots, clean_top() can NEVER pop them!
 * Whenever the sliding window is about to evict the guards, refreshes the guards.
 */
inline StreamInput make_buried_retractions(int n, int w, int retractPct = 100) {
    StreamInput in;
    in.values.resize(n);
    in.retractId.assign(n, 0);

    int guardSpan = std::max(4, std::min(w / 4, 2000));
    const int EXTREME = 100000000;

    for (int i = 0; i < n; i++) {
        int posInCycle = i % (w / 2 + 1);
        if (posInCycle < guardSpan) {
            // Insert fresh live sentinel guards close to 0 so roots of both heaps stay live
            int g = (posInCycle / 2) + 1;
            in.values[i] = (posInCycle % 2 == 0) ? -g : +g;
            in.retractId[i] = 0;
        } else {
            // Insert an extreme value buried deep inside the heap
            int mag = EXTREME + i;
            in.values[i] = (i % 2 == 0) ? -mag : +mag;
            // Retract this buried reading immediately (1-indexed ID is i + 1)
            if ((i % 100) < retractPct) {
                in.retractId[i] = i + 1;
            }
        }
    }
    return in;
}

/**
 * Target 1C: Pinned Peak-Memory Stream (Hovers right at the purge threshold without purging).
 * For purgeFactor = 2.0 (where purge triggers when total_size > 2.0 * live_size, i.e., D > L),
 * keeps L live sentinels around 0 and buries exactly D = L - 1 retracted entries inside the heaps.
 * Resident heap size stays pinned at ~2*L (maximum possible dead entries) across the run.
 */
inline StreamInput make_pinned_memory(int n, int w) {
    StreamInput in;
    in.values.resize(n);
    in.retractId.assign(n, 0);

    int L = std::max(4, w / 2);
    if (L % 2 != 0) L--;
    const int EXTREME = 100000000;

    for (int i = 0; i < n; i++) {
        int cycle = i % w;
        if (cycle < L) {
            // Live sentinel near 0 (never retracted, expires naturally after w steps)
            int g = (cycle / 2) + 1;
            in.values[i] = (cycle % 2 == 0) ? -g : +g;
            in.retractId[i] = 0;
        } else {
            // Buried extreme entry, retracted immediately so it stays dead inside the heap
            int mag = EXTREME + i;
            in.values[i] = (i % 2 == 0) ? -mag : +mag;
            in.retractId[i] = i + 1;
        }
    }
    return in;
}

/**
 * Target 2: Fenwick Compressed-Domain Blowup (Maximizes U / live_count for Structure B).
 * Generates N strictly distinct values scattered via golden-ratio hashing so the coordinate-
 * compressed Fenwick array has maximum domain U = N, while high retractions and small w keep
 * the live count tiny (live << U). Forces Structure B to allocate O(N) memory and take
 * floor(log2(N)) + 1 binary-lifting steps per query across cache-hostile indices.
 */
inline StreamInput make_fenwick_blowup(int n, int w, int retractPct = 90, unsigned seed = 1) {
    StreamInput in;
    in.values.resize(n);
    in.retractId.assign(n, 0);
    std::mt19937 rng(seed);

    for (int i = 0; i < n; i++) {
        // Bijective 30-bit golden-ratio scatter guarantees all n values are 100% distinct
        uint32_t scatter = (uint32_t)((uint64_t)(i + 1) * 2654435761ULL) & 0x3FFFFFFFU;
        in.values[i] = (int)scatter;
        if (i > 0 && (int)(rng() % 100) < retractPct) {
            int back = (int)(rng() % std::min(i + 1, w));
            int id = (i + 1) - back;
            if (id < 1) id = 1;
            in.retractId[i] = id;
        }
    }
    return in;
}

} // namespace adversary

#endif // ADVERSARY_HPP
