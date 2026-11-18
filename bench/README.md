# Windows median benchmark

From the project root run:

```powershell
python bench/run_bench.py
```

Requires MinGW g++ (C++17), Python 3 and matplotlib (`python -m pip install matplotlib`). A quick pipeline check is `python bench/run_bench.py 2000`; it replaces the saved results with small-run results.

The full run uses 1,000,000 steps, five timing seeds and 31 settings: eight window settings, twelve retraction settings, six A purge settings and five B domain settings. Each step adds a double reading, optionally retracts a reading by ID and requests the median. Even-sized sets return the average of the middle two values.

| File | Responsibility |
|---|---|
| run_bench.py | Entry point: compile, execute, save measurements, analyse and plot |
| experiments.py | Stream size, seeds and shared settings |
| bench.cpp | Calls the actual C++ structures and measures Windows time/memory |
| predict.py | Writes mathematical trend predictions |
| analyse.py | Groups five runs and calculates statistics and speedup |
| make_figures.py | Writes six PNG graphs |
| FORMULAS.md | Derivations and interpretation of the comparison formulas |
| DATAFLOW.md | Detailed execution flow |

Execution order: `run_bench.py -> predict.py -> compile bench.cpp twice -> timing/counting runs -> analyse.py -> make_figures.py`.

The timing executable has counters disabled. The counting executable enables COUNT_OPS. Separate runs avoid including counter overhead in timing. Each executable calls StructureA or StructureB directly; the oracle is not part of benchmark measurements.

Outputs are in the project-root `results/`: predictions.csv (31 rows), timing.csv (155 rows), counts.csv (31 rows), comparison.csv (31 rows), and figures/ (six PNGs). Existing output files are replaced. Executables are generated under build/.

The figures show latency/window, memory/window, latency/retraction, memory/retraction, A purge threshold and B logarithmic domain cost. Error bars are sample standard deviations over five seeds. Construction/compression is measured separately from replay latency. The memory metric is the process peak working set minus the pre-construction current working set; it is not an exact allocation count or isolated structure size.

Checksums help compare matching A/B workloads, but equal sums alone do not prove every answer correct. Use the separate oracle differential tests for correctness. Model units describe growth; they are not predicted nanoseconds or exact instruction counts.
