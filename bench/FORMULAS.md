# Comparison formulas

N is stream length, w window size, L live readings, D dead heap entries, H=L+D, U compressed distinct values. alpha=theta=purge_factor-1.

A purges when H>purge_factor*L, equivalently D>alpha*L. After checks H<=(1+alpha)L. At a purge L<D/alpha, so H<(1+1/alpha)D. Linear heap filtering and Floyd construction therefore cost O(1+1/alpha) amortized per deletion. This proof describes heap rebuilding. The team hash table can rehash during removal; that maintenance adds cost not captured by this reference.

Heap height is O(log H), giving O(log(w+1)) heap updates for fixed alpha. A's illustrative score is log2(w+1)+1+1/alpha. Hash-table work and constants are excluded.

Fenwick lowbit(i)=i & -i. Updates and binary lifting traverse O(log(U+1)) binary positions. Median ranks are (L+1)/2 and L/2+1 with integer division; average their values. One or two rank searches remain O(log(U+1)). B's unordered_map lookup is expected O(1). Reference score=3*log2(U+1), not an exact number of method calls or measured operations. Preprocessing sorts N values in O(N log N), recorded separately. Analysis uses actual U averaged over five timing seeds.

Conservative implementation storage bounds: A O(N+w), B O(N+U), including retained metadata/hash capacity. Heap entry count alone is O((1+alpha)w). B retains its full compressed domain. Input buffers also use O(N). None of these formulas predicts a number of resident MB.

Time in ms = counter tick difference*1000/counter frequency. Latency ns/step=run_ms*10^6/N. Mean=sum(t_i)/R, R=5. Sample SD=sqrt(sum((t_i-mean)^2)/(R-1)). Error bars show this SD, not confidence intervals. Throughput=10^9/mean_ns. Speedup A=mean_B/mean_A; above one means A is faster.

Extra peak resident KiB=(PeakWorkingSetSize-baseline_WorkingSetSize)/1024. Decimal MB=KiB*1024/10^6. Peak is process-lifetime peak, baseline is current working set, so this is approximate incremental process memory. Construction is excluded from replay time but included in the memory peak. Checksums sum nonempty medians; equal sums are only a smoke check, not proof of equal answers.
