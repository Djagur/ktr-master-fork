#!/usr/bin/env python3
"""Run ./main incrementally: one process per query, per setting, with live progress information.

Why: a single ./main process runs all queries and only prints its timing summary at the very end, so one
heavy query blocks everything and nothing is usable until the whole run is over. Here every query is
its own process (own timeout, own status), results are kept per query, and after every query the
per-query parts are merged into the files aggregate_results.py reads:
    <out>/<dataset>/raw/<dataset>_<s>_o<o>_summary.txt, _top.txt, _meta.txt
(the <pNN> numbers are the positions in the query list, so rule grouping stays correct even when
some queries are missing or timed out).

Per-query parts live in <out>/<dataset>/raw/parts/<dataset>_<s>_o<o>/<query>/ :
    summary.txt  top.txt  status.json  (tail.log for failed / timed-out runs)
A finished query is skipped on the next invocation, so you can stop and continue at any time.

Examples (run from the repository root):
  # one setting, all queries not yet done, inside tmux, kill queries after 1 hour:
  python run_experiment.py dbpedia --setting dist:0 --timeout 3600 --tmux
  # only specific queries (name, index or range), any setting:
  python run_experiment.py dbpedia --setting dist:0 --query generated_p17 3 10-19
  # only some rules:
  python run_experiment.py dbpedia --setting freq:0 --rules 0 1 2
  # several settings one after the other, show the plan only:
  python run_experiment.py dbpedia --setting dist:0 --setting freq:0 --dry-run
  # cheap rules: 10 queries per process, so the graph is loaded once per 10 queries instead of once per query
  python run_experiment.py dbpedia --setting dist:0 --rules 0 1 2 3 --per-process 10 --timeout 7200
  # rebuild the merged files without running anything:
  python run_experiment.py dbpedia --setting dist:0 --merge-only

--per-process N: main loads the graph and pretreats it once per process (about 2.5 minutes on DBpedia).
With N > 1 that cost is shared, but main only prints its summary when all N queries are finished and
--timeout applies to the whole process, so use N > 1 only for cheap queries.
"""
import argparse
import json
import os
import re
import shlex
import shutil
import socket
import subprocess
import sys
import threading
import time
from collections import deque
from pathlib import Path

KEEP_RE = re.compile(r"^---|^<Pattern|^Exception Score|^pe-[0-9]+:| Exception[.]$")   # same as filter.awk
RANK_RE = re.compile(r"^(\d+)(?:st|nd|rd|th) Exception\.")
CLK_TCK = os.sysconf("SC_CLK_TCK") if hasattr(os, "sysconf") else 100


# ---------------------------------------------------------------- helpers
class Log:
    def __init__(self, path):
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        self.f, self.lock = open(path, "a", buffering=1), threading.Lock()

    def __call__(self, msg=""):
        line = f"[{time.strftime('%H:%M:%S')}] {msg}" if msg else ""
        with self.lock:
            print(line, flush=True)
            self.f.write(line + "\n")


def fmt_dur(s):
    h, r = divmod(int(s), 3600)
    m, s = divmod(r, 60)
    return f"{h}:{m:02d}:{s:02d}"


def read_kb(path, key):
    try:
        for line in open(path):
            if line.startswith(key):
                return int(line.split()[1])
    except OSError:
        pass
    return None


def cpu_jiffies(pid):
    try:
        f = open(f"/proc/{pid}/stat").read().rsplit(")", 1)[1].split()
        return int(f[11]) + int(f[12])
    except (OSError, IndexError, ValueError):
        return None


def read_status(part):
    try:
        return json.loads((part / "status.json").read_text())
    except (OSError, ValueError):
        return None


def parse_setting(text):
    try:
        s, o = text.split(":")
        assert s in ("dist", "freq") and o.isdigit()
        return s, int(o)
    except (ValueError, AssertionError):
        raise SystemExit(f"bad --setting '{text}', expected dist:0, freq:1, ...")


