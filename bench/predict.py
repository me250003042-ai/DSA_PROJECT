# predict.py  (owner: Analyst)  -- the D6 cost model, written BEFORE measuring.
#
# For every setting in experiments.py it predicts, per step (one step = one add,
# maybe one retract, and one get):
#   * counted operations of Structure A and Structure B
#   * time per step (a range, from an assumed cost per counted operation)
#   * peak memory
# and writes results/predictions.csv.   Run:  python3 bench/predict.py
# The derivation of every formula is in docs/derivations/D6_analyst.md.

import math, csv, os, sys
sys.path.insert(0, os.path.dirname(__file__))
from experiments import N, make_settings

NS_LOW = 1.25     # assumed ns per counted operation, best case (data in cache)
NS_HIGH = 3.75    # assumed ns per counted operation, worst case (many cache misses)

def model_B(w, r, R, p):
    U = R * (1 - math.exp(-N / R))             # distinct values among N draws
    L = math.log2(U)
    K = math.floor(L) + 1                      # loop steps of one kth() call
    upd = L / 2 + 1                            # average loop steps of one update()
    if p == 50:
        if r == 0 and w % 2 == 0:
            d = 2.0                            # n is always even
        else:
            d = 1.5                            # n is even half of the time
    else:
        d = 1.0
    fen = 2 * upd + d * K                      # add-update + one removal + kth calls
    binary = L                                 # binary search value -> index
    mem = 8 * U + 4 * N + 4 * N + 1 * N + 4 * N + 4 * N   # values+tree, id->index, order, active, copy of input, sort scratch
    return {"fen_steps": fen, "bin_steps": binary, "ops": fen + binary, "mem_mb": mem / 1e6}

def model_A(w, r, R, p, factor):
    theta = factor - 1.0
    rr = r / 100.0
    pp = p / 100.0
    if rr > 0:
        n = (w / rr) * (1 - math.exp(-rr))     # average number of live readings
        s = 1 - math.exp(-rr)                  # successful retractions per step
    else:
        n = float(w)
        s = 0.0
    m = 2 * pp * (1 - pp) * (1 + s)            # balance moves per step (0.5 for the median, r = 0)
    h = n * (1 + theta / 2) * math.sqrt(pp * (1 - pp))   # average size of one heap
    D = math.log2(h)                           # depth of one heap
    f = 1 - math.log(1 + theta) / theta        # average dead fraction inside the heaps
    extra = m * f / (1 - f)                    # extra pops of dead items per step
    cmp_ = 2.6 + m * (2 * D + 2.6) + extra * 2 * D + 2.4 / theta
    swp = 1.6 + m * (D - 1 + 1.6) + extra * (D - 1) + 0.74 / theta
    alpha = 1.4                                # average items per hash bucket
    probes = (8 + 2 * m) * alpha + extra * 1.7
    purge = (1 + theta) / theta
    ops = cmp_ + swp + probes + purge
    # memory: 6 bytes per reading ever added (order, active, side) + heaps (8 bytes per item,
    # peak (1+theta)*live) + hash set buckets (about 56 bytes per bucket, about live*theta/2 buckets)
    peak_items = (1 + theta) * n
    mem = 6 * N + 8 * peak_items + 56 * (theta * n / 2)
    return {"heap_compares": cmp_, "heap_swaps": swp, "hash_probes": probes,
            "purge_moves": purge, "ops": ops, "mem_mb": mem / 1e6}

def main():
    here = os.path.dirname(__file__)
    out = os.path.join(here, "..", "results", "predictions.csv")
    rows = []
    for (name, st, w, r, R, p, factor) in make_settings():
        if st == "A":
            m = model_A(w, r, R, p, factor)
        else:
            m = model_B(w, r, R, p)
        row = {"experiment": name, "structure": st, "w": w, "retract_pct": r, "range": R,
               "percentile": p, "purge_factor": factor,
               "pred_ops_per_step": round(m["ops"], 2),
               "pred_ns_low": round(m["ops"] * NS_LOW, 1),
               "pred_ns_high": round(m["ops"] * NS_HIGH, 1),
               "pred_mem_mb": round(m["mem_mb"], 1)}
        for key in ["heap_compares", "heap_swaps", "hash_probes", "purge_moves", "fen_steps", "bin_steps"]:
            row["pred_" + key] = round(m[key], 2) if key in m else ""
        rows.append(row)
    cols = list(rows[0].keys())
    with open(out, "w", newline="") as fh:
        wr = csv.DictWriter(fh, fieldnames=cols)
        wr.writeheader()
        for row in rows:
            wr.writerow(row)
    print("wrote", out)
    for row in rows:
        print(row["experiment"], row["structure"], "w=%d r=%d R=%d p=%d f=%.2f" % (row["w"], row["retract_pct"], row["range"], row["percentile"], row["purge_factor"]),
              "ops=%.1f" % row["pred_ops_per_step"], "mem=%.1fMB" % row["pred_mem_mb"])

if __name__ == "__main__":
    main()
