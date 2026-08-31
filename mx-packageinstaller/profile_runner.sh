#!/bin/bash

# Performance profiling script
# Usage: ./profile_runner.sh [label] [iterations]
#
# Drives a standalone timing binary whose output is expected to contain
# lines like "AptCache::loadCacheFiles ... ms", "AptCache::parseContent ...
# ms" and "Total test time ... ms", then averages the results across a
# number of iterations.
#
# NOTE: The binary this script originally drove (timer_test_gcc, built by
# hand from the now-removed src/timer_test.cpp) and the ScopedTimer
# instrumentation it measured (src/scopedtimer.h) were both deleted from the
# project -- see commits 1ef427c0 ("Remove orphaned timer test") and
# 231fb139 ("Remove ScopedTimer diagnostic instrumentation"). Neither was
# ever wired into the CMake build (Testing/CMakeLists.txt has no profiling
# target); they were compiled and run manually. There is currently no
# equivalent profiling binary anywhere in this project.
#
# If you restore equivalent timing instrumentation and a binary that emits
# the markers above, point TARGET_BINARY at it (env var or edit below) to
# use this script again.
TARGET_BINARY="${TARGET_BINARY:-}"

LABEL=${1:-"baseline"}
ITERATIONS=${2:-5}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
PROFILE_DIR="profiles"
PROFILE_FILE="${PROFILE_DIR}/${TIMESTAMP}_${LABEL}.txt"

if [ -z "$TARGET_BINARY" ] || [ ! -x "$TARGET_BINARY" ]; then
    echo "ERROR: no profiling binary configured/found." >&2
    echo "This script used to drive ./timer_test_gcc (built from src/timer_test.cpp)" >&2
    echo "against ScopedTimer instrumentation in AptCache. Both were removed from" >&2
    echo "this project (see commits 1ef427c0 and 231fb139), and neither was ever a" >&2
    echo "CMake build target, so there is nothing equivalent to run today." >&2
    echo "Set TARGET_BINARY=/path/to/binary after restoring comparable timing" >&2
    echo "instrumentation and a binary that emits it before using this script." >&2
    exit 1
fi

mkdir -p "$PROFILE_DIR"

echo "=== Performance Profile: $LABEL ===" | tee "$PROFILE_FILE"
echo "Timestamp: $(date)" | tee -a "$PROFILE_FILE"
echo "Iterations: $ITERATIONS" | tee -a "$PROFILE_FILE"
echo "Binary: $TARGET_BINARY" | tee -a "$PROFILE_FILE"
echo "" | tee -a "$PROFILE_FILE"

# Collect system info
echo "=== System Info ===" | tee -a "$PROFILE_FILE"
echo "CPU: $(grep 'model name' /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)" | tee -a "$PROFILE_FILE"
echo "Memory: $(free -h | grep Mem | awk '{print $2}')" | tee -a "$PROFILE_FILE"
echo "Compiler: $(g++ --version | head -1)" | tee -a "$PROFILE_FILE"
echo "" | tee -a "$PROFILE_FILE"

# Run performance tests
echo "=== Performance Results ===" | tee -a "$PROFILE_FILE"

declare -a load_times=()
declare -a parse_times=()
declare -a total_times=()

for ((i=1; i<=ITERATIONS; i++)); do
    echo "--- Run $i ---" | tee -a "$PROFILE_FILE"

    # Capture output and extract timing data
    output=$("$TARGET_BINARY" 2>&1)
    echo "$output" | tee -a "$PROFILE_FILE"

    # Extract times using grep and awk
    load_time=$(echo "$output" | grep "AptCache::loadCacheFiles" | tail -1 | awk '{print $3}' | sed 's/ms//')
    parse_time=$(echo "$output" | grep "AptCache::parseContent" | tail -1 | awk '{print $3}' | sed 's/ms//')
    total_time=$(echo "$output" | grep "Total test time" | tail -1 | awk '{print $5}' | sed 's/ms//')

    if [ -n "$load_time" ]; then
        load_times+=("$load_time")
    fi
    if [ -n "$parse_time" ]; then
        parse_times+=("$parse_time")
    fi
    if [ -n "$total_time" ]; then
        total_times+=("$total_time")
    fi

    echo "" | tee -a "$PROFILE_FILE"
done

# Calculate averages
if [ ${#load_times[@]} -gt 0 ]; then
    load_avg=$(echo "${load_times[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
    parse_avg=$(echo "${parse_times[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
    total_avg=$(echo "${total_times[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')

    echo "=== Summary ===" | tee -a "$PROFILE_FILE"
    echo "Load Cache Average: ${load_avg}ms" | tee -a "$PROFILE_FILE"
    echo "Parse Content Average: ${parse_avg}ms" | tee -a "$PROFILE_FILE"
    echo "Total Time Average: ${total_avg}ms" | tee -a "$PROFILE_FILE"
    echo "" | tee -a "$PROFILE_FILE"

    # Save to CSV for easy comparison
    CSV_FILE="${PROFILE_DIR}/performance_summary.csv"
    if [ ! -f "$CSV_FILE" ]; then
        echo "label,timestamp,load_avg,parse_avg,total_avg" > "$CSV_FILE"
    fi
    echo "$LABEL,$TIMESTAMP,$load_avg,$parse_avg,$total_avg" >> "$CSV_FILE"

    echo "Profile saved to: $PROFILE_FILE"
    echo "CSV summary updated: $CSV_FILE"
else
    echo "ERROR: Could not extract timing data from test output" | tee -a "$PROFILE_FILE"
fi
