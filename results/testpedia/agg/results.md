# Results

Times are `SumTime` per query [s], summarised over the queries of each rule (mean ± sd across queries, median in brackets). The paper's ± may mean something else.

## testpedia, s=dist

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns |
|---|---|---|---|---|

## testpedia, s=freq

| rule | o=0 time [s] | o=1 time [s] | o=0 patterns | o=1 patterns |
|---|---|---|---|---|

## testpedia, s=dist: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|

## testpedia, s=freq: o=0 (pruning) vs o=1 (exhaustive)

| rule | median speedup | mean speedup | queries with same scores |
|---|---|---|---|

## Run info

### testpedia, s=dist, o=0
```
./main -d testpedia -q queries -k 100 -s dist -o 0 -f 0
Tue Oct 27 05:35:29 PM JST 2026
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
elapsed: 0 s
Tue Oct 27 05:35:29 PM JST 2026
```

### testpedia, s=dist, o=1
```
./main -d testpedia -q queries -k 100 -s dist -o 1 -f 0
Tue Oct 27 05:36:17 PM JST 2026
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
elapsed: 0 s
Tue Oct 27 05:36:17 PM JST 2026
```

### testpedia, s=freq, o=0
```
./main -d testpedia -q queries -k 100 -s freq -o 0 -f 0
Tue Oct 27 05:36:07 PM JST 2026
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
elapsed: 0 s
Tue Oct 27 05:36:07 PM JST 2026
```

### testpedia, s=freq, o=1
```
./main -d testpedia -q queries -k 100 -s freq -o 1 -f 0
Tue Oct 27 05:36:13 PM JST 2026
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
elapsed: 0 s
Tue Oct 27 05:36:14 PM JST 2026
```
