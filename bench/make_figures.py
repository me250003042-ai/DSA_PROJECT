import csv, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

here = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "results")
outdir = os.path.join(here, "figures")
os.makedirs(outdir, exist_ok=True)

rows = list(csv.DictReader(open(os.path.join(here, "comparison.csv"))))
with open(os.path.join(here, "timing.csv")) as timing_file:
    stream_size = int(next(csv.DictReader(timing_file))["N"])
for r in rows:
    for k in r:
        try:
            r[k] = float(r[k])
        except ValueError:
            pass

def pick(exp, st):

    return [r for r in rows if r["experiment"] == exp and r["structure"] == st]

COL = {"A": "tab:blue", "B": "tab:red"}
NAME = {"A": "A: two heaps + lazy deletion", "B": "B: Fenwick tree"}

def time_panel(ax, exp, xkey, xlabel, logx=True):

    for st in ["A", "B"]:
        d = pick(exp, st)
        x = [r[xkey] for r in d]
        y = [r["meas_ns_mean"] for r in d]
        e = [r["meas_ns_sd"] for r in d]
        lo = [r["pred_ns_low"] for r in d]
        hi = [r["pred_ns_high"] for r in d]
        ax.fill_between(x, lo, hi, color=COL[st], alpha=0.12, label="predicted band (%s)" % st)
        ax.errorbar(x, y, yerr=e, color=COL[st], marker="o", capsize=3, label="measured " + NAME[st])
    if logx:
        ax.set_xscale("log")
    ax.set_xlabel(xlabel)
    ax.set_ylabel("time per step (ns)")
    ax.set_ylim(bottom=0)
    ax.grid(alpha=0.3)

def ops_panel(ax, exp, xkey, xlabel, logx=True):

    for st in ["A", "B"]:
        d = pick(exp, st)
        x = [r[xkey] for r in d]
        ax.plot(x, [r["pred_ops"] for r in d], color=COL[st], linestyle="--", marker="s", label="predicted " + st)
        ax.plot(x, [r["meas_ops"] for r in d], color=COL[st], marker="o", label="measured " + st)
    if logx:
        ax.set_xscale("log")
    ax.set_xlabel(xlabel)
    ax.set_ylabel("counted operations per step")
    ax.set_ylim(bottom=0)
    ax.grid(alpha=0.3)

def two_panel(exp, xkey, xlabel, title, fname, logx=True):

    fig, axs = plt.subplots(1, 2, figsize=(12, 4.2))
    time_panel(axs[0], exp, xkey, xlabel, logx)
    ops_panel(axs[1], exp, xkey, xlabel, logx)
    axs[0].legend(fontsize=7)
    axs[1].legend(fontsize=8)
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(os.path.join(outdir, fname), dpi=140)
    plt.close(fig)

two_panel("exp1_window", "w", "window size w", "Experiment 1: window size (10%% retractions, N = %d)" % stream_size, "fig1_window.png")
two_panel("exp2_retract", "retract_pct", "retraction chance per step (%)", "Experiment 2: retraction rate (w = 10^4)", "fig2_retract.png", logx=False)
two_panel("exp3_domain", "range", "value range R (size of the compressed domain U is about R)", "Experiment 3: compressed domain (w = 100)", "fig3_domain.png")

fig, axs = plt.subplots(1, 2, figsize=(12, 4.2))
for st in ["A", "B"]:
    d = pick("exp1_window", st)
    x = [r["w"] for r in d]
    axs[0].plot(x, [r["pred_mem_mb"] for r in d], color=COL[st], linestyle="--", marker="s", label="predicted " + st)
    axs[0].errorbar(x, [r["meas_mem_mb"] for r in d], yerr=[r["meas_mem_sd"] for r in d], color=COL[st], marker="o", capsize=3, label="measured " + st)
    d = pick("exp3_domain", st)
    x = [r["range"] for r in d]
    axs[1].plot(x, [r["pred_mem_mb"] for r in d], color=COL[st], linestyle="--", marker="s", label="predicted " + st)
    axs[1].errorbar(x, [r["meas_mem_mb"] for r in d], yerr=[r["meas_mem_sd"] for r in d], color=COL[st], marker="o", capsize=3, label="measured " + st)
axs[0].set_xscale("log"); axs[1].set_xscale("log")
axs[0].set_xlabel("window size w"); axs[1].set_xlabel("value range R (w = 100)")
for ax in axs:
    ax.set_ylabel("peak extra memory (MB)")
    ax.set_ylim(bottom=0)
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8)
fig.suptitle("Peak memory above the input (N = %d)" % stream_size)
fig.tight_layout()
fig.savefig(os.path.join(outdir, "fig4_memory.png"), dpi=140)
plt.close(fig)

fig, axs = plt.subplots(1, 2, figsize=(12, 4.2))
d = pick("exp4_purge", "A")
x = [r["purge_factor"] for r in d]
axs[0].errorbar(x, [r["meas_ns_mean"] for r in d], yerr=[r["meas_ns_sd"] for r in d], color="tab:blue", marker="o", capsize=3, label="measured (main run)")
axs[0].set_xlabel("purge factor (purge when heap items > factor x live)")
axs[0].set_ylabel("time per step (ns)")
axs[0].set_ylim(bottom=0)
axs[0].legend(fontsize=8)
axs[0].grid(alpha=0.3)
axs[1].plot(x, [r["pred_ops"] for r in d], color="tab:blue", linestyle="--", marker="s", label="predicted")
axs[1].plot(x, [r["meas_ops"] for r in d], color="tab:blue", marker="o", label="measured")
axs[1].set_xlabel("purge factor")
axs[1].set_ylabel("counted operations per step")
axs[1].set_ylim(bottom=0)
axs[1].legend()
axs[1].grid(alpha=0.3)
fig.suptitle("Experiment 4: purge threshold of Structure A (w = 10^4, 10% retractions)")
fig.tight_layout()
fig.savefig(os.path.join(outdir, "fig5_purge.png"), dpi=140)
plt.close(fig)

fig, ax = plt.subplots(figsize=(5.5, 5))
for st in ["A", "B"]:
    d = [r for r in rows if r["structure"] == st]
    ax.scatter([r["pred_ops"] for r in d], [r["meas_ops"] for r in d], color=COL[st], label=NAME[st], s=22)
ax.plot([0, 90], [0, 90], color="gray", linestyle=":")
ax.set_xlabel("predicted counted operations per step")
ax.set_ylabel("measured counted operations per step")
ax.legend(fontsize=8)
ax.grid(alpha=0.3)
ax.set_title("Prediction against measurement (all settings)")
fig.tight_layout()
fig.savefig(os.path.join(outdir, "fig6_pred_vs_meas.png"), dpi=140)
plt.close(fig)
two_panel("exp5_percentile", "percentile", "percentile", "Experiment 5: requested percentile", "fig7_percentile.png", logx=False)
print("figures written to", outdir)
