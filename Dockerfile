# Image for the knowledge-graph outlier detection experiments.
# Contains the compiled C++ programs and the Python scripts. The data (dataset/, index/, enumerate/,
# query/, results/, config/) is NOT in the image: docker_run.sh mounts it from the host.
#
# Build (from the project root, next to main.cpp):   docker build -t kg-outlier:latest .
# Other optimisation or assertion flags:             docker build --build-arg CXXFLAGS="-O2 -D_GLIBCXX_ASSERTIONS" -t kg-outlier:latest .

# Base image. ubuntu:20.04 (glibc 2.31) also works on old Docker hosts, where newer images (22.04, 24.04) fail
# during `apt-get update` with "Problem executing scripts APT::Update::Post-Invoke".
# Other base:  docker build --build-arg BASE=ubuntu:24.04 -t kg-outlier:latest .
ARG BASE=ubuntu:20.04

# ---------- stage 1: compile ----------
FROM ${BASE} AS build
ENV DEBIAN_FRONTEND=noninteractive
# rapidjson-dev is only a fallback; the rapidjson/ folder copied below takes precedence for #include "rapidjson/..."
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++
WORKDIR /src
COPY main.cpp enumerate_view.cpp ./
# the rapidjson headers from your project (delete this line if the project has no rapidjson/ folder)
COPY rapidjson/ ./rapidjson/
RUN { g++ --version | head -n 1; } > build_info.txt \
 && g++ -o main main.cpp \
 && g++ -o enumerate_view enumerate_view.cpp

# ---------- stage 2: runtime ----------
FROM ${BASE}
ENV DEBIAN_FRONTEND=noninteractive PYTHONUNBUFFERED=1
# procps provides pgrep, which run_experiment.py uses to warn about other running ./main processes
RUN apt-get update \
 && apt-get install -y --no-install-recommends python3 procps 
WORKDIR /app
COPY --from=build /src/main /src/enumerate_view /src/build_info.txt ./
COPY run_experiment.py aggregate_results.py ./
CMD ["bash"]
