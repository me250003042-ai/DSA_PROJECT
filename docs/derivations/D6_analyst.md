# D6: Benchmark models and derivations

Om Sunil Jain
Roll number: 250003042
Role: Analyst
Project 8: Running Median with Retractions

## 1. Workload and notation

Each replay step adds a double reading, optionally retracts a reading by unique ID, and requests the median. Expiry follows the last w arrivals, independent of retractions. N is stream length, w window size, L live readings, D dead heap entries, H physical heap entries, U distinct compressed values, and alpha the dead-to-live purge threshold. All logarithms below have base two.

There are four experiment families: window size (8 settings), retraction probability (12), A purge factor (6), and B domain size (5). Together these give 31 settings. Five seeds give 155 timing runs, with N=1,000,000 steps per run. Operation counting and percentile experiments are excluded.

## 2. Structure A: heap bound and purge threshold

Lazy deletion leaves some dead readings physically stored. Therefore:

H = L + D

The generalized purge condition is D > alpha*L, with alpha > 0. Adding L to both sides gives H > (1+alpha)*L. Thus the code's purge factor f equals 1+alpha. The official threshold alpha=1 corresponds to f=2.

After purge checks, D <= alpha*L and consequently H <= (1+alpha)*L <= (1+alpha)*w. For f=2, both heaps together hold at most 2w entries after checks. This bounds entry count, not total resident memory or temporary rebuild allocations.

## 3. Structure A: amortized rebuild derivation

At a purge, D > alpha*L implies L < D/alpha. Substituting into H=L+D gives:

H < D/alpha + D = (1 + 1/alpha)*D

Filtering heap entries costs O(H). Floyd's bottom-up heap construction costs O(L): nodes at height h number at most proportional to L/2^(h+1), so total sifting work is bounded by L times the convergent sum of h/2^(h+1). As L <= H, heap rebuilding costs O(H).

Each dead entry originated in a previous successful removal and is charged at most once to a purge before disappearing. Dividing the rebuild bound across the D deletions gives O(1+1/alpha) amortized heap-rebuild overhead per deletion. For fixed alpha=1 this is O(1). A single purge still has linear cost.

The proof covers heap filtering and reconstruction. The integrated hash table can also rehash while removing records. Full-table scans, probing and allocation add work outside this simple rebuild reference.

## 4. Structure A: depth and time model

A complete binary tree of height h contains between 2^h and 2^(h+1)-1 nodes. Taking logarithms gives h=O(log(H+1)). With fixed alpha and H=O(w), heap pushes and pops cost O(log(w+1)). Lazy root cleanup is amortized by charging each dead entry's eventual pop to its earlier deletion.

Assuming expected constant-time hash operations and separately amortized hash maintenance, the intended update bound is O(log(w+1)) amortized. The current hash implementation may have long probe sequences, so measured runtime need not follow this ideal heap-only bound.

The script's dimensionless trend reference is:

C_A = log2(w+1) + 1 + 1/alpha

It combines a heap-depth proxy and a purge-overhead reference. Its coefficients are illustrative. It does not predict nanoseconds or explicitly model retraction probability, allocation or hash probing.

## 5. Structure B: Fenwick derivation

Coordinate compression sorts the known input values and removes duplicates. Each distinct double receives an index from 1 to U. A hash map translates values to indices with expected O(1) lookup. Sorting the N values costs O(N log N), measured separately as construction time.

lowbit(i) = i & (-i)

In a Fenwick update, i advances by lowbit(i). This moves the lowest set bit upward. At most O(log(U+1)) binary positions are visited. Prefix-sum queries clear the lowest set bit and have the same bound.

Binary lifting considers powers of two from the largest not exceeding U down to one. There are floor(log2(U))+1 positions for U >= 1. Each decision takes constant work, giving O(log(U+1)) to locate the kth reading by cumulative frequency.

## 6. Median ranks and total replay work

Using integer division, lower rank=(L+1)/2 and upper rank=L/2+1. For odd L both equal the middle rank. For even L they are consecutive middle ranks.

median = lower_value/2 + upper_value/2

The implementation searches once for odd L and twice for even L. Both cases cost O(log(U+1)). Empty queries return NAN. Frequency counts remain integers even though readings are double.

