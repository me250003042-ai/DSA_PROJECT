---
title: "D6: Cost models for the running median (Structure A and Structure B)"
author: "Analyst: Om"
---

**Status.** This document and `results/predictions.csv` were written *before* any benchmark was run.
The file `results/predictions_frozen.txt` stores the time and the SHA-256 of `predictions.csv` at that moment.
The comparison with measurement is in `REPORT.md`, not here, so that this text is never edited to fit the data.

# 0. What is predicted

One **step** of the benchmark is: add one reading, with chance $r$ retract one earlier reading, then ask for the percentile once.
The program counts "operations" (the counters are in `src/common/counters.h`):

| Counter | Structure | Counts |
|---|---|---|
| `heap_compares` | A | calls of `better(x, y)`, one comparison of two heap items |
| `heap_swaps` | A | swaps of two heap items |
| `hash_probes` | A | items looked at inside a hash bucket |
| `purge_moves` | A | items looked at while purging dead items |
| `fen_steps` | B | loop turns in Fenwick `update` and in `kth` |
| `bin_steps` | B | loop turns in the binary search from value to compressed index |

Symbols: $N=10^6$ readings in the stream, window $w$, retraction chance $r$ per step, readings uniform in $[0,R)$,
percentile $p$ (written as a fraction in the formulas, $p=\tfrac12$ for the median), $U$ = number of distinct values, $\theta$ = purge threshold
(A purges when the heaps hold more than $(1+\theta)\cdot$ live items; the default is $\theta=1$, "dead items outnumber live items").

# 1. Structure B (Fenwick tree)

**Number of distinct values.** $N$ draws from $R$ values leave on average $U = R\,(1-(1-1/R)^N)\approx R\,(1-e^{-N/R})$ distinct values.
For $R=10^6$ this is $U\approx 632{,}000$. Write $L=\log_2 U$.

**`kth` costs exactly $K=\lfloor L\rfloor+1$ loop turns.** The loop starts with `step` equal to the largest power of two not above $U$,
and halves `step` every turn until it is 0. The values $2^{K-1},\dots,2,1$ give exactly $K$ turns, whatever the data are.
*Why it is correct* is D2 of the Structure B owner.

**`update` costs about $L/2+1$ loop turns on average.** `i = i + (i & -i)` adds the lowest set bit of $i$.
Each turn the lowest set bit moves up by at least one position. Let $j$ be the position of the lowest set bit of a random $i$
($\Pr[j]=2^{-(j+1)}$, so $E[j]\approx1$). The bits above $j$ are random; the loop needs one turn for each of them that is 0 (a carry), plus one turn for the first update
and one for the final jump past $U$. That gives $\tfrac12(L-1-E[j])+2\approx L/2+1$.

**Removals.** Every reading is removed exactly once, either by expiry or by a successful retraction. In the steady state one reading is added per step,
so exactly one removal happens per step, and a failed retraction (the id is already dead) costs nothing in the tree.
So the number of `update` calls per step is 2 (one for the add, one for the removal), *independent of $r$*.

**`get` makes $d$ calls of `kth`.** For $p\ne 50\%$: $d=1$. For the median, an even number $n$ of live readings needs two calls ($k$-th and $(k+1)$-th).
With $r=0$ and even $w$ the live count is always even, so $d=2$. With retractions the parity of $n$ is random, so $d=1.5$.

**Binary search.** Finding the compressed index of the new value takes about $L$ loop turns.

$$C_B \;=\; \underbrace{2\left(\tfrac L2+1\right)}_{\text{two updates}} \;+\; \underbrace{d\,K}_{\text{kth calls}} \;+\; \underbrace{L}_{\text{binary search}}
\;\approx\; (2+d)\,L + 2 + d .$$

So B's cost is **a function of $\log_2 U$ only**: for the median with retractions ($d=1.5$) it is about $3.5\log_2U+3.5$ operations. It does **not** depend on $w$ or on $r$.
For $R=10^6$: $C_B=70.5$ (with $r=0$: $80.5$, because $d=2$).

