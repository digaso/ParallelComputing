#!/bin/bash

# Simple timing script that captures real execution times
echo "📊 Performance Results Summary:"
echo "================================"
printf "%-12s %-10s %s\n" "Processes" "Matrix" "Time (seconds)"
echo "------------------------------------"

# P=1 tests
echo "🔹 Running P=1 tests..." >&2
for input in input6 input300 input600 input900 input1200; do
    echo "Testing P=1 $input..." >&2
    time_result=$(TIMEFORMAT='%3R'; time (cat matrix_examples/$input | mpirun --oversubscribe -np 1 ./program >/dev/null 2>&1) 2>&1)
    printf "%-12s %-10s %s\n" "P=1" "$input" "${time_result}s"
done

# P=4 tests  
echo "🔹 Running P=4 tests..." >&2
for input in input6 input300 input600 input900 input1200; do
    echo "Testing P=4 $input..." >&2
    time_result=$(TIMEFORMAT='%3R'; time (cat matrix_examples/$input | mpirun --oversubscribe -np 4 ./program >/dev/null 2>&1) 2>&1)
    printf "%-12s %-10s %s\n" "P=4" "$input" "${time_result}s"
done

# P=9 tests
echo "🔹 Running P=9 tests..." >&2
for input in input6 input300 input600 input900 input1200; do
    echo "Testing P=9 $input..." >&2
    time_result=$(TIMEFORMAT='%3R'; time (cat matrix_examples/$input | mpirun --oversubscribe -np 9 ./program >/dev/null 2>&1) 2>&1)
    printf "%-12s %-10s %s\n" "P=9" "$input" "${time_result}s"
done

# P=16 tests
echo "🔹 Running P=16 tests..." >&2
for input in input300 input600 input1200; do
    echo "Testing P=16 $input..." >&2
    time_result=$(TIMEFORMAT='%3R'; time (cat matrix_examples/$input | mpirun --oversubscribe -np 16 ./program >/dev/null 2>&1) 2>&1)
    printf "%-12s %-10s %s\n" "P=16" "$input" "${time_result}s"
done

# P=25 tests
echo "🔹 Running P=25 tests..." >&2
for input in input300 input600 input900 input1200; do
    echo "Testing P=25 $input..." >&2
    time_result=$(TIMEFORMAT='%3R'; time (cat matrix_examples/$input | mpirun --oversubscribe -np 25 ./program >/dev/null 2>&1) 2>&1)
    printf "%-12s %-10s %s\n" "P=25" "$input" "${time_result}s"
done