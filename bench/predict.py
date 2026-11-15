import math, csv, os, sys
sys.path.insert(0, os.path.dirname(__file__))
from experiments import N, make_settings

NS_LOW = 1.25
NS_HIGH = 3.75

def model_B(w, r, R, p):

    U = R * (1 - math.exp(-N / R))
    L = math.log2(U)
    K = math.floor(L) + 1
    upd = L / 2 + 1
    if p == 50:
        if r == 0 and w % 2 == 0:
            d = 2.0
        else:
            d = 1.5
    else:
        d = 1.0
    fen = 2 * upd + d * K
    binary = L
    mem = 8 * U + 4 * N + 4 * N + 1 * N + 4 * N + 4 * N
    return {"fen_steps": fen, "bin_steps": binary, "ops": fen + binary, "mem_mb": mem / 1e6}

def model_A(w, r, R, p, factor):

    theta = factor - 1.0
    rr = r / 100.0
    pp = p / 100.0
    if rr > 0:
        n = (w / rr) * (1 - math.exp(-rr))
        s = 1 - math.exp(-rr)
    else:
        n = float(w)
        s = 0.0
    m = 2 * pp * (1 - pp) * (1 + s)
    h = n * (1 + theta / 2) * math.sqrt(pp * (1 - pp))
    D = math.log2(h)
    f = 1 - math.log(1 + theta) / theta
    extra = m * f / (1 - f)
    cmp_ = 2.6 + m * (2 * D + 2.6) + extra * 2 * D + 2.4 / theta
    swp = 1.6 + m * (D - 1 + 1.6) + extra * (D - 1) + 0.74 / theta
    alpha = 1.4
    probes = (8 + 2 * m) * alpha + extra * 1.7
    purge = (1 + theta) / theta
    ops = cmp_ + swp + probes + purge

    peak_items = (1 + theta) * n
    mem = 6 * N + 8 * peak_items + 56 * (theta * n / 2)
    return {"heap_compares": cmp_, "heap_swaps": swp, "hash_probes": probes,
            "purge_moves": purge, "ops": ops, "mem_mb": mem / 1e6}

def main():

    global N
    if len(sys.argv) > 1:
        N = int(sys.argv[1])
    here = os.path.dirname(__file__)
    os.makedirs(os.path.join(here, "..", "results"), exist_ok=True)
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
