# CS 253 / MA 253 Lab Project 8: Running Median with Retractions

## Team and roles

| Name | GitHub | Role | Module folder | Derivation |
|---|---|---|---|---|
| Yata Naresh |  | Structure A owner | `src/structA/` | D1 |
| Atharv Vyas |  | Structure B owner | `src/structB/` | D2 |
| Rajewar Shivram |  | Architect | `app/` | D3 |
| Abhishek Singh |  | Oracle owner | `oracle/` | D4 |
| Stavan Ingaldar |  | Adversary | `adversary/` | D5 |
| Om Sunil Jain | `@me250003042-ai` | Analyst | `bench/` | D6 |

## Integrated Windows benchmark

Run `python bench/run_bench.py` from the repository root. Requires g++ C++17, Python and matplotlib. The corrected team implementation copies used by the benchmark are under `src/`; original team sources remain under `Structure_A/` and `Structure_B/`. See [integration changes](bench/TEAM_INTEGRATION.md), [benchmark instructions](bench/README.md), and [current results](results/README.md). Operation counting has been removed. Run `powershell -NoProfile -ExecutionPolicy Bypass -File run_all.ps1 -Quick` for builds, differential checks and demos without replacing saved results.

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
