N = 1000000
SEEDS = [1, 2, 3, 4, 5]

def make_settings():
    s = []

    for w in [100, 1000, 10000, 100000]:
        for st in ["A", "B"]:
            s.append(("exp1_window", st, w, 10, 1000000, 50, 2.0))

    for r in [0, 10, 30, 50, 70, 90]:
        for st in ["A", "B"]:
            s.append(("exp2_retract", st, 10000, r, 1000000, 50, 2.0))

    for R in [100, 1000, 10000, 100000, 1000000]:
        for st in ["A", "B"]:
            s.append(("exp3_domain", st, 100, 10, R, 50, 2.0))

    for f in [1.25, 1.5, 2.0, 3.0, 4.0, 6.0]:
        s.append(("exp4_purge", "A", 10000, 10, 1000000, 50, f))

    for p in [50, 90, 99]:
        for st in ["A", "B"]:
            s.append(("exp5_percentile", st, 10000, 10, 1000000, p, 2.0))
    return s
