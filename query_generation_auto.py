#!/usr/bin/env python3
"""Generate random valid contexts for the rules rho0 .. rho6 (Figure 6.2), with low memory use.

For every rule, until --per-rule (default 10) valid contexts are collected:
  1. every blue node gets a label drawn uniformly from the label vocabulary,
  2. every blue edge gets a relation drawn uniformly from the relation vocabulary,
  3. white nodes and black edges stay wildcards,
  4. the context is discarded if it matches no entity in the graph.
rho5 follows the paper's arrangement: a label l' is fixed and the other node takes five labels
l1..l5 (five contexts per fixed l'), repeated until --per-rule contexts exist.

  rho0: L -r-> L        rho1: L -r-> *        rho2: L -*-> *
  rho3: L -r-> L, L -r-> L                    rho4: L -r-> *, L -*-> *
  rho5: * -*-> L, * -*-> L                    rho6: L -*-> *, L -*-> *
  (L = random label, r = random relation, * = wildcard; every rule is a star around node 0)

A context matches when some head entity has outgoing edges satisfying every branch (link equals
the blue relation, or anything; tail has the blue label, or anything), with DIFFERENT tail entities
for the two branches. That strict reading is my assumption about the matcher.

Memory: the entity file is streamed (never loaded as one JSON object). Only compact arrays of the
edges, one label tuple per entity, and small sets of label-level facts are kept in memory; no
per-(relation, label) entity sets and no index file are built. Validity is decided exactly:
  rho0, rho1, rho2, rho4, rho5, rho6 from precomputed label-level sets (one pass over the graph),
  rho3 by drawing candidates and verifying them in batches against the edge arrays.

Output, under <out>/<dataset>/ (default query/<dataset>/):
  <prefix>_pXX/context.json and target.json   one folder per accepted context (XX = 00..69,
                                              ordered rho0..rho6, so p00-p09 are rho0 and so on)
  <prefix>_queries.json                       the list of those folder names (this is what main's -q reads)
  <prefix>_contexts.json                      the random seed, the settings and all accepted contexts
Default prefix: generated. Existing list/contexts files are renamed to *.bak, never overwritten.

This script only samples. Run the experiments separately, e.g.
  QUERIES=generated_queries ./run_settings.sh <dataset>

usage:  python generate_queries.py <dataset> [--per-rule 10] [--seed 1] [--entity-file FILE]
            [--label-file FILE] [--link-file FILE]
        python generate_queries.py demo --demo        (toy graph, to see the output)
"""

import argparse
import csv
import json
import random
import time
from array import array
from collections import defaultdict
from pathlib import Path

# rule -> (head, [(link, tail), ...]); "L" = random label, "r" = random relation, "*" = wildcard
RULES = {
    0: ("L", [("r", "L")]),
    1: ("L", [("r", "*")]),
    2: ("L", [("*", "*")]),
    3: ("L", [("r", "L"), ("r", "L")]),
    4: ("L", [("r", "*"), ("*", "*")]),
    5: ("*", [("*", "L"), ("*", "L")]),
    6: ("L", [("*", "*"), ("*", "*")]),
}


# ------------------------------------------------------------------ input
def iter_entities(path, chunk_size=1 << 22):
    """Yield the objects of a huge top-level JSON array one by one (constant memory)."""
    dec = json.JSONDecoder()
    with open(path, encoding="utf-8") as f:
        buf, pos, eof = "", 0, False

        def more():
            nonlocal buf, pos, eof
            s = f.read(chunk_size)
            if not s:
                eof = True
            buf, pos = buf[pos:] + s, 0

        more()
        while True:
            while True:  # skip '[' ',' and whitespace
                while pos < len(buf) and buf[pos] in " \t\r\n,[":
                    pos += 1
                if pos < len(buf) or eof:
                    break
                more()
            if pos >= len(buf) or buf[pos] == "]":
                return
            while True:
                try:
                    obj, end = dec.raw_decode(buf, pos)
                    break
                except json.JSONDecodeError:
                    if eof:
                        raise
                    more()  # object cut by the chunk boundary
            yield obj
            pos = end


