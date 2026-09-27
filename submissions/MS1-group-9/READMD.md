## Q1. Group information

**Group 9**

Isuru Anuranga Wijesinghe - E1373624

A Akil Ahamed - E0406341

## Q2. Experiment results and original output

Running ecmp

```shell
cd /app/astra-sim
SEEDS="1 2 3 4 5 6 7 8 9 10" ./build_scripts/astra_ns3/microbenchmarks/run.sh -r ecmp

tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/qlen.txt
tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/fct.txt

```

Running placeholder

```shell
cd /app/astra-sim
SEEDS="1 2 3 4 5 6 7 8 9 10" ./build_scripts/astra_ns3/microbenchmarks/run.sh -r placeholder

tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_placeholder_1/qlen.txt
tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_placeholder_1/fct.txt
```


| Workload | Placeholder LB completion time (ms) | Your ECMP LB completion time (ms) |
|---|------------------------------------:|---:|
| 32-host, eight-ring AllReduce; 64 MB per host |                             667.964 | 246.962 |


Placeholder completion results

```shell
sys[16] finished, 586298693 cycles
sys[22] finished, 588426311 cycles
sys[7] finished, 611631901 cycles
sys[1] finished, 616945744 cycles
sys[8] finished, 618921637 cycles
sys[30] finished, 625824597 cycles
sys[4] finished, 631745434 cycles
sys[3] finished, 638614599 cycles
sys[14] finished, 643311214 cycles
sys[31] finished, 646009224 cycles
sys[6] finished, 647601130 cycles
sys[23] finished, 655501766 cycles
sys[18] finished, 656235248 cycles
sys[27] finished, 656423628 cycles
sys[20] finished, 658722671 cycles
sys[29] finished, 659231841 cycles
sys[17] finished, 659611270 cycles
sys[15] finished, 659740864 cycles
sys[28] finished, 660105777 cycles
sys[26] finished, 660520203 cycles
sys[10] finished, 660863629 cycles
sys[5] finished, 663593812 cycles
sys[11] finished, 663902464 cycles
sys[24] finished, 665035539 cycles
sys[0] finished, 665048171 cycles
sys[12] finished, 665516845 cycles
sys[9] finished, 666487018 cycles
sys[25] finished, 666570806 cycles
sys[2] finished, 666582810 cycles
sys[13] finished, 666598429 cycles
sys[21] finished, 667223426 cycles
sys[19] finished, 667964577 cycles

```

ECMP completion results

```shell
sys[21] finished, 193338770 cycles
sys[17] finished, 196954085 cycles
sys[18] finished, 197593216 cycles
sys[27] finished, 203490963 cycles
sys[31] finished, 205777358 cycles
sys[25] finished, 207308480 cycles
sys[2] finished, 208203500 cycles
sys[15] finished, 210133847 cycles
sys[19] finished, 212109073 cycles
sys[12] finished, 212742810 cycles
sys[3] finished, 214913193 cycles
sys[14] finished, 217517922 cycles
sys[28] finished, 219916526 cycles
sys[8] finished, 221280528 cycles
sys[23] finished, 222619369 cycles
sys[5] finished, 224854763 cycles
sys[1] finished, 226148385 cycles
sys[13] finished, 227249695 cycles
sys[4] finished, 228009617 cycles
sys[16] finished, 228225450 cycles
sys[11] finished, 234762044 cycles
sys[24] finished, 235216348 cycles
sys[20] finished, 235483302 cycles
sys[29] finished, 236034682 cycles
sys[6] finished, 237133087 cycles
sys[0] finished, 237175701 cycles
sys[26] finished, 237559766 cycles
sys[7] finished, 238521401 cycles
sys[10] finished, 238998167 cycles
sys[22] finished, 240188251 cycles
sys[30] finished, 241217762 cycles
sys[9] finished, 246962370 cycles
```

We took the maximum completion time across all hosts.

## Q3. How does your ECMP implementation work?

