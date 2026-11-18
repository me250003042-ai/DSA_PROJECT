import csv
import os
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


results = os.path.join(os.path.dirname(__file__), "..", "results")
figures = os.path.join(results, "figures")
os.makedirs(figures, exist_ok=True)

for name in os.listdir(figures):
    if name.endswith(".png"):
        os.remove(os.path.join(figures, name))

with open(os.path.join(results, "comparison.csv")) as file:
    rows = list(csv.DictReader(file))

for row in rows:
    for key, value in list(row.items()):
        try:
            row[key] = float(value)
        except (ValueError, TypeError):
            pass

colors = {"A": "tab:blue", "B": "tab:red"}
labels = {"A": "A: two heaps", "B": "B: Fenwick tree"}


def select(experiment, structure=None):
    return [
        row for row in rows
        if row["experiment"] == experiment
        and (structure is None or row["structure"] == structure)
    ]


def measured_graph(experiment, x_name, y_name, error_name, xlabel, ylabel, title, filename, log_x=False):
    figure, axis = plt.subplots(figsize=(7, 4.5))
    for structure in ["A", "B"]:
        data = select(experiment, structure)
        axis.errorbar(
            [row[x_name] for row in data],
            [row[y_name] for row in data],
            yerr=[row[error_name] for row in data],
            color=colors[structure], marker="o", capsize=4,
            label=labels[structure],
        )
    if log_x:
        axis.set_xscale("log")
    axis.set_xlabel(xlabel)
    axis.set_ylabel(ylabel)
    axis.set_ylim(bottom=0)
    axis.grid(alpha=0.3)
    axis.legend()
    axis.set_title(title)
    figure.tight_layout()
    figure.savefig(os.path.join(figures, filename), dpi=150)
    plt.close(figure)


measured_graph(
    "window", "w", "mean_ns_per_step", "sd_ns_per_step",
    "window size w", "latency (ns per step)",
    "Latency as window size grows", "fig1_latency_window.png", True,
)

measured_graph(
    "window", "w", "mean_memory_mb", "sd_memory_mb",
    "window size w", "extra peak resident memory (MB)",
    "Memory as window size grows", "fig2_memory_window.png", True,
)

measured_graph(
    "retraction", "retract_pct", "mean_ns_per_step", "sd_ns_per_step",
    "retraction chance (%)", "latency (ns per step)",
    "Latency as retraction rate changes", "fig3_latency_retraction.png",
)

measured_graph(
    "retraction", "retract_pct", "mean_memory_mb", "sd_memory_mb",
    "retraction chance (%)", "extra peak resident memory (MB)",
    "Memory as retraction rate changes", "fig4_memory_retraction.png",
)

purge = select("purge", "A")
figure, axes = plt.subplots(1, 2, figsize=(11, 4.3))
axes[0].plot(
    [row["theta"] for row in purge],
    [row["purge_overhead"] for row in purge],
    color=colors["A"], marker="o",
)
axes[0].set_xlabel("purge threshold theta")
axes[0].set_ylabel("model overhead 1 + 1/theta")
axes[0].set_title("Amortized purge model")
axes[1].errorbar(
    [row["theta"] for row in purge],
    [row["mean_ns_per_step"] for row in purge],
    yerr=[row["sd_ns_per_step"] for row in purge],
    color=colors["A"], marker="o", capsize=4,
)
axes[1].set_xlabel("purge threshold theta")
axes[1].set_ylabel("measured latency (ns per step)")
axes[1].set_title("Measured Structure A cost")
for axis in axes:
    axis.set_ylim(bottom=0)
    axis.grid(alpha=0.3)
figure.suptitle("Structure A: effect of purge threshold")
figure.tight_layout()
figure.savefig(os.path.join(figures, "fig5_purge_threshold.png"), dpi=150)
plt.close(figure)

domain = select("domain", "B")
figure, axes = plt.subplots(1, 2, figsize=(11, 4.3))
depth = [row["log_depth"] for row in domain]
axes[0].plot(depth, [row["model_step_units"] for row in domain], "--s", label="3 log2(U+1) model")
axes[0].plot(depth, [row["measured_operations_per_step"] for row in domain], "-o", label="measured counted operations")
axes[0].set_xlabel("log2(U+1)")
axes[0].set_ylabel("cost per step")
axes[0].set_title("Logarithmic operation model")
axes[0].legend()
axes[1].errorbar(
    depth,
    [row["mean_ns_per_step"] for row in domain],
    yerr=[row["sd_ns_per_step"] for row in domain],
    color=colors["B"], marker="o", capsize=4,
)
axes[1].set_xlabel("log2(U+1)")
axes[1].set_ylabel("measured latency (ns per step)")
axes[1].set_title("Measured Structure B cost")
for axis in axes:
    axis.set_ylim(bottom=0)
    axis.grid(alpha=0.3)
figure.suptitle("Structure B: cost as compressed domain grows")
figure.tight_layout()
figure.savefig(os.path.join(figures, "fig6_fenwick_logU.png"), dpi=150)
plt.close(figure)

print("Wrote six graphs to", figures)
