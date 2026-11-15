import csv
import os
from statistics import mean, stdev

here = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "results")

def read(name):
    with open(os.path.join(here, name)) as f:
        return list(csv.DictReader(f))

def key(r):

    return (r["experiment"], r["structure"], int(r["w"]), int(r["retract_pct"]), int(r["range"]),
            int(r["percentile"]), float(r["purge_factor"]))

timing = read("timing.csv")
counts = read("counts.csv")
preds = read("predictions.csv")

sums = {}
for r in timing:
    k = (r["experiment"], int(r["w"]), int(r["retract_pct"]), int(r["range"]), int(r["percentile"]), r["seed"], float(r["purge_factor"]))
    sums.setdefault((k[0], k[1], k[2], k[3], k[4], k[5]), {}).setdefault(r["structure"], set()).add(r["checksum"])
bad = 0
total = 0
for k, d in sums.items():
    if "A" in d and "B" in d:
        total += 1
        if d["A"] != d["B"]:
            bad += 1
print("checksum agreement A vs B: %d of %d settings/seeds agree" % (total - bad, total))
if bad:
    raise SystemExit("Benchmark checksums differ; investigate before generating graphs.")

groups = {}
for r in timing:
    groups.setdefault(key(r), []).append(r)
cnt = {key(r): r for r in counts}
pred = {key(r): r for r in preds}

rows = []
for k in pred:

    g = groups[k]
    ns = [float(r["ns_per_step"]) for r in g]
    mem = [float(r["peak_kb"]) * 1024 / 1e6 for r in g]
    c = cnt[k]

    counter_names = ["heap_compares", "heap_swaps", "hash_probes", "purge_moves", "fen_steps", "bin_steps"]
    ops = sum(float(c[name] or 0) for name in counter_names)
    p = pred[k]
    rows.append({
        "experiment": k[0], "structure": k[1], "w": k[2], "retract_pct": k[3], "range": k[4], "percentile": k[5], "purge_factor": k[6],
        "pred_ops": float(p["pred_ops_per_step"]), "meas_ops": round(ops, 2), "ops_ratio": round(ops / float(p["pred_ops_per_step"]), 2),
        "pred_ns_low": float(p["pred_ns_low"]), "pred_ns_high": float(p["pred_ns_high"]),
        "meas_ns_mean": round(mean(ns), 1), "meas_ns_sd": round(stdev(ns), 1),
        "pred_mem_mb": float(p["pred_mem_mb"]), "meas_mem_mb": round(mean(mem), 1), "meas_mem_sd": round(stdev(mem), 1),
        "meas_heap_compares": c["heap_compares"], "meas_heap_swaps": c["heap_swaps"], "meas_hash_probes": c["hash_probes"],
        "meas_purge_moves": c["purge_moves"], "meas_fen_steps": c["fen_steps"], "meas_bin_steps": c["bin_steps"],
        "pred_heap_compares": p["pred_heap_compares"], "pred_heap_swaps": p["pred_heap_swaps"], "pred_hash_probes": p["pred_hash_probes"],
        "pred_purge_moves": p["pred_purge_moves"], "pred_fen_steps": p["pred_fen_steps"], "pred_bin_steps": p["pred_bin_steps"],
    })

with open(os.path.join(here, "comparison.csv"), "w", newline="") as f:
    wr = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
    wr.writeheader()
    for r in rows:
        wr.writerow(r)

print("Wrote results/comparison.csv")
