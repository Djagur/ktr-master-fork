#!/usr/bin/env bash
# Run ./main for one setting inside a tmux session and keep only the useful output.
#
# usage:  ./run_setting.sh <dataset> <dist|freq> <o> [k=100]
#             starts a detached tmux session named <dataset>_<s>_o<o> and returns
#         ./run_setting.sh --foreground <dataset> <dist|freq> <o> [k=100]
#             runs right here, without tmux (this is what the tmux session calls)
#
# Run it from the repository root (where ./main, config/ and query/ live).
# Optional environment variables: FILTER (default ./filter.awk),
#   OUTDIR_BASE (default results), QUERIES (default queries)
#
# Writes to $OUTDIR_BASE/<dataset>/raw/:
#   <dataset>_<s>_o<o>_summary.txt, _top.txt, _meta.txt, _stderr.log
set -uo pipefail

SELF=$(realpath "$0")
FILTER=${FILTER:-./filter.awk}
OUTDIR_BASE=${OUTDIR_BASE:-results}
QUERIES=${QUERIES:-queries}

foreground=0
if [[ ${1:-} == --foreground ]]; then foreground=1; shift; fi
if [[ $# -lt 3 ]]; then sed -n '2,7p' "$0"; exit 1; fi
ds=$1; s=$2; o=$3; k=${4:-100}

[[ $s == dist || $s == freq ]] || { echo "s must be dist or freq" >&2; exit 1; }
[[ $o =~ ^[0-9]+$ ]] || { echo "o must be a number (0, 1 or 2)" >&2; exit 1; }
[[ -x ./main ]] || { echo "./main not found or not executable; run from the repo root" >&2; exit 1; }
[[ -f $FILTER ]] || { echo "filter file '$FILTER' not found" >&2; exit 1; }

pre=${ds}_${s}_o${o}
outdir=$OUTDIR_BASE/$ds/raw
mkdir -p "$outdir"

if [[ $foreground == 1 ]]; then
    cmd="./main -d $ds -q $QUERIES -k $k -s $s -o $o -f 0"
    {
        echo "$cmd"; date; echo "host: $(hostname)"; echo "nproc: $(nproc)"; lscpu | head -n 15
    } > "$outdir/${pre}_meta.txt"

    start=$SECONDS
    # stderr goes both to a log file and into the pipe, so nothing is lost
    { ./main -d "$ds" -q "$QUERIES" -k "$k" -s "$s" -o "$o" -f 0 \
        2> >(tee "$outdir/${pre}_stderr.log" >&2); } 2>&1 \
        | awk -v pre="$outdir/$pre" -f "$FILTER"
    status=${PIPESTATUS[0]}

    { echo "exit status: $status"; echo "elapsed: $((SECONDS - start)) s"; date; } >> "$outdir/${pre}_meta.txt"
    if [[ $status -ne 0 ]]; then
        echo "FAILED with exit status $status, see $outdir/${pre}_stderr.log" >&2
        exit "$status"
    fi
    echo "done: $outdir/${pre}_{summary,top,meta}.txt"
    exit 0
fi

# ---- launch mode: start a detached tmux session that calls this script with --foreground
command -v tmux >/dev/null || { echo "tmux not found" >&2; exit 1; }
if tmux has-session -t "=$pre" 2>/dev/null; then
    echo "tmux session '$pre' already exists (attach with: tmux attach -t $pre)" >&2
    exit 1
fi
if pgrep -x main >/dev/null; then
    echo "WARNING: another ./main is already running; timings of concurrent runs are distorted." >&2
fi

# env is passed explicitly because a tmux session may not inherit this shell's variables
tmux new-session -d -s "$pre" -c "$PWD" \
    "$(printf '%q ' env "FILTER=$FILTER" "OUTDIR_BASE=$OUTDIR_BASE" "QUERIES=$QUERIES" \
        "$SELF" --foreground "$ds" "$s" "$o" "$k")"

echo "started tmux session '$pre'"
echo "  attach:  tmux attach -t $pre     (detach again with Ctrl-b d)"
echo "  list:    tmux ls"
echo "  output:  $outdir/${pre}_{summary,top,meta}.txt"