def find_entity_file(dataset_dir, entity_file):
    if entity_file:
        return Path(entity_file)
    cands = [p for p in Path(dataset_dir).rglob("*.json") if "entity" in p.name.lower()]
    if len(cands) != 1:
        raise SystemExit(
            f"could not pick the entity file in {dataset_dir} "
            f"(candidates: {[str(c) for c in cands]}); pass --entity-file"
        )
    return cands[0]


def demo_entities(seed=0):
    rng = random.Random(seed)
    n_ent, n_lab, n_link = 200, 25, 8
    rel = defaultdict(list)
    for _ in range(900):
        h, t = rng.sample(range(1, n_ent + 1), 2)
        rel[h].append({"Link": rng.randint(1, n_link), "Tail": t})
    for e in range(1, n_ent + 1):
        yield {
            "ID": e,
            "Name": f"e{e}",
            "Label": rng.sample(range(1, n_lab + 1), rng.randint(1, 3)),
            "Relation": rel.get(e, []),
        }


# ------------------------------------------------------------------ graph
class Graph:
    """Compact graph: one (shared) label tuple per entity and edges in flat arrays."""

    def __init__(self, entities):
        self.labels_of, canon = {}, {}
        self.heads, self.off = array("Q"), array("Q", [0])
        self.e_link, self.e_tail = array("q"), array("Q")
        for e in entities:
            lab = tuple(e.get("Label", ()))
            lab = canon.setdefault(lab, lab)  # identical label tuples are stored once
            self.labels_of[e["ID"]] = lab
            rel = list(
                dict.fromkeys((r["Link"], r["Tail"]) for r in e.get("Relation", ()))
            )
            if rel:
                self.heads.append(e["ID"])
                for r, t in rel:
                    self.e_link.append(r)
                    self.e_tail.append(t)
                self.off.append(len(self.e_link))
        self.label_vocab = sorted({l for lab in canon for l in lab})
        self.link_vocab = sorted(set(self.e_link))

    def iter_heads(self):
        for i, h in enumerate(self.heads):
            a, b = self.off[i], self.off[i + 1]
            yield h, self.labels_of[h], list(zip(self.e_link[a:b], self.e_tail[a:b]))

    def matches(self, edges, branches):
        """Do the edges of one head satisfy all branches, with different tails for two branches?"""
        tails = []
        for r, lt in branches:
            t = {
                t
                for rr, t in edges
                if (r is None or rr == r)
                and (lt is None or lt in self.labels_of.get(t, ()))
            }
            if not t:
                return False
            tails.append(t)
        return len(tails) < 2 or not (len(tails[0]) == 1 and tails[0] == tails[1])


def count_matches(g, contexts):
    """One pass over the graph: number of matching head entities per context."""
    by_hl, any_head = defaultdict(list), []
    for k, (hl, _) in enumerate(contexts):
        (any_head if hl is None else by_hl[hl]).append(k)
    counts = [0] * len(contexts)
    for h, lh, edges in g.iter_heads():
        ks = list(any_head)
        for l in lh:
            ks += by_hl.get(l, ())
        for k in ks:
            if g.matches(edges, contexts[k][1]):
                counts[k] += 1
    return counts


