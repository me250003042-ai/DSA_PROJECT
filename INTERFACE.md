# Common median interface

Constructors: StructureA(w), StructureB(w, allValues), Oracle(w).

- add(double) returns the next 1-based ID.
- retract(id) removes exactly one active reading and returns success. Invalid, expired or repeated IDs return false.
- get() returns NAN when empty, the middle value for odd live count, and the average of the middle values for even count.
- A also provides setPurgeFactor(f), with finite f>1.

Window means last w arrivals, regardless of explicit retractions. Values are double. B needs the complete finite input domain before construction and rejects unknown values.

A's adapter wraps team RunningMedian. Its lower max-heap and upper min-heap partition live values, with ceil(L/2) live entries in the lower heap. Heap roots are pruned before use. Hash entries track active state and heap side. Circular expiry queue tracks arrivals. Purge filters dead entries and rebuilds both heaps when H>f*L. Rehashing maintains the team's open-addressed table.

B stores compressed indices in an unordered_map, frequencies in a Fenwick tree, and value/active pairs indexed by native reading ID. Its adapter translates native 0-based IDs to common 1-based IDs. Expiry/retraction decrements each reading at most once. Fenwick total equals live count. selfCheck() validates counts independently for differential testing. The corrected team get() uses separate binary-lifting searches for the middle ranks when the live count is even.

The oracle keeps a sorted vector of live values. No operation counters are used in the integrated benchmark.
