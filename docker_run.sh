#!/usr/bin/env bash
# Start the experiment container with the standard mounts (run from the project root).
#   ./docker_run.sh <container-name> <command...>          detached; follow with: docker logs -f <container-name>
#   INTERACTIVE=1 ./docker_run.sh <container-name> bash    interactive shell, container removed on exit
# Optional environment: IMAGE (default kg-outlier-django:latest), MEMORY (e.g. 200g), CPUSET (e.g. 0-7)
# Examples:
#   ./docker_run.sh exp_dbpedia_dist0 python3 run_experiment.py dbpedia --setting dist:0 --timeout 7200
#   INTERACTIVE=1 ./docker_run.sh shell bash
set -euo pipefail
IMAGE=${IMAGE:-kg-outlier-django:latest}
[[ $# -ge 2 ]] || { sed -n '2,8p' "$0"; exit 1; }
NAME=$1; shift

mkdir -p config dataset index enumerate query results

opts=(--init --name "$NAME" --user "$(id -u):$(id -g)" -w /app)
[[ -n ${MEMORY:-} ]] && opts+=(--memory "$MEMORY" --memory-swap "$MEMORY")
[[ -n ${CPUSET:-} ]] && opts+=(--cpuset-cpus "$CPUSET")
mounts=(-v "$PWD/config:/app/config:ro"
        -v "$PWD/dataset:/app/dataset:ro"
        -v "$PWD/index:/app/index"
        -v "$PWD/enumerate:/app/enumerate"
        -v "$PWD/query:/app/query"
        -v "$PWD/results:/app/results")

if [[ ${INTERACTIVE:-0} == 1 ]]; then
    docker run -it --rm "${opts[@]}" "${mounts[@]}" "$IMAGE" "$@"
else
    docker run -d "${opts[@]}" "${mounts[@]}" "$IMAGE" "$@"
    echo "started container '$NAME'"
    echo "  follow:  docker logs -f $NAME"
    echo "  stop:    docker stop -t 60 $NAME     (the running query is marked interrupted, nothing is lost)"
    echo "  resume:  docker rm $NAME, then start the same command again"
fi
