# Execution flow

1. experiments.py supplies N, five seeds and 31 settings.
2. predict.py writes results/predictions.csv from simple cost models.
3. run_bench.py compiles bench.cpp once, with no counters.
4. bench_time.exe constructs team A or adapted team B, then replays add/retract/median and prints timing, resident memory, checksum and U.
5. run_bench.py captures 155 rows in results/timing.csv.
6. analyse.py groups five seeds and writes means, sample SD, throughput, actual average U and A/B speedup into comparison.csv.
7. make_figures.py creates six graphs under results/figures/.
8. write_analysis.py refreshes results/README.md from these measurements.

No oracle is included in timed measurements. oracle/diff_test.cpp separately validates every query against a sorted reference.