def select_queries(args, names):
    """Indices chosen by --query / --rules (all when neither is given)."""
    chosen = set(range(len(names)))
    if args.query:
        chosen = set()
        for tok in args.query:
            if tok in names:
                chosen.add(names.index(tok))
            elif re.fullmatch(r"\d+", tok):
                chosen.add(int(tok))
            elif re.fullmatch(r"\d+-\d+", tok):
                a, b = map(int, tok.split("-"))
                chosen.update(range(a, b + 1))
            else:
                raise SystemExit(f"unknown query '{tok}' (use a name from the list, an index or a-b)")
    if args.rules:
        chosen = {i for i in chosen if i // args.per_rule in args.rules}
    bad = [i for i in chosen if not 0 <= i < len(names)]
    if bad:
        raise SystemExit(f"query indices out of range: {sorted(bad)} (list has {len(names)} queries)")
    return sorted(chosen)


# ------------------------------------------------------------ one process
def split_summary(summary, n):
    """Split the summary of a process that ran n queries into n single-query summaries (tagged <p00>)."""
    head, sep, counts = summary.partition("Searched Pattern Num:")
    times = head.partition("Process Time:")[2]
    t_parts = re.split(r"^<p\d+>\n?", times, flags=re.M)      # [preparation, block 0, block 1, ...]
    c_parts = re.split(r"^<p\d+>\n?", counts, flags=re.M)
    if not sep or len(t_parts) != n + 1 or len(c_parts) != n + 1:
        return None
    prep = t_parts[0].rstrip("\n")
    return [f"Process Time:{prep}\n\n<p00>\n{t_parts[k + 1].rstrip(chr(10))}\n\n"
            f"Searched Pattern Num:\n\n<p00>\n{c_parts[k + 1].rstrip(chr(10))}\n" for k in range(n)]


def split_top(lines, n):
    """Split the exception lines of a process that ran n queries; None if the boundaries are unclear."""
    if n == 1:
        return ["".join(l for l in lines if not l.startswith("---"))]
    seps = [i for i, l in enumerate(lines) if l.startswith("---")]      # one separator line per query?
    if len(seps) == n:
        bounds = seps + [len(lines)]
        return ["".join(l for l in lines[bounds[k] + 1:bounds[k + 1]] if not l.startswith("---")) for k in range(n)]
    starts, last = [], None                                             # otherwise: rank goes down = new query
    for i, l in enumerate(lines):
        m = RANK_RE.match(l)
        if m:
            if last is None or int(m.group(1)) < last:
                starts.append(i)
            last = int(m.group(1))
    if len(starts) == n:
        bounds = starts + [len(lines)]
        return ["".join(l for l in lines[bounds[k]:bounds[k + 1]] if not l.startswith("---")) for k in range(n)]
    return None


def run_group(args, log, setting, group, parts_dir):
    """Run ./main once for the queries in group = [(index, name), ...] and store one part per query."""
    s, o = setting
    names = [n for _, n in group]
    gdir = parts_dir / f"_group_{names[0]}"
    gdir.mkdir(parents=True, exist_ok=True)
    list_name = f"_list_{args.dataset}_{s}_o{o}_{names[0]}"
    list_path = Path(args.query_dir) / f"{list_name}.json"
    list_path.write_text(json.dumps(names))
    cmd = [args.main, "-d", args.dataset, "-q", list_name, "-k", str(args.k), "-s", s, "-o", str(o), "-f", "0"]
    if shutil.which("stdbuf"):
        cmd = ["stdbuf", "-oL", "-eL"] + cmd

    state = {"stage": "starting", "top_lines": 0, "rss_peak": 0, "last_out": time.time(), "timed_out": False}
    t0, started = time.time(), time.strftime("%Y-%m-%d %H:%M:%S")
    tail = deque(maxlen=60)
    top_f, sum_f = open(gdir / "top.txt", "w"), open(gdir / "summary.txt", "w")
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                            errors="replace", bufsize=1)
    stop = threading.Event()

    def watch():
        last_beat, prev_j, prev_t = t0, cpu_jiffies(proc.pid), time.time()
        while not stop.wait(1):
            now = time.time()
            el = now - t0
            rss = read_kb(f"/proc/{proc.pid}/status", "VmRSS:")
            if rss:
                state["rss_peak"] = max(state["rss_peak"], rss)
            if args.timeout and el > args.timeout and not state["timed_out"]:
                state["timed_out"] = True
                log(f"  TIMEOUT after {fmt_dur(el)}: stopping the process")
                proc.terminate()
                threading.Timer(20, lambda: proc.poll() is None and proc.kill()).start()
            if now - last_beat >= args.heartbeat:
                last_beat = now
                j = cpu_jiffies(proc.pid)
                cpu = (j - prev_j) / CLK_TCK / (now - prev_t) * 100 if j is not None and prev_j is not None else 0
                prev_j, prev_t = j, now
                avail = read_kb("/proc/meminfo", "MemAvailable:")
                log(f"  ... {fmt_dur(el)} | stage: {state['stage']} | RSS {(rss or 0) / 1048576:.1f} GB "
                    f"(peak {state['rss_peak'] / 1048576:.1f}) | CPU {cpu:.0f}% | "
                    f"free RAM {(avail or 0) / 1048576:.1f} GB | exception lines {state['top_lines']} | "
                    f"last output {now - state['last_out']:.0f}s ago")

    th = threading.Thread(target=watch, daemon=True)
    th.start()
    in_summary, interrupted = False, False
    try:
        for line in proc.stdout:
            state["last_out"] = time.time()
            raw = line.rstrip("\n")
            tail.append(raw)
            if not in_summary and raw.startswith("Process Time:"):
                in_summary = True
            if in_summary:
                sum_f.write(line)
            else:
                if raw.startswith("> "):
                    state["stage"] = raw[2:].strip()
                    log(f"  {raw}")
                elif raw.startswith("   RG "):
                    log(f"  {raw.strip()}")
                if KEEP_RE.search(raw):
                    top_f.write(line)
                    state["top_lines"] += 1
        rc = proc.wait()
    except KeyboardInterrupt:
        interrupted = True
        proc.terminate()
        rc = proc.wait()
    finally:
        stop.set()
        top_f.close()
        sum_f.close()
        list_path.unlink(missing_ok=True)

    elapsed = time.time() - t0
    summary = (gdir / "summary.txt").read_text(errors="replace")
    pieces = split_summary(summary, len(group)) if rc == 0 and not interrupted and not state["timed_out"] else None
    status = ("interrupted" if interrupted else "timeout" if state["timed_out"]
              else "ok" if rc == 0 and pieces else "failed")
    tops = None
    if status == "ok":
        tops = split_top((gdir / "top.txt").read_text(errors="replace").splitlines(True), len(group))
        if tops is None:
            log("  WARNING: could not tell which exceptions belong to which query; timings are kept, "
                "exceptions of this group are not stored")
    infos, summaries = [], []
    for k, (idx, name) in enumerate(group):
        part = parts_dir / name
        part.mkdir(parents=True, exist_ok=True)
        (part / "tail.log").unlink(missing_ok=True)
        (part / "summary.txt").write_text(pieces[k] if status == "ok" else "")
        (part / "top.txt").write_text(tops[k] if tops else "")
        info = {"query": name, "index": idx, "status": status, "exit_code": rc,
                "elapsed_s": round(elapsed / len(group), 1), "process_elapsed_s": round(elapsed, 1),
                "queries_in_process": len(group), "started": started, "finished": time.strftime("%Y-%m-%d %H:%M:%S"),
                "rss_peak_gb": round(state["rss_peak"] / 1048576, 2),
                "exception_lines": len(tops[k].splitlines()) if tops else 0, "cmd": " ".join(cmd)}
        if status == "ok" and tops is None:
            info["top_attributed"] = False
        if status != "ok":
            (part / "tail.log").write_text("\n".join(tail) + "\n")
        (part / "status.json").write_text(json.dumps(info, indent=1))
        infos.append(info)
        summaries.append(pieces[k] if status == "ok" else "")
    shutil.rmtree(gdir, ignore_errors=True)
    if interrupted:
        raise KeyboardInterrupt
    return infos, summaries


