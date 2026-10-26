#!/usr/bin/env python3
"""Aggregate the results of ./main per setting (dataset, score s, search mode o).

Expected input files (written by the awk filter), one set per setting:
  <dataset>_<s>_o<o>_summary.txt   required  "Process Time" + "Searched Pattern Num"
  <dataset>_<s>_o<o>_top.txt       optional  compact exception list
  <dataset>_<s>_o<o>_meta.txt      optional  command, date, CPU info
e.g. nell_dist_o0_summary.txt

Queries are assumed to be ordered rho0, rho1, ... with --per-rule queries each.

Usage:
  python aggregate_results.py DIR_OR_FILES... [--out results] [--per-rule 10]
"""
import argparse
import csv
import re
import statistics as st
from collections import defaultdict
from pathlib import Path

PREFIX_RE = re.compile(r"(?P<ds>.+)_(?P<s>dist|freq)_o(?P<o>\d)$")
BLOCK_RE = re.compile(r"^<p(\d+)>")
PHASE_RE = re.compile(r"^\s*(\d\d(?:-\d)?\. .+?):\s*([\d.]+)\s*$")
SUM_RE = re.compile(r"^\s*SumTime:\s*([\d.]+)")
COUNT_RE = re.compile(r"^\s*(\d+):\s*(\d+)\s*$")
RANK_RE = re.compile(r"^(\d+)(?:st|nd|rd|th) Exception\.")
SCORE_RE = re.compile(r"^Exception Score is\s+([\d.eE+-]+)")
PAT_RE = re.compile(r"^<Pattern ([\d-]+)>")
PE_RE = re.compile(r"^pe-(\d+): (.*?)\s+/\s+([\d.eE+-]+) \((\w+)\)")

# Paper, NELL, average runtime [s]: rule -> (proposed, baseline), per score type.
PAPER = {
    ("nell", "dist"): {0: (0.0158, 0.0155), 1: (4.342, 4.461), 2: (71.703, 79.701),
                       3: (0.044, 0.030), 4: (83.343, 185.944), 5: (50.190, 62.424),
                       6: (1398, 9098)},
    ("nell", "freq"): {0: (0.0162, 0.0156), 1: (4.145, 4.441), 2: (73.196, 77.845),
                       3: (0.045, 0.030), 4: (15.456, 203.072), 5: (36.408, 61.558),
                       6: (485, 9265)},
}


# ---------------------------------------------------------------- parsing
def parse_summary(path):
    """Stream the file; works on a summary file or on a full log."""
    prep = {}
    queries = defaultdict(lambda: {"phases": {}, "sum": None, "counts": {}})
    section, cur = None, None
    with open(path, errors="replace") as f:
        for line in f:
            if line.startswith("Process Time:"):
                section = "time"
                continue
            if line.startswith("Searched Pattern Num:"):
                section = "count"
                continue
            if section is None:
                continue
            s = line.strip()
            if s == "<Preparation>":
                cur = "prep"
                continue
            m = BLOCK_RE.match(s)
            if m:
                cur = int(m.group(1))
                continue
            if cur is None:
                continue
            if section == "time":
                m = SUM_RE.match(line)
                if m:
                    if cur == "prep":
                        prep["SumTime"] = float(m.group(1))
                    else:
                        queries[cur]["sum"] = float(m.group(1))
                    continue
                m = PHASE_RE.match(line)
                if m:
                    target = prep if cur == "prep" else queries[cur]["phases"]
                    target[m.group(1)] = float(m.group(2))
            else:
                m = COUNT_RE.match(line)
                if m and cur != "prep":
                    queries[cur]["counts"][int(m.group(1))] = int(m.group(2))
    return prep, dict(queries)


def parse_top(path, n_queries=None):
    """Exceptions with a query index.

    If the number of lines starting with '---' equals n_queries, those lines are
    taken as query separators. Otherwise a new query is assumed whenever the rank
    goes down, which fails for queries with no exceptions or with a single one.
    Returns (exceptions, method)."""
    exceptions, q, last_rank, cur, delim = [], -1, None, None, 0
    with open(path, errors="replace") as f:
        for line in f:
            if line.startswith("---"):
                delim += 1
                continue
            m = RANK_RE.match(line)
            if m:
                rank = int(m.group(1))
                if last_rank is None or rank < last_rank:
                    q += 1
                last_rank = rank
                cur = {"query": q, "rank": rank, "_delim": delim - 1}
                continue
            if cur is None:
                continue
            m = SCORE_RE.match(line)
            if m:
                cur["score"] = float(m.group(1))
                continue
            m = PAT_RE.match(line)
            if m:
                cur["pattern"] = m.group(1)
                continue
            m = PE_RE.match(line)
            if m:
                cur.update(pe=int(m.group(1)), triple=m.group(2), type=m.group(4))
                cur.setdefault("score", float(m.group(3)))
                exceptions.append(cur)
                cur = None
    method = "rank"
    if n_queries is not None and delim == n_queries:
        method = "separator"
        for e in exceptions:
            e["query"] = e["_delim"]
    for e in exceptions:
        e.pop("_delim")
    return exceptions, method


