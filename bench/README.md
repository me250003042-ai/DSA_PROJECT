# Team median benchmarking

Run from the project root: `python bench/run_bench.py`.
Requires Windows, g++ C++17, Python and matplotlib. A small run (`python bench/run_bench.py 2000`) replaces saved outputs.

There are four experiment families: window (8 settings), retractions (12), A purge factor (6), B domain (5). Total: 31 settings, five seeds each, 155 timing runs, one million steps each. A step is add, optional ID retraction, median query. Values are double. There are no operation counters or counting executables.

The runner compiles bench.cpp once into build/bench_time.exe. It runs predict.py, executes the settings, writes results/timing.csv, runs analyse.py to write comparison.csv, make_figures.py to create six graphs, and write_analysis.py to refresh results/README.md. All output is at the project root.

A uses src/structA/teamA.h and its five supporting headers via a small medianA.h adapter. B uses src/structB/teamB.h from the latest team commit, corrected for safe ID retractions and arrival expiry, via a small medianB.h ID adapter. See TEAM_INTEGRATION.md for source commit and fixes.

Construction time includes compression for B and is separate from replay latency. Replay latency includes loop and checksum work. Memory is peak resident process working set minus pre-construction current working set, an approximate process metric. Error bars show sample SD over five seeds. Formulas are trend references, not exact nanosecond predictions.

The graphs cover window latency/memory, retraction latency/memory, A purge reference versus timing, and B log-domain reference versus timing. Graph 6 no longer contains measured operation counts. comparison.csv compares time/memory only.