# 2. Structure A (two heaps with lazy deletion)

**Live count.** A reading dies at age $w$ (expiry) or earlier (retraction). A given live reading is hit by a retraction with chance $r/w$ per step,
so a reading is retracted before it expires with probability $s\approx 1-e^{-r}$, and the mean number of live readings is
$n=\dfrac{w}{r}\,(1-e^{-r})$ (and $n=w$ when $r=0$). $s$ is also the number of successful retractions per step.

**Purge, amortised (this is the model as a function of $\theta$).** Each step adds one item and kills one reading. The heaps grow by one dead item per step.
A purge happens when dead items exceed $\theta n$, so a purge happens every $\theta n$ steps. One purge scans $(1+\theta)n$ items and rebuilds heaps of $n$ items
by Floyd's method. I estimate the build at $2.4n$ compares and $0.74n$ swaps (the $0.74n$ is the known average number of exchanges for building a heap from random data; the $2.4n$ is my count for two comparisons per level
in this code). Per step:

$$\text{purge moves}=\frac{1+\theta}{\theta},\qquad \text{purge compares}=\frac{2.4}{\theta},\qquad \text{purge swaps}=\frac{0.74}{\theta}.$$

This is $O(1)$ amortised per step for every fixed $\theta>0$, and it **does not depend on $w$**.

**Balance moves.** After a step, the new reading went to the low heap with probability $p$ and the dead reading was in the low heap with probability $p$ (independently,
because the window removes the oldest reading, whose value is independent of its age). So the low heap changes by $-1,0,+1$ and the number of moves needed is
$E|\Delta|=2p(1-p)$, which is $\tfrac12$ for the median. A successful retraction triggers another balance, so I use $m=2p(1-p)(1+s)$ moves per step.
Each move is one pop from one heap and one push into the other.

**Push and pop costs.** Pushing a value into a heap of random order costs a constant: about $2.6$ compares and $1.6$ swaps on average (the item rises about 1.6 levels).
A pop moves the last item to the root and sifts it down almost the full depth $D=\log_2 h$, where $h$ is the heap size: $2D$ compares and $D-1$ swaps.
The average heap size is $h=n\,(1+\theta/2)\sqrt{p(1-p)}$ (the total size grows evenly from $n$ to $(1+\theta)n$, shared between the two heaps).

**Dead items at the top.** After a pop the new top is dead with probability $\bar f$, the average dead fraction
$\bar f = 1-\ln(1+\theta)/\theta$. A dead top is popped by `prune`, and the next one may be dead again, so the extra pops per top-removal are $\bar f/(1-\bar f)$:
$\text{extra}=m\,\bar f/(1-\bar f)$ per step.

**Hash set.** `contains` is called about $8+2m$ times per step (add: 1; balance: 4; get: 2; marking the death: 1; one more per move), and each call looks at
$\alpha\approx1.4$ items on average (the table doubles when it holds more than 2 items per bucket, so the load is between 1 and 2). I add $1.7$ probes for each extra pop (erase).

**Total.**

$$\begin{aligned}
\text{compares} &= 2.6 + m\,(2D+2.6) + \text{extra}\cdot 2D + 2.4/\theta\\
\text{swaps}    &= 1.6 + m\,(D-1+1.6) + \text{extra}\cdot(D-1) + 0.74/\theta\\
\text{probes}   &= (8+2m)\cdot1.4 + 1.7\cdot\text{extra}\\
C_A &= \text{compares}+\text{swaps}+\text{probes}+\frac{1+\theta}{\theta}.
\end{aligned}$$

