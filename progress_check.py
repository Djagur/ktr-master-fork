#!/usr/bin/env python3
"""Show which queries are done and which still need to run, per setting.

usage:  python check_progress.py dbpedia [--out results] [--list generated_queries] [--total 70]
A query is done when its <pNN> block is in results/<dataset>/raw/<dataset>_<s>_o<o>_summary.txt (this also
works for runs made before the parts folder existed) or when its status.json in
results/<dataset>/raw/parts/<dataset>_<s>_o<o>/<query>/ says ok. The status files, if present, also tell
which of the others timed out, failed or were interrupted. The query list is query/<dataset>/<list>.json;
if it does not exist, use --total.
"""
import argparse
import json
import re
from pathlib import Path

SETTINGS = [("dist", 0), ("freq", 0), ("dist", 1), ("freq", 1)]


def ranges(idx):
    """[0, 1, 2, 5] -> ['0-2', '5']  (the form run_experiment.py --query accepts)"""
    out, i = [], 0
    while i < len(idx):
        j = i
        while j + 1 < len(idx) and idx[j + 1] == idx[j] + 1:
            j += 1
        out.append(str(idx[i]) if i == j else f"{idx[i]}-{idx[j]}")
        i = j + 1
    return out


def done_in_summary(path):
    """Query numbers that have a <pNN> block in the timing section of a (merged) summary file."""
    if not path.exists():
        return set()
    text = path.read_text(errors="replace").split("Searched Pattern Num:")[0]
    return {int(m) for m in re.findall(r"^<p(\d+)>", text, re.M)}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("dataset")
    ap.add_argument("--out", default="results")
    ap.add_argument("--list", default="generated_queries")
    ap.add_argument("--total", type=int, help="number of queries, if there is no query list file")
    args = ap.parse_args()

    list_path = Path(f"query/{args.dataset}/{args.list}.json")
    if list_path.exists():
        names = json.loads(list_path.read_text())
        print(f"{args.dataset}: {len(names)} queries in {list_path}\n")
    elif args.total:
        names = [f"p{i:02d}" for i in range(args.total)]
        print(f"{args.dataset}: {len(names)} queries (--total)\n")
    else:
        raise SystemExit(f"{list_path} not found; give --list NAME or --total N")
    for s, o in SETTINGS:
        raw = Path(args.out) / args.dataset / "raw"
        parts = raw / "parts" / f"{args.dataset}_{s}_o{o}"
        done = done_in_summary(raw / f"{args.dataset}_{s}_o{o}_summary.txt")
        groups = {"ok": [], "timeout": [], "failed": [], "interrupted": [], "not run": []}
        for i, name in enumerate(names):
            try:
                status = json.loads((parts / name / "status.json").read_text())["status"]
            except (OSError, ValueError, KeyError):
                status = "not run"
            if i in done or status == "ok":
                status = "ok"
            groups[status if status in groups else "failed"].append(i)
        print(f"{s}:{o}   done {len(groups['ok'])}/{len(names)}   " +
              "   ".join(f"{k} {len(v)}" for k, v in groups.items() if k != "ok" and v).rstrip())
        for k, v in groups.items():
            if k != "ok" and v:
                print(f"    {k}: " + ", ".join("-".join("p" + x for x in t.split("-")) for t in ranges(v)))
        todo = groups["not run"] + groups["failed"] + groups["interrupted"]
        if todo:
            print(f"    run: python run_experiment.py {args.dataset} --setting {s}:{o} --query {' '.join(ranges(sorted(todo)))}")
        if groups["timeout"]:
            print(f"    timed out ones again: add --retry-timeouts --query {' '.join(ranges(groups['timeout']))}")
        print()


if __name__ == "__main__":
    main()
