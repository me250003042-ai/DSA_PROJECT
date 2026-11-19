# Team implementation integration

Source: GitHub main commit 54ed2b5, pulled into .review-team-a on 19 November 2026.

## A

Copied RunningMedian plus item, maxheap, minheap, hashtable and expiryqueue headers into src/structA. The latest team methods are add/get. medianA.h is a small adapter normalizing the return type and empty NAN. The team heaps, circular expiry queue, resizing/tombstone hash table and purge run in the benchmark.

Integration fixes: corrected include paths and case; rejected nonfinite or <=1 purge factors; rebalanced immediately after expiry before classifying a new reading. The last fix was required by differential seed 1282, step 42, window 3: original A returned 16.5 while oracle/B returned 15.5. The updated version passes that test.

## B

The new team B is copied into src/structB/teamB.h, including its vector of value/active pairs, unordered_map, Fenwick update and two binary-lifting searches for even medians. medianB.h only adapts native 0-based IDs to the common 1-based interface.

Local corrections: start=-1 became start=0; expiry now uses arrival count rather than live count; expired records are marked inactive; repeated/invalid retractions return false; unsafe start-scanning loops were removed; empty queries return NAN. Median averaging uses lower/2+upper/2 to avoid overflow in upper-lower. Added distinctValues() for CSV reporting and selfCheck() for tests. These are adapted team sources, not unmodified upstream sources.

Without these fixes, the upstream B accesses arr[-1] on retraction/window overflow and can retain readings past their arrival window or decrement expired readings again. The corrected code passed 300 randomized runs and a million-operation differential run against the oracle before benchmarking.

## Benchmark changes

Removed all operation-count instrumentation and counting runs. Only timing, memory, preprocessing time, throughput and model references remain. B's hash lookup changes the runtime constant compared with the previous binary-search lookup. All old numerical results must be replaced.

Hash maintenance in A includes full-table rehashes. The 1+1/alpha reference describes heap rebuilding only, not the complete purge/rehash runtime. Existing space formulas are conservative benchmark bounds, not promises that A uses only O(w) resident bytes.
