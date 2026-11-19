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
predictions = read("predictions.csv")

paired_runs = {}
for row in timing:
    key = (row['experiment'], row['w'], row['retract_pct'], row['range'], row['seed'], row['purge_factor'])
    paired_runs.setdefault(key, {})[row['structure']] = row
for key, pair in paired_runs.items():
    if 'A' in pair and 'B' in pair:
        for field in ['checksum', 'retractions_done']:
            if pair['A'][field] != pair['B'][field]:
                raise ValueError(f'A/B {field} mismatch for {key}')
print('Matching A/B checksums and successful retractions verified')

groups = {}
for row in timing:
    groups.setdefault(setting(row), []).append(row)

rows = []

for prediction in predictions:
    key = setting(prediction)
    runs = groups[key]
    ns = [float(row["ns_per_step"]) for row in runs]
    memory = [float(row["peak_kb"]) * 1024 / 1e6 for row in runs]
    mean_ns = mean(ns)

    universe = round(mean(int(row["distinct_values"]) for row in runs)) if key[1] == "B" else key[2]
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
