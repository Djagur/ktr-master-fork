#!/usr/bin/env python3
"""Print the rules rho0 .. rho6 together with example contexts from a query list, for illustration.

Every context listed in the queries file (query/<dataset>/<name>/context.json) is classified
structurally into one of the rules below, and label / relation IDs are resolved to their names
using dataset/<dataset>/<dataset>_label.json and _link.json.

  rho0: L -r-> L        rho1: L -r-> *        rho2: L -*-> *
  rho3: L -r-> L, L -r-> L                    rho4: L -r-> *, L -*-> *
  rho5: * -*-> L, * -*-> L                    rho6: L -*-> *, L -*-> *
  (L = label, r = relation, * = wildcard; IDs >= --wildcard are wildcards)

usage:  python show_rhos.py <dataset> [--list query/<dataset>/queries.json]
            [--per-rho 1] [--raw] [--wildcard 10000000]
examples:
  python show_rhos.py nell
  python show_rhos.py dbpedia --list query/dbpedia/generated_queries.json --per-rho 3
"""

import argparse
import json
from collections import Counter
from pathlib import Path

# rule -> (center is a label?, sorted branches as (relation fixed?, tail is a label?))
RULES = {
    0: ("L -r-> L", True, [(True, True)]),
    1: ("L -r-> *", True, [(True, False)]),
    2: ("L -*-> *", True, [(False, False)]),
    3: ("L -r-> L, L -r-> L", True, [(True, True), (True, True)]),
    4: ("L -r-> *, L -*-> *", True, [(False, False), (True, False)]),
    5: ("* -*-> L, * -*-> L", False, [(False, True), (False, True)]),
    6: ("L -*-> *, L -*-> *", True, [(False, False), (False, False)]),
}


def load_names(path):
    try:
        with open(path, encoding="utf-8") as f:
            return {e["ID"]: e.get("Name", str(e["ID"])) for e in json.load(f)}
    except FileNotFoundError:
        print(f"warning: {path} not found, showing raw IDs")
        return {}


def center_of(links):
    """The node shared by every link (contexts are stars); falls back to the first head."""
    common = set.intersection(*({l["HeadID"], l["TailID"]} for l in links))
    if len(links) == 1 or not common:
        return links[0]["HeadID"]
    return min(common)


def classify(ctx, W):
    ents, links = ctx["Entity"], ctx["Link"]
    if not links:
        return None
    c = center_of(links)
    if not all(c in (l["HeadID"], l["TailID"]) for l in links):
        return None
    center_label = ents[c]["EntityID"] < W
    branches = []
    for l in links:
        other = l["TailID"] if l["HeadID"] == c else l["HeadID"]
        branches.append((l["LinkID"] < W, ents[other]["EntityID"] < W))
    key = (center_label, sorted(branches))
    for rho, (_, cl, br) in RULES.items():
        if (cl, br) == key:
            return rho
    return None


def render(ctx, W, labels, links_names):
    """Readable one-liner plus a small tree drawing around the center node."""
    ents, links = ctx["Entity"], ctx["Link"]
    ln = lambda i: "*" if ents[i]["EntityID"] >= W else labels.get(ents[i]["EntityID"], f"#{ents[i]['EntityID']}")
    rn = lambda r: "*" if r >= W else links_names.get(r, f"#{r}")
    one = "; ".join(f"{ln(l['HeadID'])} -[{rn(l['LinkID'])}]-> {ln(l['TailID'])}" for l in links)

    c = center_of(links)
    tree = [ln(c)]
    for j, l in enumerate(links):
        last = j == len(links) - 1
        branch = "└─" if last else "├─"
        if l["HeadID"] == c:
            tree.append(f"{branch}[{rn(l['LinkID'])}]──> {ln(l['TailID'])}")
        else:
            tree.append(f"{branch}<──[{rn(l['LinkID'])}]─ {ln(l['HeadID'])}")
    return one, tree


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("dataset", help="e.g. nell, dbpedia, testpedia")
    ap.add_argument("--list", help="a *queries.json file (default: query/<dataset>/queries.json)")
    ap.add_argument("--query-dir", help="folder holding the query folders (default: the --list file's folder)")
    ap.add_argument("--data-dir", default="dataset", help="default: dataset")
    ap.add_argument("--per-rho", type=int, default=1, help="contexts shown per rule, 0 = all (default 1)")
    ap.add_argument("--raw", action="store_true", help="also print the raw context JSON")
    ap.add_argument("--wildcard", type=int, default=10_000_000, help="IDs >= this are wildcards")
    args = ap.parse_args()

    qlist = Path(args.list or f"query/{args.dataset}/queries.json")
    qdir = Path(args.query_dir) if args.query_dir else qlist.parent
    ddir = Path(args.data_dir) / args.dataset
    labels = load_names(ddir / f"{args.dataset}_label.json")
    link_names = load_names(ddir / f"{args.dataset}_link.json")
    W = args.wildcard

    names = json.loads(qlist.read_text())
    by_rho = {r: [] for r in RULES}
    unmatched = []
    for n in names:
        path = qdir / n / "context.json"
        try:
            ctx = json.loads(path.read_text())[0]
        except FileNotFoundError:
            print(f"warning: {path} not found, skipped")
            continue
        rho = classify(ctx, W)
        (by_rho[rho] if rho is not None else unmatched).append((n, ctx))

    print(f"dataset: {args.dataset}   list: {qlist}   ({len(names)} queries)")
    print(f"counts:  " + "  ".join(f"rho{r}={len(v)}" for r, v in by_rho.items())
          + (f"  unmatched={len(unmatched)}" if unmatched else ""))

    for rho, (pattern, _, _) in RULES.items():
        items = by_rho[rho]
        print(f"\n{'=' * 70}\nrho{rho}:  {pattern}    ({len(items)} contexts)\n{'=' * 70}")
        if not items:
            print("  (no context of this rule in the list)")
        shown = items if args.per_rho <= 0 else items[: args.per_rho]
        for n, ctx in shown:
            one, tree = render(ctx, W, labels, link_names)
            print(f"\n  {n}:  {one}")
            for line in tree:
                print(f"      {line}")
            if args.raw:
                print("      " + json.dumps(ctx))

    if unmatched:
        print(f"\n{'=' * 70}\nunmatched (not one of rho0..rho6)\n{'=' * 70}")
        for n, ctx in unmatched:
            print(f"  {n}:  {render(ctx, W, labels, link_names)[0]}")


if __name__ == "__main__":
    main()
