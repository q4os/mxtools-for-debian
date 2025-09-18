#!/bin/bash

# Performance profiling script
# Usage: ./profile_runner.sh [label] [iterations]

LABEL=${1:-"baseline"}
ITERATIONS=${2:-5}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
PROFILE_DIR="profiles"
PROFILE_FILE="${PROFILE_DIR}/${TIMESTAMP}_${LABEL}.txt"

mkdir -p "$PROFILE_DIR"

echo "=== Performance Profile: $LABEL ===" | tee "$PROFILE_FILE"
echo "Timestamp: $(date)" | tee -a "$PROFILE_FILE"
echo "Iterations: $ITERATIONS" | tee -a "$PROFILE_FILE"
echo "Binary: timer_test_gcc" | tee -a "$PROFILE_FILE"
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
    output=$(./timer_test_gcc 2>&1)
    echo "$output" | tee -a "$PROFILE_FILE"
    
    # Extract times using grep and awk
    load_time=$(echo "$output" | grep "AptCache::loadCacheFiles" | tail -1 | awk '{print $3}' | sed 's/ms//')
    parse_time=$(echo "$output" | grep "AptCache::parseContent" | tail -1 | awk '{print $3}' | sed 's/ms//')
    total_time=$(echo "$output" | grep "Total test time" | tail -1 | awk '{print $5}' | sed 's/ms//')
    
    if [ ! -z "$load_time" ]; then
        load_times+=($load_time)
    fi
    if [ ! -z "$parse_time" ]; then
        parse_times+=($parse_time)
    fi
    if [ ! -z "$total_time" ]; then
        total_times+=($total_time)
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