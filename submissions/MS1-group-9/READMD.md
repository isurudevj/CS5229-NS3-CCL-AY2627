## Q1. Group information

Group 9

Isuru Anuranga Wijesinghe - E1373624
A Akil Ahamed - E0406341

## Q2. Experiment results and original output

Running ecmp

```shell
cd /app/astra-sim
SEEDS="1 2 3 4 5 6 7 8 9 10" ./build_scripts/astra_ns3/microbenchmarks/run.sh -r ecmp
```

/app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/qlen.txt
/app/astra-sim/extern/network_backend/ns-3/scratch/output/multiring_8ring_64mb_32nodes_ring_2D_1_datasplit_1_parallel_8x4_4_4_32_ecmp_baseline_1/fct.txt

Running placeholder

```shell
cd /app/astra-sim
SEEDS="1 2 3 4 5 6 7 8 9 10" ./build_scripts/astra_ns3/microbenchmarks/run.sh -r placeholder
```

Workload	Placeholder LB completion time (ms)	Your ECMP LB completion time (ms)
32-host, eight-ring AllReduce; 64 MB per host		
Below the table, paste the original simulator output reporting completion for each run into separate, clearly labelled Markdown code blocks.

Include all completion lines needed to support the reported value and show the original units.

A concise excerpt is sufficient; do not replace the output with a manually rewritten summary or screenshot. Briefly state how you obtained the table values from this output.

## Q3. How does your ECMP implementation work?

The next-hop selection is handled in RouteInput() inside load-balancing-ecmp.cc file, which calls a GetFlowHash() helper to compute a hash over the packet's 5-tuple (source IP, destination IP, source port, destination port, and protocol). The output port is then chosen as nextHops[hash % nextHops.size()]. Since the hash depends only on the header fields and not on any runtime state, packets belonging to the same flow always produce the same hash value and land on the same next-hop, provided the candidate path list has not changed.

## Q4. How did you check your implementation?

We have added optional log that can be activated at run time. This log prints the 5-tuple, computed hash, and the selected output device for every forwarded packet.

```shell
NS_LOG="ECMPLoadBalancing=level_debug" \
./build_scripts/astra_ns3/microbenchmarks/run.sh -r ecmp
```

The following are the logs:

ECMP_HASH switch=32 observation=3 sip=184552449 dip=184550401 sport=100 dport=10000 protocol=252 hash=604606148 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=38 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=3
ECMP_HASH switch=33 observation=3 sip=184555009 dip=184552961 sport=100 dport=10000 protocol=252 hash=2118411912 pathIndex=0 pathCount=1 outDev=7
ECMP_HASH switch=34 observation=3 sip=184556545 dip=184554497 sport=100 dport=10000 protocol=252 hash=3059612823 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=32 observation=3 sip=184551681 dip=184549633 sport=100 dport=10000 protocol=252 hash=1355040892 pathIndex=0 pathCount=1 outDev=2
ECMP_HASH switch=33 observation=3 sip=184555265 dip=184553217 sport=100 dport=10000 protocol=252 hash=1163780685 pathIndex=0 pathCount=1 outDev=8
ECMP_HASH switch=34 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=7

From the logs, it can be confirmed that the packets sharing the same source IP, destination IP, ports and protocol always produced the same hash value. For example, both switches 38 and 34 computed hash=920603074 for the same flow. Also, it can be seen that that flows with different 5-tuples produced distinct hashes, mapping to different output devices when multiple paths were available.

Q5. What will you investigate next?

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
^C
```

Our hypothesis is that the ring-based all-reduce traffic pattern generates flows that are highly correlated in their source-destination pairs, causing multiple flows to hash to the same next-hop and create hotspot links while other paths stay underutilised.

To test this hypothesis, we plan to log per-port utilisation and the queue lengths to confirm whether load is skewed across paths, and then experiment to see if dynamically re-assigning short bursts of packets reduces queue depth and brings flow completion time closer to the ideal baseline.