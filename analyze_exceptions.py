#!/usr/bin/env python3
"""Analyse the exceptions found by main, per setting (dataset, s, o) and across settings.

Reads what aggregate_results.py wrote (default results/<dataset>/agg/):
  <dataset>_<s>_o<o>_exceptions.csv   required   query, rule, rank, score, type, pattern, pe, triple
  <dataset>_<s>_o<o>_queries.csv      optional   run time and searched patterns per query
and optionally the generated_contexts.json of the query generator (readable context, matching heads).

Writes to --out (default results/<dataset>/analysis/):
  exceptions_report.md                the overview, one section per setting plus the comparisons
  <prefix>_per_query.csv, _per_rule.csv, _links.csv, _head_labels.csv, _tail_labels.csv,
  _entities.csv, _top_exceptions.csv,
  _top_per_context.csv                the tables behind the report (the last one: the strongest fact of every context)
  comparison.csv                      overlap of the top exceptions between settings (dist/freq, o0/o1)
  *.png                               figures, if matplotlib is installed

The printed triples contain entity names only. Labels of the head and tail entities are looked up in the
dataset (--entity-file, optionally --label-file for names instead of IDs), otherwise that section is skipped.

"Unique facts": one fact is often printed more than once. Two merge steps are applied, and the report
shows how many exceptions each step removes:
  1. direction: a fact can be reported from both ends of an edge, as "A X-> B" (pattern node of A) and
     as "B X<- A" (pattern node of B). Both are rewritten as (subject, link, object).
     This needs no inverse relations in the dataset.
  2. inverse relations (NELL only): "A inverseofX<- B" and "A X-> B" are the same fact. The prefix is
     set with --inverse-prefix; with no relation starting with it, this step changes nothing.
--merge chooses which level the tables use (default: inverse, i.e. both steps; 'none' keeps every
printed triple). When both ends of an edge are reported, their scores can differ, because each end
looks at the edge from its own side: merged facts keep the larger score. --show-merged N prints
examples of merged facts with every printed form, and reports how often their scores differ.

usage:  python analyze_exceptions.py results/dbpedia/agg --per-rule 10
            [--contexts query/dbpedia/generated_contexts.json] [--top 15] [--out DIR]
            [--merge inverse|direction|none] [--show-merged 10]
            [--entity-file dataset/dbpedia/<entity file>.json --label-file <label file>.json]
"""

import argparse
import csv
import json
import re
import statistics as st
from collections import Counter, defaultdict
from pathlib import Path

PREFIX_RE = re.compile(r"(?P<ds>.+)_(?P<s>dist|freq)_o(?P<o>\d)$")
TRIPLE_RE = re.compile(r"^(?P<h>\S+)\s+(?P<l>\S+?)(?P<d>->|<-)\s+(?P<t>\S+)$")


# ---------------------------------------------------------------- helpers
def pct(xs, p):
    """Percentile of a sorted list, linear interpolation."""
    if not xs:
        return float("nan")
    k = (len(xs) - 1) * p / 100
    f = int(k)
    c = min(f + 1, len(xs) - 1)
    return xs[f] + (xs[c] - xs[f]) * (k - f)


def fmt(x):
    return (
        "-" if x is None or x != x else (f"{x:.4g}" if isinstance(x, float) else str(x))
    )


def md_table(headers, rows):
    out = ["| " + " | ".join(headers) + " |", "|" + "---|" * len(headers)]
    out += ["| " + " | ".join(fmt(c) for c in r) + " |" for r in rows]
    return "\n".join(out) + "\n"


def write_csv(path, rows, fields=None):
    if not rows:
        return
    fields = fields or list(dict.fromkeys(k for r in rows for k in r))
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields, restval="")
        w.writeheader()
        w.writerows(rows)


