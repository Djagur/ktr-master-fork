# Results

Times are `SumTime` per query [s], summarised over the queries of each rule (mean ± sd across queries, median in brackets). The paper's ± may mean something else.

## nell, s=dist

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns | paper proposed [s] | paper baseline [s] |
|---|---|---|---|---|---|---|
| ρ0 | 0.021 ± 0.02225 (0.0155) | 0.0208 ± 0.02211 (0.0155) | 2 | 1 | 0.0158 | 0.0155 |
| ρ1 | 0.0943 ± 0.06567 (0.098) | 0.1103 ± 0.09439 (0.0935) | 76 | 74 | 4.342 | 4.461 |
| ρ2 | 3.466 ± 6.96 (0.262) | 4.23 ± 9.163 (0.2685) | 747 | 746 | 71.7 | 79.7 |
| ρ3 | 0.2109 ± 0.3038 (0.0955) | 0.0183 ± 0.04121 (0.0055) | 3 | 1 | 0.044 | 0.03 |
| ρ4 | 213.2 ± 406.8 (38.6) | 798.2 ± 1533 (39.68) | 39979 | 217763 | 83.34 | 185.9 |
| ρ5 | 62.06 ± 81.91 (40.03) | 87.47 ± 147.2 (27.07) | 4169 | 12808 | 50.19 | 62.42 |
| ρ6 | 77.43 ± 114.3 (20.23) | 174.4 ± 237.3 (28.37) | 44452 | 119868 | 1398 | 9098 |

## nell, s=freq

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns | paper proposed [s] | paper baseline [s] |
|---|---|---|---|---|---|---|
| ρ0 | 0.0224 ± 0.02271 (0.0165) | 0.0221 ± 0.02265 (0.017) | 2 | 1 | 0.0162 | 0.0156 |
| ρ1 | 0.0811 ± 0.06063 (0.09) | 0.1114 ± 0.09713 (0.0935) | 76 | 74 | 4.145 | 4.441 |
| ρ2 | 3.231 ± 6.486 (0.2355) | 4.203 ± 9.093 (0.265) | 747 | 746 | 73.2 | 77.84 |
| ρ3 | 0.2059 ± 0.3015 (0.0825) | 0.0162 ± 0.03601 (0.0045) | 3 | 1 | 0.045 | 0.03 |
| ρ4 | 124.4 ± 239.2 (28.25) | 873.7 ± 1700 (39.61) | 15709 | 217763 | 15.46 | 203.1 |
| ρ5 | 39.13 ± 45.44 (30.28) | 88.22 ± 151.2 (26.73) | 2917 | 12808 | 36.41 | 61.56 |
| ρ6 | 35.53 ± 71.75 (12.04) | 177.7 ± 239.9 (28.83) | 27696 | 119868 | 485 | 9265 |

## nell, s=dist: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|
| ρ0 | 1 | 0.994 | 10/10 |
| ρ1 | 1 | 1.13 | 10/10 |
| ρ2 | 1 | 1 | 10/10 |
| ρ3 | 0.145 | 0.236 | 5/5 |
| ρ4 | 1.33 | 2.28 | 10/10 |
| ρ5 | 0.875 | 0.955 | 9/9 |
| ρ6 | 1.42 | 1.87 | 10/10 |

## nell, s=freq: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|
| ρ0 | 1 | 0.957 | 10/10 |
| ρ1 | 1.06 | 1.33 | 2/10 |
| ρ2 | 1.1 | 1.08 | 2/10 |
| ρ3 | 0.0943 | 0.225 | 5/5 |
| ρ4 | 2.91 | 4.24 | 2/10 |
| ρ5 | 1.14 | 1.35 | 1/9 |
| ρ6 | 3.52 | 6.78 | 1/10 |

## Run info

### nell, s=dist, o=0
```
./main -d nell -q queries -k 100 -s dist -o 0 -f 0
Tue Oct 27 05:40:19 PM JST 2026
host: vritra
nproc: 36
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
exit status: 0
elapsed: 4163 s
Tue Oct 27 06:49:42 PM JST 2026
```

### nell, s=dist, o=1
```
./main -d nell -q queries -k 100 -s dist -o 1 -f 0
Tue Oct 27 07:35:20 PM JST 2026
host: vritra
nproc: 36
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
exit status: 0
elapsed: 12529 s
Tue Oct 27 11:04:09 PM JST 2026
```

### nell, s=freq, o=0
```
./main -d nell -q queries -k 100 -s freq -o 0 -f 0
Tue Oct 27 06:49:42 PM JST 2026
host: vritra
nproc: 36
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
exit status: 0
elapsed: 2738 s
Tue Oct 27 07:35:20 PM JST 2026
```

### nell, s=freq, o=1
```
./main -d nell -q queries -k 100 -s freq -o 1 -f 0
Tue Oct 27 11:04:09 PM JST 2026
host: vritra
nproc: 36
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
exit status: 0
elapsed: 13668 s
Wed Oct 28 02:51:57 AM JST 2026
```
