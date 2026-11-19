import csv
from pathlib import Path

root = Path(__file__).resolve().parent.parent
results = root / 'results'
with (results / 'comparison.csv').open() as file:
    rows = list(csv.DictReader(file))

lines = ['# Team implementation benchmark results', '',
         'One million steps per run, five timing seeds, 31 settings across four experiment families. Each step adds a double, optionally retracts an ID, and requests the median. Operation counting has been removed.', '',
         'Source: GitHub main commit 54ed2b5. A uses the team RunningMedian implementation with integration fixes. B uses the latest team value/active-pair storage, hash compression and two-search median implementation with ID and expiry corrections. See ../bench/TEAM_INTEGRATION.md. These are corrected integration copies, not unmodified upstream files.', '',
         'Latency excludes input generation and construction. Memory is approximate extra peak process working set, not exact allocations. Error bars are sample standard deviations over five seeds.', '']
graphs = [('window', 'Window size', ['fig1_latency_window.png', 'fig2_memory_window.png']),
          ('retraction', 'Retraction probability', ['fig3_latency_retraction.png', 'fig4_memory_retraction.png']),
          ('purge', 'A purge threshold', ['fig5_purge_threshold.png']),
          ('domain', 'B compressed universe', ['fig6_fenwick_logU.png'])]
for experiment, title, files in graphs:
    selected = [r for r in rows if r['experiment'] == experiment]
    lines += ['## ' + title, '']
    lines += [f'![{title}](figures/{file})' for file in files]
    lines += ['', '| Structure | Setting | Mean ns/step | SD ns | Memory MB |', '|---|---:|---:|---:|---:|']
    key = {'window': 'w', 'retraction': 'retract_pct', 'purge': 'purge_factor', 'domain': 'model_scale'}[experiment]
    lines += [f"| {r['structure']} | {r[key]} | {r['mean_ns_per_step']} | {r['sd_ns_per_step']} | {r['mean_memory_mb']} |" for r in selected]
    if experiment == 'window':
        lines += ['', 'Window graphs vary w while holding the generated domain and retraction probability fixed. A heap occupancy depends on w. B domain storage depends on U. Timing can also reflect cache, allocation and machine-load effects; a finite sweep does not prove an asymptotic bound.']
    elif experiment == 'retraction':
        lines += ['', 'The x-axis is retraction request probability, not successful deletion fraction. A successful retraction may replace a later expiry removal. Do not assume runtime must grow linearly with request probability.']
    elif experiment == 'purge':
        best = min(selected, key=lambda r: float(r['mean_ns_per_step']))
        lines += ['', f"Lowest mean latency in this sweep: purge factor {best['purge_factor']} (alpha={best['theta']}), {best['mean_ns_per_step']} ns/step. Larger alpha permits more dead entries and lowers the rebuild-overhead reference 1+1/alpha. Actual latency also includes hash-table maintenance, including rehashing, so the reference is not a complete runtime model."]
    else:
        lines += ['', 'The left panel shows a dimensionless logarithmic reference, not counted operations. The right panel shows measured latency against log2(U+1). U is averaged across timing seeds. Larger U changes both tree traversal depth and hash/domain memory.']
    lines += ['']
pairs = [r for r in rows if r['structure'] == 'A' and r['speedup_A_over_B']]
wins = sum(float(r['speedup_A_over_B']) > 1 for r in pairs)
lines += ['## Conclusions', '', f'A has lower mean latency in {wins} of {len(pairs)} paired settings. This is a comparison of means, not a statistical significance test.', '',
          'Choose based on the actual workload and domain size. Inspect the memory and timing tables together. The purge setting trades retained heap entries against rebuilding work. B needs a known finite domain. These results replace the earlier local-implementation measurements.', '',
          'Raw data: timing.csv. Aggregated measurements: comparison.csv. Mathematical references: predictions.csv. Definitions and derivations: ../bench/FORMULAS.md.']
(results / 'README.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('Wrote results/README.md')