For m insertions, d successful removals including expiry, and q median queries, B's post-compression work is O((m+d+q)*log(U+1)), with expected hash costs. The reference score is C_B=3*log2(U+1). The coefficient three represents a simple aggregate trend, not an exact call count. Actual steps can include expiry and explicit retraction, and even medians require two searches. Analysis uses actual U averaged over the five seeds rather than treating the value-range bound as exact U.

## 7. Memory comparison

A's physical heap entries are O((1+alpha)*w). Its circular expiry queue is O(w). Hash capacity, tombstones, retained vector capacity and temporary rehash/rebuild buffers also consume memory. A conservative benchmark storage upper bound is O(N+w); it should not be read as saying every implementation retains N live records.

B stores O(U) compressed values, frequencies and hash entries, plus O(N) value/active records retained by ID. Its storage is O(N+U). Expired values remain in the known domain, so retracting a reading does not shrink that domain storage. Input generation also consumes O(N), but occurs before the memory baseline.

These are growth bounds, not byte-accurate predictions. Actual sizeof values, vector capacities, allocator overhead, hash buckets and temporary copies affect resident MB.

## 8. Timing and statistical derivations

Let delta be elapsed QueryPerformanceCounter ticks and F its ticks-per-second frequency. Elapsed seconds=delta/F, hence milliseconds=1000*delta/F. Construction uses its own interval; replay timing excludes input generation and construction but includes loop and checksum work.

latency_ns_per_step = run_ms * 1,000,000 / N

This divides replay time by the number of complete steps, not the number of individual method calls. Across R=5 runs, mean=sum(t_i)/R. Sample standard deviation is sqrt(sum((t_i-mean)^2)/(R-1)). The denominator R-1 accounts for estimating the mean from the same sample. Graph error bars show this spread, not confidence intervals.

throughput_steps_per_second = 1,000,000,000 / mean_latency_ns

speedup_of_A = mean_latency_B / mean_latency_A

A speedup above one means A is faster; below one means B is faster. Only matching settings are paired. The throughput is reciprocal mean latency, not mean of individual throughputs.

## 9. Resident-memory measurement

Windows GetProcessMemoryInfo reports current WorkingSetSize and process-lifetime PeakWorkingSetSize in bytes. The baseline is sampled after input generation and before construction.

extra_peak_KiB = (PeakWorkingSetSize - baseline_WorkingSetSize) / 1024

extra_peak_MB = extra_peak_KiB * 1024 / 1,000,000

This is approximate additional resident process memory. A lifetime peak can reflect earlier input-generation activity, while the baseline is current memory. Therefore subtraction does not isolate exact structure allocations. Construction contributes to the peak even though construction time is excluded from replay latency. The same mean and sample-SD formulas summarize memory across seeds.

## 10. Graph construction and interpretation

Graphs 1 and 2 vary w and compare mean latency and resident memory. Graphs 3 and 4 vary retraction request probability, which differs from successful-retraction fraction. Graph 5 places A's 1+1/alpha reference beside measured latency. Graph 6 places B's logarithmic reference beside measured latency against log2(U+1). Model units and nanoseconds are different quantities and appear in separate panels.

The runner compiles bench.cpp once and calls the corrected team structures directly. Python writes timing.csv, combines means, SD and models in comparison.csv, and draws six figures. There is no oracle inside the timed region. Separate differential tests compare every answer with a sorted reference. Matching benchmark checksums and successful-removal counts provide an additional smoke check; equal sums alone do not prove identical answers.

## 11. Limits of the comparison

The models explain growth and tradeoffs rather than exact runtime. Hash clustering, rehashes, cache behavior and allocation may dominate a measured curve. Changing alpha trades rebuilding work against retained heap entries, but no universal optimal factor follows from the reference alone. B requires its finite value domain in advance. Conclusions apply to the integrated implementations and workload, and should be supported by the current tables rather than older results.

Implementation references: bench/predict.py, bench/bench.cpp, bench/analyse.py, bench/make_figures.py, src/structA/teamA.h, src/structB/teamB.h and bench/TEAM_INTEGRATION.md. Current numerical results and graph analysis are in results/README.md.