# ------------------------------------------------------------ aggregation
def stats(xs):
    return {"n": len(xs), "mean": st.mean(xs), "sd": st.stdev(xs) if len(xs) > 1 else 0.0,
            "median": st.median(xs), "min": min(xs), "max": max(xs)}


def write_csv(path, rows, fields=None):
    if not rows:
        return
    fields = fields or list(dict.fromkeys(k for r in rows for k in r))
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, restval="")
        w.writeheader()
        w.writerows(rows)


def process(prefix, summary, top, meta, per_rule, out):
    prep, queries = parse_summary(summary)
    exceptions, method = parse_top(top, len(queries)) if top.exists() else ([], None)
    n_blocks = len({e["query"] for e in exceptions})
    if method == "rank" and exceptions and n_blocks != len(queries):
        print(f"WARNING {prefix}: could not split the top file into queries "
              f"({n_blocks} blocks found by rank, {len(queries)} queries in summary); "
              f"query numbers in the exceptions file and the o0-vs-o1 overlap/score checks are unreliable.")

    sizes = sorted({s for q in queries.values() for s in q["counts"]})
    rows = []
    for i in sorted(queries):
        q = queries[i]
        row = {"query": f"p{i:02d}", "rule": i // per_rule, "sum_time": q["sum"],
               "patterns_total": sum(q["counts"].values())}
        row.update({f"patterns_size{s}": q["counts"].get(s) for s in sizes})
        row.update(q["phases"])
        rows.append(row)

    by_rule = {}
    for r in sorted({x["rule"] for x in rows}):
        sel = [x for x in rows if x["rule"] == r and x["sum_time"] is not None]
        if sel:
            by_rule[r] = stats([x["sum_time"] for x in sel])
            by_rule[r]["patterns_mean"] = st.mean(x["patterns_total"] for x in sel)

    write_csv(out / f"{prefix}_queries.csv", rows)
    write_csv(out / f"{prefix}_by_rule.csv",
              [{"rule": r, **v} for r, v in by_rule.items()])
    for e in exceptions:
        e["rule"] = e["query"] // per_rule
    write_csv(out / f"{prefix}_exceptions.csv", exceptions,
              ["query", "rule", "rank", "score", "type", "pattern", "pe", "triple"])
    return {"prep": prep, "queries": queries, "exceptions": exceptions, "by_rule": by_rule,
            "meta": meta.read_text(errors="replace") if meta.exists() else ""}


def group_by_query(exceptions):
    g = defaultdict(list)
    for e in exceptions:
        g[e["query"]].append(e)
    return g


def compare(d0, d1, per_rule):
    """Per-query comparison of o=0 (pruning) against o=1 (exhaustive)."""
    q0, q1 = d0["queries"], d1["queries"]
    e0, e1 = group_by_query(d0["exceptions"]), group_by_query(d1["exceptions"])
    rows = []
    for i in sorted(set(q0) & set(q1)):
        t0, t1 = q0[i]["sum"], q1[i]["sum"]
        row = {"query": f"p{i:02d}", "rule": i // per_rule, "time_o0": t0, "time_o1": t1,
               "speedup": (t1 / t0) if t0 and t1 is not None else None,
               "patterns_o0": sum(q0[i]["counts"].values()),
               "patterns_o1": sum(q1[i]["counts"].values())}
        a, b = e0.get(i), e1.get(i)
        if a and b:
            # ties can come out in a different order, so compare sorted rounded scores
            row["same_scores"] = (sorted(round(e["score"], 2) for e in a)
                                  == sorted(round(e["score"], 2) for e in b))
            ta, tb = {e["triple"] for e in a}, {e["triple"] for e in b}
            row["triple_overlap"] = round(len(ta & tb) / len(tb), 3)
        rows.append(row)
    return rows