class Facts:
    """Small label-level sets, computed in one pass, that decide validity exactly."""

    def __init__(self, g):
        self.KL, self.KR = max(g.label_vocab) + 1, max(g.link_vocab) + 1
        KL, KR = self.KL, self.KR
        self.T = set()  # (head label, link, tail label)         exists          -> rho0, rho3 prefilter
        self.Tr = (
            set()
        )  # (head label, link)                     exists          -> rho1
        self.T4 = (
            set()
        )  # (head label, link), head has >= 2 distinct tails       -> rho4
        self.Lany = (
            set()
        )  # head label, head has >= 1 edge                         -> rho2
        self.L2 = (
            set()
        )  # head label, head has >= 2 distinct tails               -> rho6
        self.P5 = (
            set()
        )  # (a <= b) tail labels usable with two different tails   -> rho5
        for h, lh, edges in g.iter_heads():
            tails = {t for _, t in edges}
            links = {r for r, _ in edges}
            rl, lab_tails = set(), defaultdict(set)
            for r, t in edges:
                for lt in g.labels_of.get(t, ()):
                    rl.add((r, lt))
                    lab_tails[lt].add(t)
            multi = len(tails) >= 2
            for l in lh:
                self.Lany.add(l)
                if multi:
                    self.L2.add(l)
                for r in links:
                    self.Tr.add(l * KR + r)
                    if multi:
                        self.T4.add(l * KR + r)
                for r, lt in rl:
                    self.T.add((l * KR + r) * KL + lt)
            if multi:
                labs = sorted(lab_tails)
                for i, a in enumerate(labs):
                    A = lab_tails[a]
                    for b in labs[i:]:
                        key = a * KL + b
                        if key in self.P5:
                            continue
                        B = lab_tails[b]
                        if (a == b and len(A) >= 2) or (
                            a != b and not (len(A) == 1 and A == B)
                        ):
                            self.P5.add(key)

    def t3(self, hl, r, lt):
        return (hl * self.KR + r) * self.KL + lt in self.T

    def pair(self, a, b):
        return min(a, b) * self.KL + max(a, b) in self.P5

    def valid_exact(self, rule, hl, br):
        """Exact validity for every rule except rho3 (and rho5, handled by pair())."""
        if rule == 0:
            return self.t3(hl, br[0][0], br[0][1])
        if rule == 1:
            return hl * self.KR + br[0][0] in self.Tr
        if rule == 2:
            return hl in self.Lany
        if rule == 4:
            return hl * self.KR + br[0][0] in self.T4
        if rule == 6:
            return hl in self.L2
        raise ValueError(rule)


# --------------------------------------------------------------- sampling
def draw(rule, g, rng):
    head_t, br_t = RULES[rule]
    hl = rng.choice(g.label_vocab) if head_t == "L" else None
    branches = [
        (
            rng.choice(g.link_vocab) if lt == "r" else None,
            rng.choice(g.label_vocab) if tl == "L" else None,
        )
        for lt, tl in br_t
    ]
    return hl, branches


def signature(hl, branches):
    # swapping the two branches gives the same context, so sort for the duplicate check
    return hl, tuple(
        sorted(
            ((-1 if r is None else r), (-1 if l is None else l)) for r, l in branches
        )
    )


def sample_rule(rule, g, facts, rng, per_rule, max_attempts):
    out, seen, attempts = [], set(), 0
    while len(out) < per_rule and attempts < max_attempts:
        attempts += 1
        hl, branches = draw(rule, g, rng)
        sig = signature(hl, branches)
        if sig in seen or not facts.valid_exact(rule, hl, branches):
            continue
        seen.add(sig)
        out.append((hl, branches, {}))
    return out, attempts


def sample_rho3(g, facts, rng, per_rule, max_attempts, batch):
    """Candidates pass a cheap per-branch filter, then are verified in batches (one pass each).
    Contexts are accepted in draw order, exactly as in plain rejection sampling."""
    out, seen, attempts = [], set(), 0
    while len(out) < per_rule and attempts < max_attempts:
        cands = []
        while len(cands) < batch and attempts < max_attempts:
            attempts += 1
            hl, branches = draw(3, g, rng)
            sig = signature(hl, branches)
            if sig in seen:
                continue
            seen.add(sig)
            if all(facts.t3(hl, r, lt) for r, lt in branches):
                cands.append((hl, branches))
        if not cands:
            break
        for (hl, branches), c in zip(cands, count_matches(g, cands)):
            if c > 0 and len(out) < per_rule:
                out.append((hl, branches, {}))
    return out, attempts


