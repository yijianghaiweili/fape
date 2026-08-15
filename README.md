# fape
This repository provides the implementation and experimental artifacts for the paper "FAPE: Efficient FPGA Acceleration for k-Hop Constrained s-t Simple Path Enumeration".

We evaluate the scalability of FAPE using 2, 4, 6, 8, and 10 CUs under identical query workloads. The bars report the total query time for different hop constraints, while the dashed line shows the geometric-mean speedup over the 2-CU configuration. Overall, increasing the number of CUs reduces the total query time, particularly for computation-intensive workloads. With 10 CUs, FAPE achieves geometric-mean speedups of 2.07× and 2.05× on RT and ES, respectively. The scaling benefit is limited for lightweight queries because fixed execution and synchronization overheads become dominant, and the marginal improvement gradually decreases at larger CU counts. These results demonstrate that FAPE supports configurable CU scaling: six CUs provide a favorable performance-to-resource tradeoff for the main evaluation, while additional CUs can be deployed when lower query latency is prioritized and sufficient hardware resources are available.
<img width="3681" height="1230" alt="cu_scaling_query_time" src="https://github.com/user-attachments/assets/f5500367-3d8d-4de0-9816-c1cef6a0c815" />

PEFP clock-frequency and resource-utilization report:
<p><img width="400" height="78" alt="image" src="https://github.com/user-attachments/assets/d6b6293a-8535-46bb-a56b-507c29f1a2e7" /></p>

<p><img width="515" height="38" alt="image" src="https://github.com/user-attachments/assets/0cc4be2b-2303-46cd-a6d6-34c294248dc2" /></p>


The main evaluation uses hot-to-hot queries to provide representative and challenging workloads. High-degree endpoints generally induce larger branching factors, larger intermediate states, and larger search spaces. As a supplementary experiment, we also evaluate non-hot endpoint combinations. The results on RT, ES, and TS show the same overall performance trend as the main evaluation, while the query times are generally shorter because of the less demanding search spaces.
<img width="5538" height="1514" alt="image" src="https://github.com/user-attachments/assets/f57debc6-35f0-4dea-aab4-47d45b01f855" />