# ----------------------------------------------------------------- report
def build_markdown(settings, compares):
    groups = defaultdict(dict)
    for (ds, s, o), d in settings.items():
        groups[(ds, s)][o] = d
    L = ["# Results", "",
         "Times are `SumTime` per query [s], summarised over the queries of each rule "
         "(mean ± sd across queries, median in brackets). The paper's ± may mean something else.", ""]
    for (ds, s), by_o in sorted(groups.items()):
        os_ = sorted(by_o)
        paper = PAPER.get((ds, s))
        head = (["rule"] + [f"o={o} time [s]" for o in os_] + [f"o={o} patterns" for o in os_]
                + (["paper proposed [s]", "paper baseline [s]"] if paper else []))
        L += [f"## {ds}, s={s}", "", "| " + " | ".join(head) + " |", "|" + "---|" * len(head)]
        for r in sorted({r for o in os_ for r in by_o[o]["by_rule"]}):
            cells = [f"ρ{r}"]
            for o in os_:
                t = by_o[o]["by_rule"].get(r)
                cells.append(f"{t['mean']:.4g} ± {t['sd']:.4g} ({t['median']:.4g})" if t else "-")
            for o in os_:
                t = by_o[o]["by_rule"].get(r)
                cells.append(f"{t['patterns_mean']:.0f}" if t else "-")
            if paper:
                cells += [f"{x:.4g}" for x in paper[r]] if r in paper else ["-", "-"]
            L.append("| " + " | ".join(cells) + " |")
        L.append("")
    for (ds, s), rows in sorted(compares.items()):
        L += [f"## {ds}, s={s}: o=0 (pruning) vs o=1 (exhaustive)", "",
              "| rule | median speedup | mean speedup | queries with same scores |", "|---|---|---|---|"]
        for r in sorted({x["rule"] for x in rows}):
            sel = [x for x in rows if x["rule"] == r]
            sp = [x["speedup"] for x in sel if x["speedup"] is not None]
            chk = [x["same_scores"] for x in sel if "same_scores" in x]
            L.append(f"| ρ{r} | {st.median(sp):.3g} | {st.mean(sp):.3g} | "
                     f"{sum(chk)}/{len(chk)} |" if sp else f"| ρ{r} | - | - | - |")
        L.append("")
    L += ["## Run info", ""]
    for (ds, s, o), d in sorted(settings.items()):
        if d["meta"]:
            L += [f"### {ds}, s={s}, o={o}", "```", d["meta"].rstrip(), "```", ""]
    return "\n".join(L)


# ------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("inputs", nargs="+", help="directories or *_summary.txt files")
    ap.add_argument("--out", default="results", help="output directory (default: results)")
    ap.add_argument("--per-rule", type=int, default=10, help="queries per rule (default: 10)")
    args = ap.parse_args()

    files = []
    for p in map(Path, args.inputs):
        files += sorted(p.glob("*_summary.txt")) if p.is_dir() else [p]
    if not files:
        print("No *_summary.txt files found in the given inputs.")
        return
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    settings = {}
    for summary in files:
        print(f"found {summary}")
        prefix = summary.name[:-len("_summary.txt")]
        m = PREFIX_RE.match(prefix)
        if not m:
            print(f"skipping {summary.name}: name must look like <dataset>_<dist|freq>_o<N>_summary.txt")
            continue
        top = summary.with_name(f"{prefix}_top.txt")
        meta = summary.with_name(f"{prefix}_meta.txt")
        key = (m["ds"], m["s"], int(m["o"]))
        settings[key] = process(prefix, summary, top, meta, args.per_rule, out)
        print(f"processed {prefix}: {len(settings[key]['queries'])} queries, "
              f"{len(settings[key]['exceptions'])} exceptions")

    compares = {}
    for (ds, s, o), d in settings.items():
        if o == 0 and (ds, s, 1) in settings:
            rows = compare(d, settings[(ds, s, 1)], args.per_rule)
            compares[(ds, s)] = rows
            write_csv(out / f"{ds}_{s}_o0_vs_o1.csv", rows)

    all_rows = [{"dataset": ds, "s": s, "o": o, "rule": r, **v}
                for (ds, s, o), d in sorted(settings.items()) for r, v in d["by_rule"].items()]
    write_csv(out / "by_rule_all.csv", all_rows)
    (out / "results.md").write_text(build_markdown(settings, compares), encoding="utf-8")
    if not settings:
        print("No file matched <dataset>_<dist|freq>_o<N>_summary.txt, so the report is empty.")
    print(f"written to {out}/")


if __name__ == "__main__":
    main()
