import json
import sys
import os

WILD = 10_000_000  # set to the "RG wildcard >=" value printed for nell (or the value you used)
qdir, qfile = sys.argv[1], sys.argv[2]  # e.g. query/nell  and the queries JSON
names = json.load(open(qfile))  # adapt if it isn't a plain list of directory names
sig = {
    (2, 0, 0): "rho0",
    (2, 1, 0): "rho1",
    (2, 1, 1): "rho2",
    (3, 0, 0): "rho3",
    (3, 2, 1): "rho4",
    (3, 1, 2): "rho5",
    (3, 2, 2): "rho6",
}
for i, n in enumerate(names):
    ctx = json.load(open(os.path.join(qdir, n, "context.json")))[0]
    ents, links = ctx["Entity"], ctx["Link"]
    w_e = sum(e["EntityID"] >= WILD for e in ents)
    w_l = sum(l["LinkID"] >= WILD for l in links)
    print(
        f"p{i:02d} {n:10s} nodes={len(ents)} wild_nodes={w_e} wild_links={w_l} -> "
        f"{sig.get((len(ents), w_e, w_l), 'NO MATCH')}"
    )

