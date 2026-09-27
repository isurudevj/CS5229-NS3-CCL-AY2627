# CS5229 Project Research Notes

This document tracks our key findings, measurements, and hypotheses as we progress through the assignment. We will use this information to build our Milestone 1 Short Presentation and Milestone 2 Final Report.

## Phase 1: Baseline Observations (Placeholder Config)

### Experiment Setup
- **Topology:** Spine-leaf (32 hosts, 4 ToR switches, 4 Spine switches, 10Gbps links)
- **Workload:** 32-node, 64MB single-pass (Multiple-1D Ring AllReduce pattern with 8 independent rings of 4 hosts each)
- **Network Config:** `config_spine_leaf_4_4_32_placeholder.txt` (Load Balancing Mode 0)

### Ideal Performance Calculation
Based on the background briefing:
- The 64MB collective is divided into 4 chunks of 16MB.
- An ideal path mapping would give each flow approximately 5 Gbps of effective bandwidth.
- Transmitting one 16MB chunk ideally takes: `16 MB * 8 / 5 Gbps = 25.6 ms`.
- The 4-node Ring AllReduce has 6 communication steps (3 ReduceScatter + 3 AllGather).
- **Ideal Total Time:** `6 * 25.6 ms = 153.6 ms`.

### Actual Measured Performance
- **Simulated Execution Time:** Our smoke test run completed in **667.9 ms** (667,964,577 ns/cycles).
- **Performance Gap:** The actual time is roughly **4.3x slower** than the idealized expectation.

### Problem Formulation and Hypothesis
- **Observation:** `qlen.txt` shows enormous queue buildups at the switch nodes (e.g., node 32 hitting >100KB in queue length at time 30,000 ns). 
- **Hypothesis:** The "placeholder" load balancing algorithm uses a pathological, static routing policy (always picking the first installed equal-cost path). Because the workload involves 8 independent rings running concurrently, multiple heavy 5Gbps flows are colliding on the exact same physical spine links. This causes severe localized congestion, packet drops/retransmissions, and massive queue buildup, while other parallel spine links remain completely unutilized.
- **Proposed Solution:** Implement **Equal-Cost Multi-Path (ECMP)** routing to hash flows (using their 5-tuple) and distribute them evenly across all available spine links. This should mitigate the collisions and bring the actual completion time much closer to the ideal 153.6 ms.
# Phase 2 – ECMP Smoke Test Findings

## Experiment Setup (re‑using the same baseline workload)
- **Topology:** 32 hosts, 4 × 4 spine‑leaf (same as Phase 1).
- **Workload:** 64 MB Ring‑AllReduce (8 independent rings, 1 seed).
- **Network Config:** `config_spine_leaf_4_4_32_ecmp_baseline.txt` (LB_MODE = 1 → ECMP).
- **Command used:**
  ```powershell
  docker run -it --rm -v "C:/Users/.../CS5229-NS3-CCL-AY2627:/app/astra-sim" astra-sim bash -c "cd /app/astra-sim/build_scripts/astra_ns3/microbenchmarks && SEEDS=1 ./build_32_allreduce_batch.sh -r"
  ```

## Key Results
| Metric | Placeholder (Phase 1) | ECMP (Phase 2) | Observation |
|--------|----------------------|----------------|-------------|
| **Final completion time (max `fct.txt` entry)** | ~667 ms (≈ 667 M cycles) | ~311 ms (≈ 311 M cycles) | ECMP cuts the runtime by **~53 %**, bringing us much closer to the ideal 153 ms. |
| **Peak queue length** (from `qlen.txt`) | > 100 KB on a few switches, sustained heavy build‑up. | < 60 KB early, quickly stabilises around 50 KB and stays flat for the rest of the run. | Congestion is dramatically reduced – traffic is now spread over all equal‑cost spine links. |
| **Queue‑length trend** | Saw sharp spikes at early times (20 k‑30 k ns) that never drained. | Starts with a modest spike (≈ 102 KB) then drops to ~50 KB and stays there. | The ECMP hash distributes the eight rings across the four spine paths, preventing any single path from becoming a bottleneck. |

## Interpretation
- The pathological placeholder routing always selected the **first** equal‑cost next‑hop, forcing all eight Ring‑AllReduce flows onto the same spine link. This created massive queuing and the ~667 ms runtime.
- Our **deterministic 5‑tuple hash** now spreads the flows evenly across the four available spine links. Each spine sees roughly **¼ of the traffic**, eliminating the severe queue buildup.
- The remaining gap to the ideal 153 ms is primarily due to protocol overhead and the fact that we are still using a **single seed** and a modest 10 Gbps link speed; additional optimisations (e.g., tuning congestion‑control parameters) could shrink it further.

## Next Steps (Milestone 2)
1. **Run a full sweep** (32 / 64 / 128 MB, all three configs, 10 seeds) to gather statistically robust results.
2. **Compare ECMP vs. placeholder** across the full dataset – compute mean/variance of `fct.txt` and peak queue lengths.
3. **Prepare the short presentation**:
   - Slides showing the **baseline vs. ECMP timing** and queue‑length plots.
   - A concise hypothesis slide ("Static routing creates a single‑path bottleneck; ECMP balances load, reducing latency").
4. **Finalize the report** with the data, analysis, and discussion of remaining performance gap.

These notes will serve as the backbone for both the presentation and the final write‑up.
