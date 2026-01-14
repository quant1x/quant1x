#!/bin/bash

# Script to loop execute build/q1x update --base=xdxr until issue appears
# Assumes the executable is at build/q1x relative to project root

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$PROJECT_ROOT/build/q1x"
COMMAND="$EXECUTABLE update --base=xdxr"

# Timeout in seconds to detect if the process hangs
TIMEOUT=300

iteration=1

while true; do
    echo "Starting iteration $iteration at $(date)"

    # Run the command in background
    $COMMAND &
    pid=$!

    # Wait up to TIMEOUT seconds
    timed_out=false
    for ((i=1; i<=TIMEOUT; i++)); do
        if ! kill -0 $pid 2>/dev/null; then
            # Process finished
            wait $pid
            exit_code=$?
            echo "Iteration $iteration completed with exit code $exit_code"
            break
        fi
        sleep 1
    done

    if kill -0 $pid 2>/dev/null; then
        # Still running, timed out
        echo "Iteration $iteration timed out (possible hang detected), killing process"
        kill $pid
        echo "Issue appeared at iteration $iteration"
        break
    fi

    iteration=$((iteration + 1))

    # Optional: sleep between iterations to avoid overwhelming the system
    sleep 1
done

echo "Script ended"