def sample_rho5(g, facts, rng, per_rule, max_attempts):
    """Fix l', take five partner labels for which (l', l_i) is valid; repeat for the next group."""
    out, seen, attempts, group = [], set(), 0, 0
    while len(out) < per_rule and attempts < max_attempts:
        attempts += 1
        lp = rng.choice(g.label_vocab)
        partners = [
            l
            for l in g.label_vocab
            if tuple(sorted((lp, l))) not in seen and facts.pair(lp, l)
        ]
        if len(partners) < 5:
            continue
        group += 1
        for l in rng.sample(partners, 5):
            seen.add(tuple(sorted((lp, l))))
            out.append(
                (None, [(None, lp), (None, l)], {"group": group, "fixed_label": lp})
            )
    return out[:per_rule], attempts


# ----------------------------------------------------------------- output
def load_names(path):
    """Metadata file: a JSON array of {"ID": 1, "Name": "LiveIn"} -> {id: name}."""
    try:
        with open(path, encoding="utf-8") as f:
            return {e["ID"]: e.get("Name", str(e["ID"])) for e in json.load(f)}
    except (KeyError, TypeError):
        raise SystemExit(
            f'{path}: expected a JSON array of {{"ID": ..., "Name": ...}} objects'
        )


def backup(path):
    if path.exists():
        bak, n = path.with_name(path.name + ".bak"), 1
        while bak.exists():
            bak, n = path.with_name(f"{path.name}.bak{n}"), n + 1
        path.rename(bak)
        print(f"existing {path.name} renamed to {bak.name}")


def describe(hl, branches, label_names, link_names):
    ln = lambda x: "*" if x is None else label_names.get(x, str(x))
    rn = lambda x: "*" if x is None else link_names.get(x, str(x))
    return "; ".join(f"{ln(hl)} -[{rn(r)}]-> {ln(l)}" for r, l in branches)