def query_sumtime(summary):
    """SumTime of the query itself (the one after the Preparation block) and patterns searched."""
    times = re.findall(r"SumTime:\s*([\d.]+)", summary.split("Searched Pattern Num:")[0])
    counts = summary.split("Searched Pattern Num:")[1] if "Searched Pattern Num:" in summary else ""
    patterns = sum(int(c) for c in re.findall(r"^\s*\d+:\s*(\d+)\s*$", counts, re.M))
    return (float(times[1]) if len(times) > 1 else None), patterns


# ------------------------------------------------------------------ merge
def merge(args, log, setting, names, raw, quiet=False):
    s, o = setting
    pre = f"{args.dataset}_{s}_o{o}"
    parts = raw / "parts" / pre
    width = max(2, len(str(len(names) - 1)))
    prep, time_blocks, count_blocks, top = None, [], [], []
    done, timeouts, failed, wall, peak, unattributed = [], [], [], 0.0, 0.0, []
    first, last = None, None
    for idx, name in enumerate(names):
        st = read_status(parts / name)
        if st is None:
            continue
        if st["status"] == "timeout":
            timeouts.append(name)
            continue
        if st["status"] != "ok":
            failed.append(f"{name} ({st['status']})")
            continue
        summary = (parts / name / "summary.txt").read_text(errors="replace")
        head, _, counts = summary.partition("Searched Pattern Num:")
        times = head.partition("Process Time:")[2]
        m, mc = re.search(r"^<p\d+>", times, re.M), re.search(r"^<p\d+>", counts, re.M)
        if not m or not mc:
            failed.append(f"{name} (unreadable summary)")
            continue
        if prep is None:
            prep = times[:m.start()]
        tag = f"<p{idx:0{width}d}>"
        time_blocks.append(tag + times[m.end():].rstrip("\n") + "\n")
        count_blocks.append(tag + counts[mc.end():].rstrip("\n") + "\n")
        body = (parts / name / "top.txt").read_text(errors="replace").splitlines(True)
        # exactly one separator line per query, so aggregate_results.py can split the exceptions safely
        top.append(f"---{name}---\n" + "".join(l for l in body if not l.startswith("---")))
        done.append(name)
        if st.get("top_attributed") is False:
            unattributed.append(name)
        wall += st["elapsed_s"]
        peak = max(peak, st.get("rss_peak_gb", 0))
        first = min(first or st["started"], st["started"])
        last = max(last or st["finished"], st["finished"])
    if not done:
        if not quiet:
            log("  nothing to merge yet")
        return
    (raw / f"{pre}_summary.txt").write_text(
        "Process Time:" + prep.rstrip("\n") + "\n\n" + "\n".join(time_blocks)
        + "\nSearched Pattern Num:\n\n" + "".join(count_blocks))
    (raw / f"{pre}_top.txt").write_text("".join(top))
    try:
        cpu = subprocess.run(["lscpu"], capture_output=True, text=True).stdout.splitlines()[:15]
    except OSError:
        cpu = []
    meta = [f"./main -d {args.dataset} -q {args.list} -k {args.k} -s {s} -o {o} -f 0",
            "mode: incremental, one process per query (results merged from per-query parts)",
            f"first start: {first}", f"last finish: {last}", f"host: {socket.gethostname()}",
            f"nproc: {os.cpu_count()}", f"queries done: {len(done)}/{len(names)}",
            f"queries per process: {args.per_process}; timeout per process: {args.timeout or 'none'} s",
            f"timed out (not in the results): {', '.join(timeouts) or 'none'}",
            f"failed or interrupted (not in the results): {', '.join(failed) or 'none'}",
            f"timings kept but exceptions not attributable to single queries: {', '.join(unattributed) or 'none'}",
            f"sum of wall-clock time of the processes including graph loading: {fmt_dur(wall)}",
            f"largest resident memory of a query process: {peak} GB", *cpu]
    (raw / f"{pre}_meta.txt").write_text("\n".join(meta) + "\n")
    if not quiet:
        log(f"  merged {len(done)}/{len(names)} queries into {pre}_summary.txt / _top.txt / _meta.txt"
            + (f" ({len(timeouts)} timed out)" if timeouts else ""))


