N = 1000000
SEEDS = [1, 2, 3, 4, 5]


def make_settings():
    settings = []

    for w in [100, 1000, 10000, 100000]:
        for structure in ["A", "B"]:
            settings.append(("window", structure, w, 10, 1000000, 2.0))

    for retract in [0, 10, 30, 50, 70, 90]:
        for structure in ["A", "B"]:
            settings.append(("retraction", structure, 10000, retract, 1000000, 2.0))

    for purge_factor in [1.25, 1.5, 2.0, 3.0, 4.0, 6.0]:
        settings.append(("purge", "A", 10000, 10, 1000000, purge_factor))

    for value_range in [100, 1000, 10000, 100000, 1000000]:
        settings.append(("domain", "B", 100, 10, value_range, 2.0))

    return settings
