import re
import sys
import statistics as st

text = open(sys.argv[1]).read()
sec = text.split("Process Time:")[1].split("Searched Pattern Num:")[0]
times = [
    float(re.search(r"SumTime:\s*([\d.]+)", m.group(2)).group(1))
    for m in re.finditer(r"<p(\d+)>(.*?)(?=<p\d+>|\Z)", sec, re.S)
]
for r in range(7):
    g = times[r * 10 : (r + 1) * 10]
    print(
        f"rho{r}: mean {st.mean(g):.4g}  sd {st.stdev(g):.4g}  median {st.median(g):.4g}  min {min(g):.4g}  max {max(g):.4g}"
    )

