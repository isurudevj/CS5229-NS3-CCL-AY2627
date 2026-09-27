## Q1. Group information

Group 9

Isuru Anuranga Wijesinghe - E1373624
A Akil Ahamed - E0406341

## Q2. Experiment results and original output


Workload	Placeholder LB completion time (ms)	Your ECMP LB completion time (ms)
32-host, eight-ring AllReduce; 64 MB per host		
Below the table, paste the original simulator output reporting completion for each run into separate, clearly labelled Markdown code blocks.

Include all completion lines needed to support the reported value and show the original units.

A concise excerpt is sufficient; do not replace the output with a manually rewritten summary or screenshot. Briefly state how you obtained the table values from this output.

## Q3. How does your ECMP implementation work?
Next-hop selection is handled in RouteInput() inside load-balancing-ecmp.cc, which calls a GetFlowHash() helper to compute a hash over the packet's 5-tuple (source IP, destination IP, source port, destination port, and protocol). The output port is then chosen as nextHops[hash % nextHops.size()]. Because the hash depends only on the header fields and not on any runtime state, packets belonging to the same flow always produce the same hash value and land on the same next-hop, provided the candidate path list hasn't changed.

## Q4. How did you check your implementation?

We have added optional log that can be activated at run time.

```shell
NS_LOG="ECMPLoadBalancing=level_debug" \
./build_scripts/astra_ns3/microbenchmarks/ecmp.sh -r
```

Log will help to verify if the implementation is hashing the 5 tuples in to same values , same flow result in same hash 

ECMP_HASH switch=32 observation=3 sip=184552449 dip=184550401 sport=100 dport=10000 protocol=252 hash=604606148 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=38 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=3
ECMP_HASH switch=33 observation=3 sip=184555009 dip=184552961 sport=100 dport=10000 protocol=252 hash=2118411912 pathIndex=0 pathCount=1 outDev=7
ECMP_HASH switch=34 observation=3 sip=184556545 dip=184554497 sport=100 dport=10000 protocol=252 hash=3059612823 pathIndex=0 pathCount=1 outDev=5
ECMP_HASH switch=32 observation=3 sip=184551681 dip=184549633 sport=100 dport=10000 protocol=252 hash=1355040892 pathIndex=0 pathCount=1 outDev=2
ECMP_HASH switch=33 observation=3 sip=184555265 dip=184553217 sport=100 dport=10000 protocol=252 hash=1163780685 pathIndex=0 pathCount=1 outDev=8
ECMP_HASH switch=34 observation=3 sip=184557057 dip=184555009 sport=100 dport=10000 protocol=252 hash=920603074 pathIndex=0 pathCount=1 outDev=7


Q5. What will you investigate next?
In 1–2 sentences, state one behavior you observed, a possible explanation, and one experiment you plan to run to test that explanation.

An initial hypothesis is sufficient; you do not need a completed solution at this stage.