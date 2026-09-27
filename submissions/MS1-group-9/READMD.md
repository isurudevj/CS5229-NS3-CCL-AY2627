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

Q4. How did you check your implementation?
In 1–2 sentences, briefly explain how you checked that packets of the same flow follow a consistent path and that different flows can use different paths, and summarize what you observed.

A faster completion time alone does not establish correct ECMP behavior; a small targeted check is sufficient.

Q5. What will you investigate next?
In 1–2 sentences, state one behavior you observed, a possible explanation, and one experiment you plan to run to test that explanation.

An initial hypothesis is sufficient; you do not need a completed solution at this stage.