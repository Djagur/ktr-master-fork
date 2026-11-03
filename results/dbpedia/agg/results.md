# Results

Times are `SumTime` per query [s], summarised over the queries of each rule (mean ± sd across queries, median in brackets). The paper's ± may mean something else.

## dbpedia, s=dist

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns | paper proposed [s] | paper baseline [s] |
|---|---|---|---|---|---|---|
| ρ0 | 0.4203 ± 0.5876 (0.1445) | 0.3746 ± 0.5703 (0.136) | 2 | 1 | 0.066 | 0.057 |
| ρ1 | 86.64 ± 179.2 (7.009) | 88.62 ± 184.2 (7.013) | 1747 | 1746 | 40.38 | 36.71 |
| ρ2 | 17.49 ± 20.33 (11.38) | 17.44 ± 20.25 (11.35) | 830 | 829 | 374.5 | 386.2 |
| ρ3 | 0.4993 ± 0.7738 (0.251) | 0.3419 ± 0.7147 (0.128) | 3 | 1 | 0.139 | 0.11 |
| ρ4 | 4489 ± 7050 (1687) | - | 121317 | - | 1150 | 2010 |
| ρ5 | 6502 ± 0 (6502) | - | 13004 | - | 1638 | 1693 |
| ρ6 | 494.6 ± 857.8 (126.6) | - | 10463 | - | 1406 | 1.082e+04 |

## dbpedia, s=freq

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns | paper proposed [s] | paper baseline [s] |
|---|---|---|---|---|---|---|
| ρ0 | 0.3866 ± 0.5856 (0.122) | 0.3739 ± 0.5945 (0.1115) | 2 | 1 | 0.071 | 0.062 |
| ρ1 | 87.4 ± 180.1 (7.366) | 89.77 ± 185.7 (7.27) | 1747 | 1746 | 37.55 | 35.57 |
| ρ3 | 0.4465 ± 0.7157 (0.223) | - | 3 | - | 0.155 | 0.109 |

## dbpedia, s=dist: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|
| ρ0 | 0.961 | 0.789 | 10/10 |
| ρ1 | 0.998 | 0.985 | 10/10 |
| ρ2 | 0.994 | 0.987 | 10/10 |
| ρ3 | 0.481 | 0.464 | 6/6 |

## dbpedia, s=freq: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|
| ρ0 | 0.972 | 0.901 | 10/10 |
| ρ1 | 1.03 | 1 | 2/10 |

## Run info

### dbpedia, s=dist, o=0
```
./main -d dbpedia -q queries -k 100 -s dist -o 0 -f 0
mode: incremental, one process per query (results merged from per-query parts)
first start: 2026-10-30 12:46:33
last finish: 2026-11-01 05:22:16
host: vritra
nproc: 36
queries done: 61/70
queries per process: 1; timeout per process: 36000.0 s
timed out (not in the results): p51, p52
failed or interrupted (not in the results): p53 (interrupted)
timings kept but exceptions not attributable to single queries: none
sum of wall-clock time of the processes including graph loading: 17:37:31
largest resident memory of a query process: 150.97 GB
Architecture:                         x86_64
CPU op-mode(s):                       32-bit, 64-bit
Address sizes:                        46 bits physical, 48 bits virtual
Byte Order:                           Little Endian
CPU(s):                               36
On-line CPU(s) list:                  0-35
Vendor ID:                            GenuineIntel
Model name:                           Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
CPU family:                           6
Model:                                63
Thread(s) per core:                   1
Core(s) per socket:                   18
Socket(s):                            2
Stepping:                             2
CPU max MHz:                          3600.0000
```

### dbpedia, s=dist, o=1
```
./main -d dbpedia -q queries -k 100 -s dist -o 1 -f 0
mode: incremental, one process per query (results merged from per-query parts)
first start: 2026-11-02 13:59:26
last finish: 2026-11-02 14:28:42
host: vritra
nproc: 36
queries done: 40/70
queries per process: 10; timeout per process: 36000.0 s
timed out (not in the results): none
failed or interrupted (not in the results): p40 (interrupted), p41 (interrupted), p42 (interrupted), p43 (interrupted), p44 (interrupted), p45 (interrupted), p46 (interrupted), p47 (interrupted), p48 (interrupted), p49 (interrupted)
timings kept but exceptions not attributable to single queries: none
sum of wall-clock time of the processes including graph loading: 0:29:15
largest resident memory of a query process: 9.5 GB
Architecture:                         x86_64
CPU op-mode(s):                       32-bit, 64-bit
Address sizes:                        46 bits physical, 48 bits virtual
Byte Order:                           Little Endian
CPU(s):                               36
On-line CPU(s) list:                  0-35
Vendor ID:                            GenuineIntel
Model name:                           Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
CPU family:                           6
Model:                                63
Thread(s) per core:                   1
Core(s) per socket:                   18
Socket(s):                            2
Stepping:                             2
CPU max MHz:                          3600.0000
```

### dbpedia, s=freq, o=0
```
./main -d dbpedia -q queries -k 100 -s freq -o 0 -f 0
mode: incremental, one process per query (results merged from per-query parts)
first start: 2026-11-02 15:39:07
last finish: 2026-11-02 19:32:55
host: bfa6f24eff8e
nproc: 36
queries done: 30/70
queries per process: 10; timeout per process: 10000.0 s
timed out (not in the results): p20, p21, p22, p23, p24, p25, p26, p27, p28, p29
failed or interrupted (not in the results): none
timings kept but exceptions not attributable to single queries: none
sum of wall-clock time of the processes including graph loading: 1:07:05
largest resident memory of a query process: 9.5 GB
Architecture:                         x86_64
CPU op-mode(s):                       32-bit, 64-bit
Byte Order:                           Little Endian
Address sizes:                        46 bits physical, 48 bits virtual
CPU(s):                               36
On-line CPU(s) list:                  0-35
Thread(s) per core:                   1
Core(s) per socket:                   18
Socket(s):                            2
NUMA node(s):                         2
Vendor ID:                            GenuineIntel
CPU family:                           6
Model:                                63
Model name:                           Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
Stepping:                             2
```

### dbpedia, s=freq, o=1
```
./main -d dbpedia -q queries -k 100 -s freq -o 1 -f 0
mode: incremental, one process per query (results merged from per-query parts)
first start: 2026-11-03 04:50:47
last finish: 2026-11-03 07:15:13
host: e5d45d71764d
nproc: 36
queries done: 20/70
queries per process: 10; timeout per process: 10800.0 s
timed out (not in the results): none
failed or interrupted (not in the results): none
timings kept but exceptions not attributable to single queries: none
sum of wall-clock time of the processes including graph loading: 2:24:26
largest resident memory of a query process: 9.5 GB
Architecture:                         x86_64
CPU op-mode(s):                       32-bit, 64-bit
Byte Order:                           Little Endian
Address sizes:                        46 bits physical, 48 bits virtual
CPU(s):                               36
On-line CPU(s) list:                  0-35
Thread(s) per core:                   1
Core(s) per socket:                   18
Socket(s):                            2
NUMA node(s):                         2
Vendor ID:                            GenuineIntel
CPU family:                           6
Model:                                63
Model name:                           Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
Stepping:                             2
```
