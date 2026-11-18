# Formulas used in the benchmark

Let N be stream length, w window size, n live readings, D dead heap entries, H physical heap entries, U distinct compressed values, and theta = purge_factor - 1. All logarithms below have base two.

## Structure A: heaps and purge

A binary heap of height h holds approximately 2^h entries, so solving for h gives logarithmic depth. The script uses log2(w+1) as a simple depth proxy. Actual physical heap depth depends on H, including dead entries.

H = n + D. The purge condition H > (1+theta)n is equivalent to D > theta*n. After the purge check, H <= (1+theta)n (and an empty live set is cleared).

At rebuilding, n < D/theta, so H = n+D < (1+1/theta)D. Filtering and Floyd heap construction cost O(H). Charging this rebuild to the deletions that created D gives O(1+1/theta) amortized rebuild overhead per deletion. This does not make each individual purge constant time.

For fixed theta, heap operations cost O(log(w+1)) amortized, with expected hash-table costs. The script's dimensionless trend score is:

`A_score = log2(w+1) + 1 + 1/theta`

The coefficients are illustrative, not fitted runtime constants; this score does not model retraction probability explicitly.

## Structure B: Fenwick tree

`lowbit(i) = i & (-i)` extracts the least significant set bit. An update advances by lowbit; each jump clears a binary position in the relevant index progression. At most O(log(U+1)) jumps are needed. Binary lifting also checks one candidate per binary position, so finding a rank costs O(log(U+1)).

Median ranks (integer division) are lower=(n+1)/2 and upper=n/2+1. For odd n they coincide; for even n they are adjacent. The median is the average of their values. One or two rank searches remain O(log(U+1)).

After compression, m insertions, d successful removals (including expiry) and q median queries cost O((m+d+q)log(U+1)), including logarithmic value lookup. Preprocessing sorts N input values and costs O(N log N); it is recorded separately as build_ms.

The script uses `B_score = 3*log2(U+1)` as a simple three-operation trend reference. It is not an exact count: expiry, optional retraction, binary searches and the two middle-rank searches change constants. The measured counter sum therefore need not equal this curve. Initial prediction uses min(N,range), an upper bound; analysis replaces it with actual measured U from the counted seed.

## Space

A's heap entries are O((1+theta)w), hence O(w) for fixed theta. This implementation also retains per-arrival metadata, so its benchmark storage order is O(N+w). B stores compressed values and tree counts in O(U), plus retained IDs/history in O(N), giving O(N+U). Input generation also uses O(N). These are storage orders, not predictions in MB.

## Measurements and comparisons

Windows counter ticks are converted by `milliseconds = tick_difference*1000/frequency`.

`latency_ns = run_ms*1,000,000/N`. N counts replay steps, not individual method calls. Input generation and construction are outside this replay interval; checksum accumulation and loop overhead are inside it.

`throughput = 1,000,000,000/mean_latency_ns`, in steps per second. This is reciprocal mean latency, not mean of per-run throughputs.

For R=5 runs, `mean = sum(t_i)/R`. Sample standard deviation is `sqrt(sum((t_i-mean)^2)/(R-1))`; R-1 corrects for estimating the mean from the sample. Graph error bars show this spread, not confidence intervals. The same formulas apply to memory.

`extra_peak_KiB = (PeakWorkingSetSize - baseline_WorkingSetSize)/1024`. The peak is process-lifetime resident memory, so subtracting a current baseline is an approximate incremental metric and may include earlier peaks. `MB = KiB*1024/1,000,000` converts to decimal MB.

`counter_per_step = total_counter/N`. Analysis sums A's comparisons, swaps, hash probes and purge scans, or B's Fenwick and binary-lifting steps. These are selected instrumentation events, not all CPU operations, and different event types have different costs.

`speedup_A_over_B = mean_B_latency/mean_A_latency`. Above one means A is faster; below one means B is faster. Only matching settings are paired.

`checksum = sum(nonempty_medians)` provides an aggregate comparison for identical seed/settings. Empty medians are skipped. Successful retractions are reported separately because a requested ID may already be inactive.
