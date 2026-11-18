import csv
import math
import os
import sys
from experiments import N, make_settings


def model(structure, w, value_range, purge_factor):
    if structure == "A":
        theta = purge_factor - 1.0
        depth = math.log2(w + 1)
        purge_overhead = 1.0 + 1.0 / theta
        return theta, w, depth, purge_overhead, depth + purge_overhead, "O(N + w)"

    universe = min(N, value_range)
    depth = math.log2(universe + 1)
    step_cost = 3.0 * depth
    return "", universe, depth, "", step_cost, "O(N + U)"


def main():
    global N
    if len(sys.argv) > 1:
        N = int(sys.argv[1])

    results = os.path.join(os.path.dirname(__file__), "..", "results")
    os.makedirs(results, exist_ok=True)
    rows = []

    for name, structure, w, retract, value_range, purge_factor in make_settings():
        theta, scale, depth, purge, step, memory = model(structure, w, value_range, purge_factor)
        rows.append({
            "experiment": name,
            "structure": structure,
            "w": w,
            "retract_pct": retract,
            "range": value_range,
            "purge_factor": purge_factor,
            "theta": theta,
            "model_scale": scale,
            "log_depth": round(depth, 4),
            "purge_overhead": round(purge, 4) if purge != "" else "",
            "model_step_units": round(step, 4),
            "memory_order": memory,
        })

    path = os.path.join(results, "predictions.csv")
    with open(path, "w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    print("Wrote", path)


if __name__ == "__main__":
    main()