def report(args, log, setting, names, raw):
    s, o = setting
    parts = raw / "parts" / f"{args.dataset}_{s}_o{o}"
    log(f"progress for {args.dataset} s={s} o={o} (ok / timeout / failed / not run):")
    for r in range((len(names) + args.per_rule - 1) // args.per_rule):
        sts = [(read_status(parts / n) or {}).get("status", "-") for n in names[r * args.per_rule:(r + 1) * args.per_rule]]
        log(f"  rho{r}: {sts.count('ok')} / {sts.count('timeout')} / "
            f"{sum(x not in ('ok', 'timeout', '-') for x in sts)} / {sts.count('-')}")


# ------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("dataset")
    ap.add_argument("--setting", action="append", default=[], metavar="S:O", help="dist:0, freq:1, ... (repeatable)")
    ap.add_argument("--all-settings", action="store_true", help="dist:0 freq:0 dist:1 freq:1")
    ap.add_argument("--query", nargs="+", help="query names, indices or ranges like 10-19")
    ap.add_argument("--rules", type=int, nargs="+", help="only these rules (uses --per-rule to map queries to rules)")
    ap.add_argument("--per-rule", type=int, default=10)
    ap.add_argument("--list", default="generated_queries", help="query list name without .json (default: generated_queries)")
    ap.add_argument("--k", type=int, default=100)
    ap.add_argument("--timeout", type=float, default=0, help="seconds per process, 0 = no limit")
    ap.add_argument("--per-process", type=int, default=1, metavar="N",
                    help="queries per ./main process (default 1); N > 1 shares the graph loading but a\n"
                         "timeout or crash then loses all N queries")
    ap.add_argument("--retry-timeouts", action="store_true", help="run queries again that timed out earlier")
    ap.add_argument("--force", action="store_true", help="run queries again even if they finished")
    ap.add_argument("--heartbeat", type=int, default=30, help="seconds between progress lines")
    ap.add_argument("--main", default="./main")
    ap.add_argument("--out", default="results")
    ap.add_argument("--query-dir", help="default: query/<dataset>")
    ap.add_argument("--dry-run", action="store_true", help="print the plan and stop")
    ap.add_argument("--merge-only", action="store_true", help="only rebuild the merged files")
    ap.add_argument("--tmux", action="store_true", help="run inside a detached tmux session")
    args = ap.parse_args()

    settings = [parse_setting(t) for t in args.setting]
    if args.all_settings:
        settings += [("dist", 0), ("freq", 0), ("dist", 1), ("freq", 1)]
    if not settings:
        raise SystemExit("give at least one --setting dist:0 (or --all-settings)")
    args.query_dir = args.query_dir or f"query/{args.dataset}"

    if args.tmux:
        if not shutil.which("tmux"):
            raise SystemExit("tmux not found")
        session = f"{args.dataset}_" + (f"{settings[0][0]}_o{settings[0][1]}" if len(settings) == 1 else "run")
        if subprocess.run(["tmux", "has-session", "-t", f"={session}"], capture_output=True).returncode == 0:
            raise SystemExit(f"tmux session '{session}' already exists (tmux attach -t {session})")
        inner = shlex.join([sys.executable, str(Path(__file__).resolve())]
                           + [a for a in sys.argv[1:] if a != "--tmux"])
        subprocess.run(["tmux", "new-session", "-d", "-s", session, "-c", os.getcwd(),
                        f"{inner}; echo; echo '[finished] press Enter to close this session'; read x"], check=True)
        print(f"started tmux session '{session}'\n  attach:  tmux attach -t {session}   (detach: Ctrl-b d)\n"
              f"  stop:    Ctrl-c inside the session (the query in progress is marked interrupted, nothing is lost)\n"
              f"  log:     {args.out}/{args.dataset}/raw/run_{args.dataset}.log")
        return

    list_path = Path(args.query_dir) / f"{args.list}.json"
    if not list_path.exists():
        raise SystemExit(f"{list_path} not found (run generate_queries.py first)")
    names = json.loads(list_path.read_text())
    if not (isinstance(names, list) and all(isinstance(n, str) for n in names)):
        raise SystemExit(f"{list_path} must be a JSON list of query folder names")
    raw = Path(args.out) / args.dataset / "raw"
    log = Log(raw / f"run_{args.dataset}.log")
    width = max(2, len(str(len(names) - 1)))
    chosen = select_queries(args, names)

    log("=" * 78)
    log(f"run_experiment: dataset={args.dataset} host={socket.gethostname()} cpus={os.cpu_count()} "
        f"cwd={os.getcwd()}")
    log(f"query list: {list_path} ({len(names)} queries), selected {len(chosen)}; k={args.k}; "
        f"timeout={args.timeout or 'none'}; queries per process={args.per_process}; heartbeat={args.heartbeat}s")
    if not args.merge_only and not args.dry_run:
        if not Path(args.main).exists():
            raise SystemExit(f"{args.main} not found; run from the repository root")
        if not Path(f"index/{args.dataset}/index.json").exists():
            raise SystemExit(f"index/{args.dataset}/index.json missing: run ./main -d {args.dataset} -f 1 first")
        if subprocess.run(["pgrep", "-x", "main"], capture_output=True).returncode == 0:
            log("WARNING: another ./main is running; timings of concurrent runs are distorted.")

    t_session = time.time()
    try:
        for setting in settings:
            s, o = setting
            pre = f"{args.dataset}_{s}_o{o}"
            parts = raw / "parts" / pre
            todo, skipped = [], {"ok": 0, "timeout": 0}
            for idx in chosen:
                st = read_status(parts / names[idx])
                if st and not args.force:
                    if st["status"] == "ok":
                        skipped["ok"] += 1
                        continue
                    if st["status"] == "timeout" and not args.retry_timeouts:
                        skipped["timeout"] += 1
                        continue
                todo.append(idx)
            log(f"--- setting s={s} o={o}: {len(todo)} to run, {skipped['ok']} already done, "
                f"{skipped['timeout']} timed out earlier (skipped; --retry-timeouts to run them again)")
            if args.dry_run:
                log("    would run: " + ", ".join(names[i] for i in todo[:20]) + (" ..." if len(todo) > 20 else ""))
                continue
            if args.merge_only:
                merge(args, log, setting, names, raw)
                continue
            groups = [[(i, names[i]) for i in todo[k:k + args.per_process]]
                      for k in range(0, len(todo), args.per_process)]
            for n, group in enumerate(groups, 1):
                label = (group[0][1] if len(group) == 1 else f"{group[0][1]} .. {group[-1][1]} ({len(group)} queries)")
                log(f"[{n}/{len(groups)}] {label} (rho{group[0][0] // args.per_rule}) s={s} o={o} started "
                    f"(session {fmt_dur(time.time() - t_session)})")
                try:
                    infos, summaries = run_group(args, log, setting, group, parts)
                finally:
                    merge(args, log, setting, names, raw, quiet=True)
                info = infos[0]
                if info["status"] == "ok":
                    log(f"    process ok in {fmt_dur(info['process_elapsed_s'])} (peak RSS {info['rss_peak_gb']} GB)")
                    for (idx, name), inf, summ in zip(group, infos, summaries):
                        sumtime, patterns = query_sumtime(summ)
                        log(f"    {name}: main's SumTime {sumtime} s, {patterns} patterns searched, "
                            f"{inf['exception_lines']} exception lines")
                else:
                    log(f"    {info['status'].upper()} after {fmt_dur(info['process_elapsed_s'])} (exit code {info['exit_code']}); "
                        f"{len(group)} quer{'y' if len(group) == 1 else 'ies'} not stored, see {parts / group[0][1] / 'tail.log'}")
            if not args.dry_run:
                merge(args, log, setting, names, raw)
                report(args, log, setting, names, raw)
    except KeyboardInterrupt:
        log("interrupted; the query in progress is marked 'interrupted' and will run again next time")
        for setting in settings:
            merge(args, log, setting, names, raw, quiet=True)
    log(f"session finished after {fmt_dur(time.time() - t_session)}")


if __name__ == "__main__":
    main()
