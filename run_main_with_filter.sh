#!/usr/bin/env bash
# Queue all four settings (dist/freq x o=0/1) for one dataset in a single tmux session.
# They run one after another, so the timings are not distorted by parallel runs.
#
# usage:  ./run_settings.sh <dataset> [k=100]
#         FORCE=1 ./run_settings.sh <dataset>     (rerun settings that already finished)
#
# Run it from the repository root, next to run_setting.sh and filter.awk.
# Order: dist o0, freq o0, dist o1, freq o1 (the slow exhaustive runs come last).
# A setting whose _meta.txt already says "exit status: 0" is skipped.
set -uo pipefail

HERE=$(dirname "$(realpath "$0")")
[[ $# -ge 1 ]] || { sed -n '2,7p' "$0"; exit 1; }
ds=$1; k=${2:-100}
OUTDIR_BASE=${OUTDIR_BASE:-results}
FILTER=${FILTER:-./filter.awk}
QUERIES=${QUERIES:-queries}
session=${ds}_all

[[ -f index/$ds/index.json ]] || {
    echo "index/$ds/index.json not found: run ./main -d $ds -f 1 first" >&2; exit 1; }
[[ -f $HERE/run_setting.sh ]] || { echo "run_setting.sh not found next to this script" >&2; exit 1; }
command -v tmux >/dev/null || { echo "tmux not found" >&2; exit 1; }
if tmux has-session -t "=$session" 2>/dev/null; then
    echo "tmux session '$session' already exists (attach with: tmux attach -t $session)" >&2
    exit 1
fi
if pgrep -x main >/dev/null; then
    echo "WARNING: another ./main is already running; timings will be distorted." >&2
fi

queue=""
for setting in "dist 0" "freq 0" "dist 1" "freq 1"; do
    read -r s o <<< "$setting"
    pre=${ds}_${s}_o${o}
    if [[ ${FORCE:-0} != 1 ]] && grep -qs '^exit status: 0' "$OUTDIR_BASE/$ds/raw/${pre}_meta.txt"; then
        echo "skipping $pre (already finished; use FORCE=1 to rerun)"
        continue
    fi
    # ';' (not '&&'), so one failed setting does not stop the rest
    queue+=$(printf '%q ' env "OUTDIR_BASE=$OUTDIR_BASE" "FILTER=$FILTER" "QUERIES=$QUERIES" \
                 bash "$HERE/run_setting.sh" --foreground "$ds" "$s" "$o" "$k")"; "
done

[[ -n $queue ]] || { echo "nothing to do"; exit 0; }

tmux new-session -d -s "$session" -c "$PWD" "$queue echo 'queue finished'"

echo "started tmux session '$session'"
echo "  attach:  tmux attach -t $session     (detach again with Ctrl-b d)"
echo "  output:  $OUTDIR_BASE/$ds/raw/"