def resolve(text, inverse_prefix):
    """One printed triple -> (as printed, direction resolved, inverse merged).

    printed:           (head, 'X->' or 'X<-', tail)
    direction resolved: (subject, X, object); 'A X-> B' and 'B X<- A' both become (A, X, B)
    inverse merged:     additionally 'A inverseofX<- B' becomes (A, X, B) (only with the prefix)"""
    m = TRIPLE_RE.match(text.strip())
    if not m:
        raw = ("?", text.strip(), "?")
        return raw, raw, raw
    h, l, d, t = m["h"], m["l"], m["d"], m["t"]
    s, o = (h, t) if d == "->" else (t, h)
    direction = (s, l, o)
    if inverse_prefix and l.startswith(inverse_prefix) and len(l) > len(inverse_prefix):
        inv = (o, l[len(inverse_prefix) :], s)
    else:
        inv = direction
    return (h, l + d, t), direction, inv


def load_names(path):
    """Metadata file: a JSON array of {"ID": 1, "Name": "Person"} -> {id: name}."""
    return {
        e["ID"]: e.get("Name", str(e["ID"]))
        for e in json.loads(Path(path).read_text(encoding="utf-8"))
    }


def lookup_labels(entity_file, settings):
    """Labels of the entities that occur in the exceptions: one streaming pass over the dataset's entity file."""
    needed = {
        n
        for data in settings.values()
        for fq in data["facts"].values()
        for (s, _, o) in fq
        for n in (s, o)
    }
    try:
        from generate_queries import iter_entities  # the streaming reader, same folder
    except ImportError:
        raise SystemExit(
            "generate_queries.py must be in the same folder (it provides the streaming reader)"
        )
    print(
        f"looking up the labels of {len(needed)} entities in {entity_file} (one pass)"
    )
    found = {}
    for e in iter_entities(entity_file):
        name = e.get("Name")
        if name in needed:
            found[name] = tuple(
                sorted(set(found.get(name, ())) | set(e.get("Label", ())))
            )
    print(f"  found {len(found)} of {len(needed)}")
    return found


def fact_text(fact):
    return f"{fact[0]} -[{fact[1]}]-> {fact[2]}"