The next-hop selection is handled in RouteInput() inside load-balancing-ecmp.cc file, which calls a GetFlowHash() helper to compute a hash over the packet's 5-tuple (source IP, destination IP, source port, destination port, and protocol). The output port is then chosen as nextHops[hash % nextHops.size()]. Since the hash depends only on the header fields and not on any runtime state, packets belonging to the same flow always produce the same hash value and land on the same next-hop, provided the candidate path list has not changed.

## Q4. How did you check your implementation?

We have added optional log that can be activated at run time. This log prints the 5-tuple, computed hash, and the selected output device for every forwarded packet.

```shell
NS_LOG="ECMPLoadBalancing=level_debug" \
./build_scripts/astra_ns3/microbenchmarks/run.sh -r ecmp
```

The following are the logs:

```shell
ECMP_HASH switch=32 observation=3 sip=184552449 dip=184550401 sport=100 dport=10000 protocol=252 hash=604606148 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=38 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=3
ECMP_HASH switch=33 observation=3 sip=184555009 dip=184552961 sport=100 dport=10000 protocol=252 hash=2118411912 pathIndex=0 pathCount=1 outDev=7
ECMP_HASH switch=34 observation=3 sip=184556545 dip=184554497 sport=100 dport=10000 protocol=252 hash=3059612823 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=32 observation=3 sip=184551681 dip=184549633 sport=100 dport=10000 protocol=252 hash=1355040892 pathIndex=0 pathCount=1 outDev=2
ECMP_HASH switch=33 observation=3 sip=184555265 dip=184553217 sport=100 dport=10000 protocol=252 hash=1163780685 pathIndex=0 pathCount=1 outDev=8
ECMP_HASH switch=34 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=7

```

From the logs, it can be confirmed that the packets sharing the same source IP, destination IP, ports and protocol always produced the same hash value. For example, both switches 38 and 34 computed hash=920603074 for the same flow. Also, it can be seen that that flows with different 5-tuples produced distinct hashes, mapping to different output devices when multiple paths were available.

## Q5. What will you investigate next?

We observed queue buildup at several switches even with ECMP enabled. Flow completion times are also running 2-3 times above the ideal baseline.

```shell
tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/qlen.txt

time 145530000 34 j 1 1036 j 8 2072 j 11 10360 j 12 29008
time 145530000 35 j 2 1036 j 6 9324 j 10 1036 j 11 16576 j 12 2072
time 145530000 36 j 2 2072 j 3 43512
time 145530000 37 j 1 29008
time 145530000 38 j 1 12432 j 2 1036
time 145530000 39 j 1 40404 j 2 29008
time 145540000 32 j 9 8288
time 145540000 33 j 11 12432
time 145540000 34 j 1 1036 j 8 2072 j 11 11396 j 12 30044
time 145540000 35 j 2 1036 j 6 9324 j 10 1036 j 11 16576 j 12 2072
```


```shell
root@202446041bb6:/app/astra-sim# tail -f /app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/fct.txt
0b001101 0b001901 10003 100 16000000 119651945 38882797 13425600
0b001101 0b001901 10004 100 16000000 140640958 21789921 13425600
0b000f01 0b001701 10003 100 16000000 130308577 32394697 13425600
0b000501 0b000d01 10004 100 16000000 148118561 14783766 13425600
0b001501 0b001d01 10004 100 16000000 140715880 22470878 13425600
0b001b01 0b000301 10003 100 16000000 119972459 45248704 13425600
0b001f01 0b000701 10003 100 16000000 123412383 43183256 13425600
0b001201 0b001a01 10003 100 16000000 130531292 36624533 13425600
0b001c01 0b000401 10003 100 16000000 124077375 45551023 13425600
0b001801 0b000001 10003 100 16000000 123038966 47753003 13425600
```

Our hypothesis is that the ring-based all-reduce traffic pattern generates flows that are highly correlated in their source-destination pairs, causing multiple flows to hash to the same next-hop and create hotspot links while other paths stay underutilised.

To test this hypothesis, we plan to log per-port utilisation and the queue lengths to confirm whether load is skewed across paths, and then experiment to see if dynamically re-assigning short bursts of packets reduces queue depth and brings flow completion time closer to the ideal baseline.