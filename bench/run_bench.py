import csv
import os
import subprocess
import sys
from experiments import N as DEFAULT_N, SEEDS, make_settings

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, "..")
res = os.path.join(root, "results")
build = os.path.join(root, "build")
N = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_N

def build_benchmarks():

    os.makedirs(build, exist_ok=True)
    flags = ["g++", "-std=c++17", "-O2"]
    subprocess.check_call(flags + [os.path.join(here, "bench.cpp"), "-o", os.path.join(build, "bench_time.exe"), "-lpsapi"])

def run_script(path, *args):

    subprocess.check_call([sys.executable, os.path.join(root, path), *map(str, args)])

def run_setting(binary, st, w, r, R, seed, f):

    command = [os.path.join(build, binary + ".exe"), st, str(w), str(r),
               str(R), str(N), str(seed), str(f)]
    output = subprocess.check_output(command, text=True)
    return output.strip().split(",")

def main():
    if N <= 0:
        raise ValueError("N must be positive")
    os.environ["PYTHONUTF8"] = "1"

    subprocess.check_call([sys.executable, "-c", "import matplotlib"])
    os.makedirs(res, exist_ok=True)

    run_script("bench/predict.py", N)
    build_benchmarks()
    settings = make_settings()
    timing_cols = ["experiment", "structure", "w", "retract_pct", "range", "N", "seed", "purge_factor",
                   "build_ms", "run_ms", "ns_per_step", "peak_kb", "checksum", "retractions_done", "distinct_values"]

    with open(os.path.join(res, "timing.csv"), "w", newline="") as timing_file:
        timing_writer = csv.writer(timing_file)
        timing_writer.writerow(timing_cols)
        for number, (name, st, w, r, R, f) in enumerate(settings, start=1):
            print(f"[{number}/{len(settings)}] {name} {st} w={w} retract={r}% range={R} factor={f}", flush=True)

            for seed in SEEDS:
                fields = run_setting("bench_time", st, w, r, R, seed, f)

                timing_writer.writerow([name] + fields)
                timing_file.flush()

    run_script("bench/analyse.py")
    run_script("bench/make_figures.py")
    run_script("bench/write_analysis.py")
    print("Done. CSV files: results/; graphs: results/figures/", flush=True)

if __name__ == "__main__":
    main()