def write_queries(args, results, counts, label_names, link_names):
    W = args.wildcard
    root = Path(args.out) / args.dataset
    total = sum(len(v) for v in results.values())
    width = max(2, len(str(total - 1)))
    records, k = [], 0
    for rule in range(7):
        for hl, branches, extra in results[rule]:
            name = f"{args.prefix}_p{k:0{width}d}"
            ents = [{"EntityID": W if hl is None else hl, "EntityKind": True}]
            ents += [
                {"EntityID": W if l is None else l, "EntityKind": True}
                for _, l in branches
            ]
            links = [
                {"HeadID": 0, "LinkID": W if r is None else r, "TailID": j + 1}
                for j, (r, _) in enumerate(branches)
            ]
            ctx = [{"Entity": ents, "Link": links}]
            tgt = [
                {
                    "HeadID": W,
                    "HeadKind": True,
                    "LinkID": W,
                    "TailID": W,
                    "TailKind": True,
                }
            ]
            d = root / name
            d.mkdir(parents=True, exist_ok=True)
            (d / "context.json").write_text(json.dumps(ctx, indent=2))
            (d / "target.json").write_text(json.dumps(tgt, indent=2))
            rec = {
                "name": name,
                "rule": rule,
                "readable": describe(hl, branches, label_names, link_names),
                "matching_heads": None if counts is None else counts[k],
                "context": ctx,
                "target": tgt,
            }
            if extra:
                rec["rho5_group"], rec["rho5_fixed_label"] = (
                    extra["group"],
                    extra["fixed_label"],
                )
            records.append(rec)
            k += 1
    lst = root / f"{args.prefix}_queries.json"
    backup(lst)
    lst.write_text(json.dumps([r["name"] for r in records], indent=2))
    meta = {
        "seed": args.seed,
        "dataset": args.dataset,
        "source": args.source,
        "per_rule": args.per_rule,
        "wildcard": args.wildcard,
        "max_attempts": args.max_attempts,
        "queries": records,
    }
    cj = root / f"{args.prefix}_contexts.json"
    backup(cj)
    cj.write_text(json.dumps(meta, indent=1))
    print(
        f"wrote {len(records)} query folders to {root}/, the list {lst.name} and {cj.name} (seed {args.seed})"
    )


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter
    )
    ap.add_argument("dataset")
    ap.add_argument("--dataset-dir", help="default: dataset/<dataset>")
    ap.add_argument(
        "--entity-file",
        help="entity JSON file (default: the only *entity*.json in the dataset dir)",
    )
    ap.add_argument("--out", default="query", help="default: query")
    ap.add_argument("--per-rule", type=int, default=10)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument(
        "--wildcard",
        type=int,
        default=10_000_000,
        help="must be >= the 'RG wildcard' value printed by main (default 10000000)",
    )
    ap.add_argument(
        "--prefix",
        default="generated",
        help="folders are <prefix>_pXX, the list <prefix>_queries.json (default: generated)",
    )
    ap.add_argument(
        "--label-file", help='optional label metadata: JSON array of {"ID", "Name"}'
    )
    ap.add_argument(
        "--link-file", help='optional relation metadata: JSON array of {"ID", "Name"}'
    )
    ap.add_argument(
        "--max-attempts",
        type=int,
        default=3_000_000,
        help="random draws per rule before giving up",
    )
    ap.add_argument(
        "--batch", type=int, default=200, help="rho3 candidates verified per graph pass"
    )
    ap.add_argument(
        "--no-count",
        action="store_true",
        help="skip the final pass that counts matching heads (and double-checks every context)",
    )
    ap.add_argument(
        "--demo", action="store_true", help="use a toy graph instead of the dataset"
    )
    args = ap.parse_args()

    t0 = time.time()
    if args.demo:
        args.source = "demo"
        g = Graph(demo_entities())
    else:
        path = find_entity_file(
            args.dataset_dir or f"dataset/{args.dataset}", args.entity_file
        )
        args.source = str(path)
        print(f"reading {path} (streaming)")
        g = Graph(iter_entities(path))
    print(
        f"graph: {len(g.labels_of)} entities, {len(g.e_link)} edges, {len(g.label_vocab)} labels, "
        f"{len(g.link_vocab)} relations ({time.time() - t0:.0f}s)"
    )
    label_names = load_names(args.label_file) if args.label_file else {}
    link_names = load_names(args.link_file) if args.link_file else {}
    g.label_vocab = sorted(
        set(g.label_vocab) | set(label_names)
    )  # vocabulary from the metadata files, if given
    g.link_vocab = sorted(set(g.link_vocab) | set(link_names))
    t0 = time.time()
    facts = Facts(g)
    print(
        f"label-level facts: {len(facts.T)} (head label, link, tail label) triples ({time.time() - t0:.0f}s)"
    )

    results = {}
    for rule in range(7):
        rng = random.Random(f"{args.seed}-{rule}")  # one stream per rule
        if rule == 3:
            res, attempts = sample_rho3(
                g, facts, rng, args.per_rule, args.max_attempts, args.batch
            )
        elif rule == 5:
            res, attempts = sample_rho5(
                g, facts, rng, args.per_rule, min(args.max_attempts, 20000)
            )
        else:
            res, attempts = sample_rule(
                rule, g, facts, rng, args.per_rule, args.max_attempts
            )
        results[rule] = res
        print(f"rho{rule}: {len(res)}/{args.per_rule} valid contexts, {attempts} draws")
        if len(res) < args.per_rule:
            print(
                f"  WARNING: fewer than {args.per_rule} found; the per-rule grouping in "
                f"aggregate_results.py will not line up (raise --max-attempts or lower --per-rule)"
            )

    counts = None
    if not args.no_count:
        t0 = time.time()
        flat = [(hl, br) for rule in range(7) for hl, br, _ in results[rule]]
        counts = count_matches(g, flat)
        print(f"counted matching heads ({time.time() - t0:.0f}s)")
        for (hl, br), c in zip(flat, counts):
            if c == 0:
                print(
                    f"  WARNING: context {(hl, br)} matches nothing, this is a bug; please report it"
                )
    write_queries(args, results, counts, label_names, link_names)


if __name__ == "__main__":
    main()
