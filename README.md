# CS 253 / MA 253 Lab Project 8: Running Median with Retractions

  Project Overview
This project implements and evaluates a high-throughput stream monitoring service that reports the running median (and general $p$-th percentiles) over a sliding window of the most recent $w$ readings (up to $w = 10^5$) across a stream of $N = 10^6$ operations, supporting arbitrary out-of-order retractions of faulty readings.

We compare two data structures written from scratch without standard library containers:
* **Structure A (`src/structA/`)**: Two binary heaps (a max-heap for the lower half and a min-heap for the upper half) generalized to maintain a p : (1-p) live-size ratio, paired with lazy deletion driven by a custom open-addressed hash table of pending removals and an automatic full-heap purge whenever dead entries outnumber live entries ($D > L$).
* **Structure B (`src/structB/`)**: A Fenwick tree (Binary Indexed Tree) over coordinate-compressed reading values holding frequency counts, answering arbitrary percentile queries in O(log U) time via binary-lifting descent.
* **Oracle (`oracle/`)**: A sorted array maintained via binary-search insertion and deletion, paired with structural invariant checkers and a $10^6$-operation seeded differential test harness with failing-case shrinking.
* **Adversary (`adversary/`)**: Worst-case input generators targeting lazy-deletion dead-entry accumulation ($D = \Theta(N)$ without purge, $D = L$ pinned memory, and purge thrashing) in Structure A, and compressed-domain blowup in Structure B.

---


---

 Repository Layout

```text
.
├── README.md                  # Build instructions, role table, and single-command runner
├── INTERFACE.md               # Common C++ interface and structural invariants
├── REPORT.md                  # Cost models, experimental figures, and adversarial analysis
├── TOOLS.md                   # Declaration of AI and profiling tools used
├── Makefile                   # Build and automation targets
├── src/
│   ├── structA/               # Two-heap with lazy deletion, hash table, and D > L purge
│   └── structB/               # Coordinate-compressed Fenwick tree with binary lifting
├── app/                       # Stream monitoring replay application with alert thresholds
├── oracle/                    # Sorted-array oracle, invariant checkers, and differential tester
├── adversary/                 # Adversarial generators (D5) for Heap burial & Fenwick blowup
├── bench/                     # Full-scale benchmark harness (N = 10^6, w in [10^2, 10^5])
├── docs/
│   └── derivations/           # D1.pdf through D6.pdf
└── results/                   # Raw CSV logs (with recorded seeds) and plotting scripts
