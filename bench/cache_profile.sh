#!/bin/bash
# cache_profile.sh  (owner: Analyst)
# Collects profiler output for cache claims. Needs Linux "perf" (sudo apt install linux-tools-generic).
# Output goes to results/perf_*.txt.  Run after "make all".
#
# The settings compare the same step count with a small and a large Fenwick tree, and
# Structure A at a small and a large window.
cd "$(dirname "$0")/.."
if ! command -v perf > /dev/null; then
    echo "perf is not installed, so no cache profile was made."
    echo "Without profiler output, REPORT.md must not make claims about cache behaviour."
    exit 1
fi
EVENTS=cycles,instructions,cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses
N=1000000
for setting in "B 100 10 100" "B 100 10 1000000" "A 100 10 1000000" "A 100000 10 1000000"; do
    set -- $setting
    name="perf_$1_w$2_R$4.txt"
    perf stat -e $EVENTS -o results/$name ./build/bench_time $1 $2 $3 $4 $N 50 1 2.0 > /dev/null
    echo "wrote results/$name"
done
