# run_bench.py  (owner: Analyst)
# Builds the benchmark, runs every setting from experiments.py and writes:
#   results/timing.csv   (RUNS runs per setting: time, memory, checksum, seed)
#   results/counts.csv   (operation counters per step, one run per setting, seed 1)
# Usage: python3 bench/run_bench.py [N]     (N = readings per run, default from experiments.py)

import subprocess, sys, os, csv
sys.path.insert(0, os.path.dirname(__file__))
from experiments import N as DEFAULT_N, SEEDS, make_settings

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, "..")
res = os.path.join(root, "results")
N = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_N

def build():
    flags = ["g++", "-std=c++17", "-O2"]
    subprocess.check_call(flags + [os.path.join(here, "bench.cpp"), "-o", os.path.join(here, "bench_time")])
    subprocess.check_call(flags + ["-DCOUNT_OPS", os.path.join(here, "bench.cpp"), "-o", os.path.join(here, "bench_count")])

def run(binary, st, w, r, R, p, seed, f):
    out = subprocess.check_output([os.path.join(here, binary), st, str(w), str(r), str(R), str(N), str(p), str(seed), str(f)])
    return out.decode().strip().split(",")

def main():
    build()
    settings = make_settings()
    timing_cols = ["experiment", "structure", "w", "retract_pct", "range", "N", "percentile", "seed", "purge_factor",
                   "build_ms", "run_ms", "ns_per_step", "peak_kb", "checksum", "retractions_done"]
    count_cols = ["experiment", "structure", "w", "retract_pct", "range", "N", "percentile", "seed", "purge_factor",
                  "heap_compares", "heap_swaps", "hash_probes", "purge_moves", "fen_steps", "bin_steps"]
    tf = open(os.path.join(res, "timing.csv"), "w", newline="")
    cf = open(os.path.join(res, "counts.csv"), "w", newline="")
    tw = csv.writer(tf)
    cw = csv.writer(cf)
    tw.writerow(timing_cols)
    cw.writerow(count_cols)
    total = len(settings)
    for number, (name, st, w, r, R, p, f) in enumerate(settings):
        print("[%d/%d] %s %s w=%d retract=%d%% range=%d p=%d factor=%s" % (number + 1, total, name, st, w, r, R, p, f), flush=True)
        for seed in SEEDS:
            x = run("bench_time", st, w, r, R, p, seed, f)
            # x = struct,w,r,R,n,p,seed,factor,build_ms,run_ms,ns,peak,checksum,done
            tw.writerow([name, st, w, r, R, N, p, seed, f] + x[8:])
            tf.flush()
        x = run("bench_count", st, w, r, R, p, 1, f)
        c = x[14:20]
        wanted = {"A": [0, 1, 2, 3], "B": [4, 5]}
        row = ["", "", "", "", "", ""]
        keep = wanted[st]
        for i in range(6):
            row[i] = c[i] if i in keep else ""
        cw.writerow([name, st, w, r, R, N, p, 1, f] + row)
        cf.flush()
    tf.close()
    cf.close()
    print("done")

main()
