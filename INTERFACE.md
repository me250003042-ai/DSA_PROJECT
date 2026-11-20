\# Common Median Interface



The application uses a common interface, `IOrderStatistic`, to run Structure A and Structure B through the same operations.



\## Interface Operations



\- `add(double value)` inserts a reading and returns its 1-based ID.

\- `retract(int id)` removes an active reading and returns `true` on success, otherwise `false`.

\- `getMedian()` returns the current median. For an empty window, it returns `NAN`.

\- `getSize()` returns the number of active readings.



\## Adapters



\- `StructureAAdapter` connects `IOrderStatistic` to Structure A (two heaps).

\- `StructureBAdapter` connects `IOrderStatistic` to Structure B (Fenwick tree).



Both adapters provide the same operations and use common 1-based reading IDs.



Structure A handles window expiry internally. Structure B also handles expiry internally; its adapter tracks active IDs and arrival order to provide the common interface.



\## Window and Input Rules



\- The window contains the last `w` arrivals, regardless of explicit retractions.

\- Values are stored as `double`.

\- Structure B requires the complete finite input domain (`allValues`) at construction and rejects values outside that domain.



\## Application



`app/monitor.cpp` uses the common interface to process sensor readings, retract faulty readings, compare active sizes and medians, and raise threshold alerts.



The application checks that both implementations produce matching IDs, retraction results, active sizes, and medians.