# ---------------------------------------------------------------- loading
def load_setting(path, inverse_prefix, merge, keep_examples):
    pick = {"none": 0, "direction": 1, "inverse": 2}[merge]
    facts, scores, types = defaultdict(dict), defaultdict(list), defaultdict(Counter)
    seen = [
        defaultdict(set) for _ in range(3)
    ]  # distinct facts per level (hashes, to save memory)
    examples = defaultdict(list) if keep_examples else None
    with open(path, newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            q, sc, ty = int(row["query"]), float(row["score"]), row["type"]
            scores[q].append(sc)
            types[q][ty] += 1
            forms = resolve(row["triple"], inverse_prefix)
            for lvl in range(3):
                seen[lvl][q].add(hash(forms[lvl]))
            fact = forms[pick]
            cur = facts[q].get(fact)
            if cur is None:
                facts[q][fact] = [sc, {ty}]
            else:
                cur[0] = max(cur[0], sc)
                cur[1].add(ty)
            if examples is not None and len(examples[(q, fact)]) < 6:
                examples[(q, fact)].append((row["triple"].strip(), row["pe"], ty, sc))
    levels = [sum(len(v) for v in lvl.values()) for lvl in seen]
    return {
        "facts": facts,
        "scores": scores,
        "types": types,
        "levels": levels,
        "examples": examples,
        "merge": merge,
    }


def merged_examples(prefix, data, n, out):
    """Examples of facts that were printed in several forms, and how often their scores differ."""
    rows, groups, differ = [], 0, 0
    for (q, fact), ex in sorted(data["examples"].items(), key=lambda kv: kv[0][0]):
        if len({e[0] for e in ex}) < 2:
            continue
        groups += 1
        differ += len({e[3] for e in ex}) > 1
        for raw, pe, ty, sc in ex:
            rows.append(
                {
                    "query": q,
                    "fact": fact_text(fact),
                    "printed_as": raw,
                    "pe": pe,
                    "type": ty,
                    "score": sc,
                }
            )
    write_csv(out / f"{prefix}_merged_examples.csv", rows)
    shown = 0
    print(f"--- {prefix}: examples of merged facts ---")
    for fact in dict.fromkeys((r["query"], r["fact"]) for r in rows):
        if shown >= n:
            break
        shown += 1
        print(f"query p{fact[0]:02d}: {fact[1]}")
        for r in rows:
            if (r["query"], r["fact"]) == fact:
                print(
                    f"    printed as: {r['printed_as']}   [pe-{r['pe']}, {r['type']}, score {r['score']}]"
                )
    return groups, differ


def load_queries_csv(path):
    info = {}
    if path.exists():
        with open(path, newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                q = int(row["query"].lstrip("p") or 0)
                info[q] = {
                    "time": float(row["sum_time"]) if row.get("sum_time") else None,
                    "patterns": int(float(row["patterns_total"]))
                    if row.get("patterns_total")
                    else None,
                }
    return info


def load_contexts(path):
    if not path:
        return {}
    meta = json.loads(Path(path).read_text())
    return {
        i: {"readable": q.get("readable", ""), "heads": q.get("matching_heads")}
        for i, q in enumerate(meta.get("queries", []))
    }


# --------------------------------------------------------------- analysis
def analyse(
    prefix, data, qinfo, ctx, per_rule, top, out, name_labels=None, label_names=None
):
    facts, scores, types = data["facts"], data["scores"], data["types"]
    queries = sorted(set(qinfo) | set(ctx) | set(scores))
    rule_of = lambda q: q // per_rule

    # --- per query
    per_query = []
    for q in queries:
        sc = sorted(scores.get(q, []), reverse=True)
        fq = facts.get(q, {})
        best = max(fq.items(), key=lambda kv: kv[1][0], default=None)
        ties = Counter(sc)
        per_query.append(
            {
                "query": q,
                "rule": rule_of(q),
                "context": ctx.get(q, {}).get("readable", ""),
                "matching_heads": ctx.get(q, {}).get("heads"),
                "time_s": qinfo.get(q, {}).get("time"),
                "patterns_searched": qinfo.get(q, {}).get("patterns"),
                "exceptions": len(sc),
                "unique_facts": len(fq),
                "label_exc": types[q]["LabelException"] if q in types else 0,
                "link_exc": types[q]["LinkException"] if q in types else 0,
                "max_score": sc[0] if sc else None,
                "median_score": st.median(sc) if sc else None,
                "tied_share": (sum(c for c in ties.values() if c > 1) / len(sc))
                if sc
                else None,
                "top_fact": fact_text(best[0]) if best else "",
                "top_type": "/".join(sorted(best[1][1])) if best else "",
                "top_fact_ties": sum(1 for v in fq.values() if v[0] == best[1][0])
                if best
                else 0,
            }
        )
    write_csv(out / f"{prefix}_per_query.csv", per_query)

    # --- per rule
    per_rule_rows = []
    for r in sorted({rule_of(q) for q in queries}):
        qs = [q for q in queries if rule_of(q) == r]
        allsc = sorted(s for q in qs for s in scores.get(q, []))
        n_exc = [len(scores.get(q, [])) for q in qs]
        lab = sum(types[q]["LabelException"] for q in qs if q in types)
        lin = sum(types[q]["LinkException"] for q in qs if q in types)
        uniq = sum(len(facts.get(q, {})) for q in qs)
        per_rule_rows.append(
            {
                "rule": r,
                "queries": len(qs),
                "queries_with_exceptions": sum(1 for n in n_exc if n),
                "exceptions": sum(n_exc),
                "unique_facts": uniq,
                "merged_share": 1 - uniq / sum(n_exc) if sum(n_exc) else None,
                "exc_per_query_median": st.median(n_exc),
                "exc_per_query_max": max(n_exc),
                "score_median": pct(allsc, 50),
                "score_p90": pct(allsc, 90),
                "score_p99": pct(allsc, 99),
                "score_max": allsc[-1] if allsc else None,
                "label_exc_share": lab / (lab + lin) if lab + lin else None,
            }
        )
    write_csv(out / f"{prefix}_per_rule.csv", per_rule_rows)

    # --- links, labels, entities (unique facts)
    links, hlabels, tlabels = {}, Counter(), Counter()
    ent_queries, ent_best = defaultdict(set), {}
    for q, fq in facts.items():
        for (s, l, o), (sc, _) in fq.items():
            c = links.setdefault(l, [0, 0.0, set()])
            c[0] += 1
            c[1] += sc
            c[2].add(q)
            for node in (s, o):  # entity = the printed name, as it is
                ent_queries[node].add(q)
                ent_best[node] = max(ent_best.get(node, 0), sc)
            if name_labels is not None:  # labels come from the dataset, not the triple
                for counter, node in ((hlabels, s), (tlabels, o)):
                    labs = name_labels.get(node)
                    if labs is None:
                        counter["(entity not found in dataset)"] += 1
                    elif not labs:
                        counter["(entity has no label)"] += 1
                    for lab in labs or ():
                        counter[(label_names or {}).get(lab, str(lab))] += 1
    total = sum(c[0] for c in links.values()) or 1
    link_rows = sorted(
        (
            {
                "link": l,
                "unique_facts": c[0],
                "share": c[0] / total,
                "mean_score": c[1] / c[0],
                "queries": len(c[2]),
            }
            for l, c in links.items()
        ),
        key=lambda r: -r["unique_facts"],
    )
    write_csv(out / f"{prefix}_links.csv", link_rows)
    if name_labels is not None:
        write_csv(
            out / f"{prefix}_head_labels.csv",
            [{"head_label": k, "unique_facts": v} for k, v in hlabels.most_common()],
        )
        write_csv(
            out / f"{prefix}_tail_labels.csv",
            [{"tail_label": k, "unique_facts": v} for k, v in tlabels.most_common()],
        )
    ent_rows = sorted(
        (
            {"entity": e, "queries": len(qs), "max_score": ent_best[e]}
            for e, qs in ent_queries.items()
        ),
        key=lambda r: (-r["queries"], -r["max_score"]),
    )
    write_csv(out / f"{prefix}_entities.csv", ent_rows[:5000])

    # --- top exceptions overall
    best = {}
    for q, fq in facts.items():
        for fact, (sc, ty) in fq.items():
            if fact not in best or sc > best[fact][0]:
                best[fact] = (sc, q, ty)
    top_rows = [
        {
            "score": sc,
            "fact": fact_text(f),
            "types": "/".join(sorted(ty)),
            "query": q,
            "rule": rule_of(q),
            "context": ctx.get(q, {}).get("readable", ""),
        }
        for f, (sc, q, ty) in sorted(best.items(), key=lambda kv: -kv[1][0])[
            : max(top, 100)
        ]
    ]
    write_csv(out / f"{prefix}_top_exceptions.csv", top_rows)

    # --- one row per context: its single strongest unique fact, contexts ranked by that score
    by_ctx = {}
    for r in per_query:
        if r["max_score"] is None:
            continue
        key = (
            r["context"] or f"query p{r['query']:02d}"
        )  # the same context text counts once
        if key not in by_ctx or r["max_score"] > by_ctx[key]["max_score"]:
            by_ctx[key] = r
    ctx_rows = sorted(by_ctx.values(), key=lambda r: -r["max_score"])
    write_csv(
        out / f"{prefix}_top_per_context.csv",
        [
            {
                "rank": i,
                "score": r["max_score"],
                "context": r["context"],
                "query": r["query"],
                "rule": r["rule"],
                "strongest_fact": r["top_fact"],
                "type": r["top_type"],
                "tied_facts": r["top_fact_ties"],
                "exceptions_in_context": r["exceptions"],
                "unique_facts_in_context": r["unique_facts"],
            }
            for i, r in enumerate(ctx_rows, 1)
        ],
    )

    # --- markdown
    n_exc = sum(len(v) for v in scores.values())
    n_uniq = sum(len(v) for v in facts.values())
    allsc = sorted(s for v in scores.values() for s in v)
    ntype = Counter()
    for c in types.values():
        ntype.update(c)
    md = [
        f"## {prefix}",
        "",
        f"- queries: {len(queries)}, with at least one exception: {sum(1 for q in queries if scores.get(q))}",
        f"- exceptions: {n_exc}",
        f"- distinct triples as printed: {data['levels'][0]}",
        f"- after resolving direction (the same fact reported from both ends of an edge, "
        f"'A X-> B' and 'B X<- A'): {data['levels'][1]}",
        f"- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): "
        f"{data['levels'][2]}",
        f"- unique facts used in the tables below (--merge {data['merge']}): {n_uniq} "
        f"({fmt(1 - n_uniq / n_exc if n_exc else None)} of the exceptions merged into another one)",
        f"- types: LabelException {ntype['LabelException']}, LinkException {ntype['LinkException']}",
        f"- score: median {fmt(pct(allsc, 50))}, p90 {fmt(pct(allsc, 90))}, p99 {fmt(pct(allsc, 99))}, "
        f"max {fmt(allsc[-1] if allsc else None)}",
        "",
        "### Per rule",
        "",
        md_table(
            [
                "rule",
                "queries",
                "with exc.",
                "exceptions",
                "unique",
                "merged share",
                "exc./query (median, max)",
                "score median",
                "p90",
                "p99",
                "max",
                "Label share",
            ],
            [
                [
                    f"ρ{r['rule']}",
                    r["queries"],
                    r["queries_with_exceptions"],
                    r["exceptions"],
                    r["unique_facts"],
                    r["merged_share"],
                    f"{r['exc_per_query_median']:.0f}, {r['exc_per_query_max']}",
                    r["score_median"],
                    r["score_p90"],
                    r["score_p99"],
                    r["score_max"],
                    r["label_exc_share"],
                ]
                for r in per_rule_rows
            ],
        ),
        f"### Top {top} exceptions (unique facts)",
        "",
        md_table(
            ["score", "fact", "type", "query", "context of the query"],
            [
                [r["score"], r["fact"], r["types"], f"p{r['query']:02d}", r["context"]]
                for r in top_rows[:top]
            ],
        ),
        f"### Top {top} contexts, each with its strongest exception (one fact per context)",
        "",
        md_table(
            [
                "#",
                "score",
                "context",
                "query",
                "strongest fact",
                "type",
                "tied",
                "exceptions in context",
            ],
            [
                [
                    i,
                    r["max_score"],
                    r["context"] or "-",
                    f"p{r['query']:02d} (ρ{r['rule']})",
                    r["top_fact"],
                    r["top_type"],
                    f"+{r['top_fact_ties'] - 1}" if r["top_fact_ties"] > 1 else "",
                    r["exceptions"],
                ]
                for i, r in enumerate(ctx_rows[:top], 1)
            ],
        ),
        f"### Most frequent links (unique facts)",
        "",
        md_table(
            ["link", "facts", "share", "mean score", "queries"],
            [
                [
                    r["link"],
                    r["unique_facts"],
                    r["share"],
                    r["mean_score"],
                    r["queries"],
                ]
                for r in link_rows[:top]
            ],
        ),
        "### Most frequent head and tail labels",
        "",
        (
            "The printed triples contain entity names only, never labels. Pass --entity-file (and --label-file) to "
            "look the labels up in the dataset.\n"
            if name_labels is None
            else "Labels of the head and tail entity of the unique facts, looked up in the dataset "
            "(an entity with several labels counts for each of them).\n\n"
            + md_table(
                ["head label", "facts", "tail label", "facts"],
                [
                    [a[0], a[1], b[0], b[1]]
                    for a, b in zip(hlabels.most_common(top), tlabels.most_common(top))
                ],
            )
        ),
        "### Entities in exceptions of the most queries",
        "",
        md_table(
            ["entity", "queries", "max score"],
            [[r["entity"], r["queries"], r["max_score"]] for r in ent_rows[:top]],
        ),
        "### Queries with the most and the strongest exceptions",
        "",
        md_table(
            [
                "query",
                "rule",
                "context",
                "matching heads",
                "exceptions",
                "max score",
                "time [s]",
            ],
            [
                [
                    f"p{r['query']:02d}",
                    f"ρ{r['rule']}",
                    r["context"],
                    r["matching_heads"],
                    r["exceptions"],
                    r["max_score"],
                    r["time_s"],
                ]
                for r in sorted(per_query, key=lambda r: -r["exceptions"])[:top]
            ],
        ),
    ]
    return "\n".join(md), per_query


def topk(fq, k):
    items = sorted(fq.items(), key=lambda kv: -kv[1][0])
    if not items:
        return set()
    cutoff = items[min(k, len(items)) - 1][1][0]
    return {
        f for f, (sc, _) in items if sc >= cutoff
    }  # ties at the cut-off are all included


def compare(pairs, settings, per_rule, out):
    rows, md = [], []
    for a, b, label in pairs:
        fa, fb = settings[a]["facts"], settings[b]["facts"]
        for k in (10, 100):
            by_rule = defaultdict(list)
            same_top = defaultdict(list)
            for q in sorted(set(fa) & set(fb)):
                A, B = topk(fa[q], k), topk(fb[q], k)
                if A and B:
                    by_rule[q // per_rule].append(len(A & B) / len(A | B))
                    if k == 10:
                        ta = max(fa[q].items(), key=lambda kv: kv[1][0])[1][0]
                        tb = max(fb[q].items(), key=lambda kv: kv[1][0])[1][0]
                        same_top[q // per_rule].append(
                            bool(
                                {f for f, v in fa[q].items() if v[0] == ta}
                                & {f for f, v in fb[q].items() if v[0] == tb}
                            )
                        )
            for r, js in sorted(by_rule.items()):
                rows.append(
                    {
                        "comparison": label,
                        "k": k,
                        "rule": r,
                        "queries": len(js),
                        "jaccard_median": st.median(js),
                        "jaccard_mean": st.mean(js),
                        "same_top1_share": (sum(same_top[r]) / len(same_top[r]))
                        if same_top.get(r)
                        else None,
                    }
                )
        part = [x for x in rows if x["comparison"] == label]
        md += [
            f"### {label}",
            "",
            md_table(
                [
                    "k",
                    "rule",
                    "queries",
                    "Jaccard median",
                    "Jaccard mean",
                    "same top-1",
                ],
                [
                    [
                        x["k"],
                        f"ρ{x['rule']}",
                        x["queries"],
                        x["jaccard_median"],
                        x["jaccard_mean"],
                        x["same_top1_share"],
                    ]
                    for x in part
                ],
            ),
        ]
    write_csv(out / "comparison.csv", rows)
    return "\n".join(md)


def make_plots(settings, per_queries, per_rule, out):
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed: no figures (the tables are still written)")
        return
    for prefix, data in settings.items():
        rules = sorted({q // per_rule for q in data["scores"]})
        groups = [
            [s for q, v in data["scores"].items() if q // per_rule == r for s in v]
            for r in rules
        ]
        fig, ax = plt.subplots(1, 2, figsize=(12, 4))
        ax[0].boxplot(groups, showfliers=False)
        ax[0].set_xticks(range(1, len(rules) + 1))
        ax[0].set_xticklabels([f"ρ{r}" for r in rules])
        ax[0].set_yscale("log")
        ax[0].set_title(f"{prefix}: exception score per rule")
        ax[0].set_ylabel("score (log)")
        pq = per_queries[prefix]
        ax[1].bar(
            [r["query"] for r in pq],
            [r["exceptions"] for r in pq],
            color=[f"C{r['rule'] % 10}" for r in pq],
        )
        ax[1].set_yscale("symlog")
        ax[1].set_title("exceptions per query (colour = rule)")
        ax[1].set_xlabel("query")
        fig.tight_layout()
        fig.savefig(out / f"{prefix}_overview.png", dpi=130)
        plt.close(fig)


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter
    )
    ap.add_argument(
        "agg_dir",
        help="folder with the *_exceptions.csv files written by aggregate_results.py",
    )
    ap.add_argument("--out", help="default: sibling folder 'analysis' of agg_dir")
    ap.add_argument("--per-rule", type=int, default=10)
    ap.add_argument(
        "--contexts",
        help="generated_contexts.json of the query generator (readable contexts)",
    )
    ap.add_argument(
        "--inverse-prefix",
        default="inverseof",
        help="prefix of inverse relations (default: inverseof)",
    )
    ap.add_argument("--top", type=int, default=15)
    ap.add_argument(
        "--entity-file",
        help="dataset entity JSON: look up the labels of the entities in the exceptions",
    )
    ap.add_argument(
        "--label-file",
        help='label metadata, a JSON array of {"ID", "Name"} (names instead of IDs)',
    )
    ap.add_argument(
        "--merge",
        choices=["inverse", "direction", "none"],
        default="inverse",
        help="which merge level defines a unique fact (default: inverse = direction + inverse relations)",
    )
    ap.add_argument(
        "--show-merged",
        type=int,
        default=0,
        metavar="N",
        help="print N examples of merged facts and write <prefix>_merged_examples.csv",
    )
    args = ap.parse_args()

    agg = Path(args.agg_dir)
    out = Path(args.out) if args.out else agg.parent / "analysis"
    out.mkdir(parents=True, exist_ok=True)
    ctx = load_contexts(args.contexts)

    settings, per_queries, sections = {}, {}, []
    for path in sorted(agg.glob("*_exceptions.csv")):
        prefix = path.name[: -len("_exceptions.csv")]
        if not PREFIX_RE.match(prefix):
            continue
        print(f"reading {path.name}")
        settings[prefix] = load_setting(
            path, args.inverse_prefix, args.merge, args.show_merged > 0
        )
    if not settings:
        raise SystemExit(f"no <dataset>_<s>_o<o>_exceptions.csv files in {agg}")
    name_labels = (
        lookup_labels(args.entity_file, settings) if args.entity_file else None
    )
    label_names = load_names(args.label_file) if args.label_file else {}
    for prefix in settings:
        md, pq = analyse(
            prefix,
            settings[prefix],
            load_queries_csv(agg / f"{prefix}_queries.csv"),
            ctx,
            args.per_rule,
            args.top,
            out,
            name_labels,
            label_names,
        )
        if args.show_merged > 0:
            groups, differ = merged_examples(
                prefix, settings[prefix], args.show_merged, out
            )
            md += (
                f"\n### Merged facts\n\n{groups} facts were printed in more than one form (up to 6 forms stored per "
                f"fact); in {differ} of them the forms have different scores. Examples: "
                f"`{prefix}_merged_examples.csv`.\n"
            )
        sections.append(md)
        per_queries[prefix] = pq
    if not settings:
        raise SystemExit(f"no <dataset>_<s>_o<o>_exceptions.csv files in {agg}")

    pairs = []
    for a in settings:
        for b in settings:
            ma, mb = PREFIX_RE.match(a), PREFIX_RE.match(b)
            if a < b and ma["ds"] == mb["ds"]:
                if ma["s"] == mb["s"] and ma["o"] != mb["o"]:
                    pairs.append((a, b, f"{ma['s']}: o={ma['o']} vs o={mb['o']}"))
                elif ma["o"] == mb["o"] and ma["s"] != mb["s"]:
                    pairs.append((a, b, f"o={ma['o']}: {ma['s']} vs {mb['s']}"))
    report = [
        "# Exception analysis",
        "",
        "Unique facts merge the same fact printed in several forms (counts per setting below). "
        "Scores of `dist` and `freq` are on different scales, so they are compared by rank overlap only.",
        "",
    ]
    report += sections
    if pairs:
        report += [
            "## Comparison between settings",
            "",
            "Jaccard overlap of the top-k unique facts per query (ties at the cut-off included); "
            "'same top-1' = the best exceptions of both runs share a fact.",
            "",
            compare(pairs, settings, args.per_rule, out),
        ]
    (out / "exceptions_report.md").write_text("\n".join(report), encoding="utf-8")
    make_plots(settings, per_queries, args.per_rule, out)
    print(f"written to {out}/ (start with exceptions_report.md)")


if __name__ == "__main__":
    main()