**Behaviour in $\theta$.** The purge part $1+4.14/\theta$ *falls* when $\theta$ grows. The depth part grows like $\log_2(1+\theta/2)$ and the dead-top part grows with $\bar f$.
The memory grows like $(1+\theta)$. The model has a **flat minimum for $\theta$ between 0.5 and 1** (purge factor 1.5 to 2). It predicts that $\theta=0.25$ is clearly worse and that large $\theta$ gets slowly worse.
Behaviour in $w$: only $D$ depends on $w$, so $C_A$ grows like $\log_2 w$ (for the median with $\theta=1$ the coefficient of $\log_2 w$ is $2m+2\,\text{extra}\approx1.4$ in compares and $m+\text{extra}\approx0.7$ in swaps, about $2.2$ in total).

# 3. Memory model (peak extra memory above the input array, in bytes)

* **B:** $8U$ (sorted distinct values and tree) $+\,4N$ (id to index) $+\,4N$ (order list) $+\,N$ (alive flags) $+\,4N$ (the constructor's copy of the input) $+\,4N$ (merge-sort scratch).
 For $R=10^6$ and $N=10^6$ this gives about $22$ MB, and it does **not** grow with $w$.
* **A:** $6N$ ($4N$ order list, $N$ alive flags, $N$ side flags) $+\,8\,(1+\theta)\,n$ (heap items at their peak) $+\,56\cdot\theta n/2$ (hash buckets: one `vector` header plus its small allocation).
 It grows linearly with $w$.

# 4. Time model

Time per step $=c\times C$, where $c$ is the cost of one counted operation. A counted operation is a compare and branch, an 8-byte swap, a bucket item test or a
Fenwick loop turn, so I assume 4 to 12 CPU cycles, that is **$c\in[1.25,\,3.75]$ ns** at about 3 GHz. This band is the prediction for time. The ratio of the two structures' times is predicted to follow the ratio of $C_A$ to $C_B$,
with B possibly worse when $U$ is large (its two arrays of $4U$ bytes are touched at random). I make no claim about cache behaviour without profiler output.

# 5. Predictions to be tested (all values from `results/predictions.csv`, $N=10^6$)

1. **P1** B's operations are flat in $w$ ($70.5$ for $R=10^6$, $r=10\%$) and flat in $r$ ($70.5$; $80.5$ at $r=0$).
2. **P2** A's operations grow with $\log_2 w$: $38.6\to46.5\to54.3\to62.2$ for $w=10^2,10^3,10^4,10^5$.
3. **P3** In counted operations A is below B for every $w\le10^5$ (at $R=10^6$) but the gap closes ($62.2$ against $70.5$ at $w=10^5$). No crossing in $w$ is predicted inside the tested range.
4. **P4** With more retractions A gets more expensive ($51.5\to67.9$ for $r=0\to90\%$) but stays just below B ($70.5$): no crossing.
5. **P5** B's cost rises with $\log_2 U$ ($25.8$ at $R=10^2$, $36.9$ at $10^3$, $49.6$ at $10^4$, $60.7$ at $10^5$, $70.5$ at $10^6$) and B passes A ($38.6$, flat for fixed $w=100$) near $R\approx10^3$. B is cheaper than A for $R\le 10^3$ and dearer above.
6. **P6** Memory: B about $22$ MB, flat in $w$, about $17$ MB when $R\le10^4$; A about $6$ MB, rising to about $10$ MB at $w=10^5$.
7. **P7** Purge factor (time and operations): flat minimum at factor $1.5$ to $2$ ($53.5$ and $54.3$); worse at $1.25$ ($59.1$) and slowly worse above ($61.6$, $69.6$, $85.5$ at factors $3$, $4$, $6$).
8. **P8** For $p=90\%$ and $99\%$ both structures get cheaper per step (A $32.1$ and $21.7$; B $60.5$ for both, since $d=1$).
9. **P9** Measured time per step lies inside the band $[1.25\,C,\;3.75\,C]$ ns.

# 6. Known simplifications (stated before measuring)

The model ignores memory allocation (the hash set stores each bucket in a `vector`), branch mispredictions, cache misses and the cost of `get`, `add` and `retract` function calls themselves.
It treats heap contents as random, which is true for the data used here (uniform values) but not for adversarial data. It uses averages only; worst cases belong to D5 of the Adversary.
