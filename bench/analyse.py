import csv
import math
import os
from statistics import mean, stdev


results = os.path.join(os.path.dirname(__file__), "..", "results")


def read(name):
    with open(os.path.join(results, name)) as file:
        return list(csv.DictReader(file))


def setting(row):
    return (
        row["experiment"], row["structure"], int(row["w"]),
        int(row["retract_pct"]), int(row["range"]),
        float(row["purge_factor"]),
    )


timing = read("timing.csv")
counts = read("counts.csv")
predictions = read("predictions.csv")

groups = {}
for row in timing:
    groups.setdefault(setting(row), []).append(row)

count_by_setting = {setting(row): row for row in counts}
rows = []

for prediction in predictions:
    key = setting(prediction)
    runs = groups[key]
    ns = [float(row["ns_per_step"]) for row in runs]
    memory = [float(row["peak_kb"]) * 1024 / 1e6 for row in runs]
    count = count_by_setting[key]
    names = ["heap_compares", "heap_swaps", "hash_probes", "purge_moves", "fen_steps", "bin_steps"]
    measured_operations = sum(float(count[name] or 0) for name in names)
    mean_ns = mean(ns)

    universe = int(count["distinct_values"]) if key[1] == "B" else key[2]
    depth = math.log2(universe + 1)
    model_units = 3.0 * depth if key[1] == "B" else float(prediction["model_step_units"])

    rows.append({
        "experiment": key[0],
        "structure": key[1],
        "w": key[2],
        "retract_pct": key[3],
        "range": key[4],
        "purge_factor": key[5],
        "theta": prediction["theta"],
        "model_scale": universe,
        "log_depth": round(depth, 4),
        "purge_overhead": prediction["purge_overhead"],
        "model_step_units": round(model_units, 4),
        "memory_order": prediction["memory_order"],
        "mean_ns_per_step": round(mean_ns, 2),
        "sd_ns_per_step": round(stdev(ns), 2),
        "throughput_steps_per_sec": round(1e9 / mean_ns, 2),
        "mean_memory_mb": round(mean(memory), 3),
        "sd_memory_mb": round(stdev(memory), 3),
        "measured_operations_per_step": round(measured_operations, 3),
        "measured_purge_moves_per_step": count["purge_moves"],
        "speedup_A_over_B": "",
    })

pairs = {}
for row in rows:
    key = (
        row["experiment"], row["w"], row["retract_pct"], row["range"],
        row["purge_factor"],
    )
    pairs.setdefault(key, {})[row["structure"]] = row

for pair in pairs.values():
    if "A" in pair and "B" in pair:
        speedup = pair["B"]["mean_ns_per_step"] / pair["A"]["mean_ns_per_step"]
        pair["A"]["speedup_A_over_B"] = round(speedup, 3)
        pair["B"]["speedup_A_over_B"] = round(speedup, 3)

path = os.path.join(results, "comparison.csv")
with open(path, "w", newline="") as file:
    writer = csv.DictWriter(file, fieldnames=rows[0].keys())
    writer.writeheader()
    writer.writerows(rows)

print("Wrote", path)
