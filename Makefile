# Makefile for All-Pairs Shortest Path using Fox's Algorithm
# Project Assignment I: Parallel Computing

# Compiler and flags
MPICC = mpicc
CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99
LDFLAGS = -lm

# MPI runtime detection and command setup
MPIRUN := $(shell which mpirun 2>/dev/null)

# Target executables
TARGET = fox

# Source files
SOURCES = main.c mpi_context.c io_handler.c fox_algorithm.c matrix_utils.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Helper function to run MPI commands with OpenMPI
define run_mpi
	@if [ -f ./$(TARGET) ] && [ -n "$(MPIRUN)" ]; then \
		echo "Running OpenMPI version (P=$(1)) on $(2):"; \
		cat $(2) | mpirun --oversubscribe -np $(1) ./$(TARGET) $(3); \
	else \
		echo "No MPI executable found. Run 'make' first."; \
		exit 1; \
	fi
endef

# Default target
all: $(TARGET)
	@echo "Modular Fox's Algorithm built successfully!"

# Build the MPI executable (modular version)
$(TARGET): $(OBJECTS)
	$(MPICC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)


# Compile MPI source files (modular)
main.o: main.c mpi_context.h io_handler.h fox_algorithm.h
	$(MPICC) $(CFLAGS) -c $< -o $@

mpi_context.o: mpi_context.c mpi_context.h
	$(MPICC) $(CFLAGS) -c $< -o $@

io_handler.o: io_handler.c io_handler.h mpi_context.h matrix_utils.h
	$(MPICC) $(CFLAGS) -c $< -o $@

fox_algorithm.o: fox_algorithm.c fox_algorithm.h mpi_context.h io_handler.h matrix_utils.h
	$(MPICC) $(CFLAGS) -c $< -o $@

matrix_utils.o: matrix_utils.c matrix_utils.h
	$(MPICC) $(CFLAGS) -c $< -o $@


# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run with different number of processes (using professor's test cases)
run1:
	@if [ -f ./$(TARGET) ]; then \
		echo "Running with 1 process on 6x6 matrix:"; \
		mpirun -np 1 ./$(TARGET) < matrix_examples/input6; \
	else \
		echo "No executable found. Run 'make' first."; \
	fi

run4:
	@if [ -f ./$(TARGET) ]; then \
		echo "Running with 4 processes on 6x6 matrix:"; \
		mpirun -np 4 ./$(TARGET) < matrix_examples/input6; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run9:
	@if [ -f ./$(TARGET) ]; then \
		echo "Running with 9 processes on 6x6 matrix:"; \
		mpirun -np 9 ./$(TARGET) < matrix_examples/input6; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run16:
	@if [ -f ./$(TARGET) ]; then \
		echo "Running with 16 processes on 6x6 matrix:"; \
		mpirun -np 16 ./$(TARGET) < matrix_examples/input6; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run25:
	@if [ -f ./$(TARGET) ]; then \
		echo "Running with 25 processes on 6x6 matrix:"; \
		mpirun -np 25 ./$(TARGET) < matrix_examples/input6; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

# Additional convenient run targets for different matrix sizes
run_small:
	@echo "Running 6x6 matrix with P=1,4,9:"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < matrix_examples/input6
	@echo "P=4:" && mpirun -np 4 ./$(TARGET) < matrix_examples/input6
	@echo "P=9:" && mpirun -np 9 ./$(TARGET) < matrix_examples/input6

run_medium:
	@echo "Running 300x300 matrix with P=1,4,9:"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < matrix_examples/input300
	@echo "P=4:" && mpirun -np 4 ./$(TARGET) < matrix_examples/input300
	@echo "P=9:" && mpirun -np 9 ./$(TARGET) < matrix_examples/input300

run_large:
	@echo "Running 600x600 matrix with P=1,4,9:"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < matrix_examples/input600
	@echo "P=4:" && mpirun -np 4 ./$(TARGET) < matrix_examples/input600
	@echo "P=9:" && mpirun -np 9 ./$(TARGET) < matrix_examples/input600

# Professor's test cases (read-only, do not modify)
test_input:
	@echo "Using professor's test cases from matrix_examples directory"
	@echo "Available test cases:"
	@echo "  - matrix_examples/input5 (5x5 matrix)"
	@echo "  - matrix_examples/input6 (6x6 matrix)"
	@echo "  - matrix_examples/input300 (300x300 matrix)"
	@echo "  - matrix_examples/input600 (600x600 matrix)"
	@echo "  - matrix_examples/input900 (900x900 matrix)"
	@echo "  - matrix_examples/input1200 (1200x1200 matrix)"
	@echo "Test input files are ready in matrix_examples/"

# Alias for backward compatibility
test_inputs: test_input

# Comprehensive testing suite
test: $(TARGET) test_inputs
	$(ensure_lam)
	@echo "=== COMPREHENSIVE TEST SUITE ==="
	@echo "Running functional tests..."
	@$(MAKE) test_functional
	@echo
	@echo "Running performance tests..."
	@$(MAKE) test_performance
	@echo
	@echo "Running validation tests..."
	@$(MAKE) test_validation
	@echo "=== ALL TESTS COMPLETED ==="

# Functional correctness tests using professor's test cases
test_functional: $(TARGET) test_inputs
	@echo "--- Functional Tests ---"
	@echo "Test 1: 5x5 matrix (P=1 vs P=4) - P=4 should error"
	@echo "P=1 (should work):" && cat matrix_examples/input5 | mpirun --oversubscribe -np 1 ./$(TARGET) > test_out_5x5_p1.txt 2>&1
	@echo "P=4 (should error):" && cat matrix_examples/input5 | mpirun --oversubscribe -np 4 ./$(TARGET) > test_out_5x5_p4.txt 2>&1 || true
	@if grep -qi "error\|invalid" test_out_5x5_p4.txt; then \
		echo "✓ 5x5 test PASSED (P=4 correctly rejected invalid configuration)"; \
	else \
		echo "✗ 5x5 test FAILED (P=4 should have errored for 5x5 matrix)"; \
		echo "P=4 output: $$(cat test_out_5x5_p4.txt)"; \
	fi
	@echo
	@echo "Test 2: 6x6 matrix (P=1,9)"
	@echo "P=1:" && cat matrix_examples/input6 | mpirun --oversubscribe -np 1 ./$(TARGET) > test_out_6x6_p1.txt
	@echo "P=9:" && cat matrix_examples/input6 | mpirun --oversubscribe -np 9 ./$(TARGET) > test_out_6x6_p9.txt
	@if diff -q test_out_6x6_p1.txt test_out_6x6_p9.txt > /dev/null; then \
		echo "✓ 6x6 test PASSED (P=1 vs P=9 match)"; \
	else \
		echo "✗ 6x6 test FAILED (P=1 vs P=9 differ)"; \
	fi
	@echo
	@echo "Test 3: 300x300 matrix (P=1,4,9,16,25)"
	@echo "P=1:" && cat matrix_examples/input300 | mpirun --oversubscribe -np 1 ./$(TARGET) > test_out_300x300_p1.txt
	@echo "P=4:" && cat matrix_examples/input300 | mpirun --oversubscribe -np 4 ./$(TARGET) > test_out_300x300_p4.txt
	@echo "P=9:" && cat matrix_examples/input300 | mpirun --oversubscribe -np 9 ./$(TARGET) > test_out_300x300_p9.txt
	@echo "P=16:" && cat matrix_examples/input300 | mpirun --oversubscribe -np 16 ./$(TARGET) > test_out_300x300_p16.txt
	@echo "P=25:" && cat matrix_examples/input300 | mpirun --oversubscribe -np 25 ./$(TARGET) > test_out_300x300_p25.txt
	@if diff -q test_out_300x300_p1.txt test_out_300x300_p4.txt > /dev/null && \
	   diff -q test_out_300x300_p1.txt test_out_300x300_p9.txt > /dev/null && \
	   diff -q test_out_300x300_p1.txt test_out_300x300_p16.txt > /dev/null && \
	   diff -q test_out_300x300_p1.txt test_out_300x300_p25.txt > /dev/null; then \
		echo "✓ 300x300 test PASSED (all process counts match)"; \
	else \
		echo "✗ 300x300 test FAILED (process counts differ)"; \
	fi

# Performance benchmarking
test_performance: $(TARGET) test_inputs
	@echo "--- Performance Tests ---"
	@echo "Testing with multiple matrix sizes..."
	@echo
	@echo "Small matrices (for verification):"
	@echo "6x6 matrix:"
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1 | grep real
	@echo
	@echo "Medium matrices (300x300):"
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1 | grep real
	@echo
	@echo "Large matrices (600x600):"
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1 | grep real

# Validation against professor's expected results
test_validation: $(TARGET) test_inputs
	$(ensure_lam)
	@echo "--- Validation Tests ---"
	@echo "Comparing against professor's expected outputs..."
	@echo
	@echo "Test 1: 6x6 matrix validation"
	@mpirun -np 1 ./$(TARGET) < matrix_examples/input6 > test_actual_output6.txt
	@if diff -q test_actual_output6.txt matrix_examples/output6 > /dev/null; then \
		echo "✓ 6x6 validation PASSED (matches expected output)"; \
	else \
		echo "✗ 6x6 validation FAILED"; \
		echo "Expected (first 3 lines):"; \
		head -n 3 matrix_examples/output6; \
		echo "Actual (first 3 lines):"; \
		head -n 3 test_actual_output6.txt; \
	fi
	@echo
	@echo "Test 2: 300x300 matrix validation"
	@mpirun -np 1 ./$(TARGET) < matrix_examples/input300 > test_actual_output300.txt
	@if diff -q test_actual_output300.txt matrix_examples/output300 > /dev/null; then \
		echo "✓ 300x300 validation PASSED (matches expected output)"; \
	else \
		echo "✗ 300x300 validation FAILED"; \
		echo "Expected (first 3 lines):"; \
		head -n 3 matrix_examples/output300; \
		echo "Actual (first 3 lines):"; \
		head -n 3 test_actual_output300.txt; \
	fi
	@echo
	@echo "Test 3: 600x600 matrix validation"
	@mpirun -np 1 ./$(TARGET) < matrix_examples/input600 > test_actual_output600.txt
	@if diff -q test_actual_output600.txt matrix_examples/output600 > /dev/null; then \
		echo "✓ 600x600 validation PASSED (matches expected output)"; \
	else \
		echo "✗ 600x600 validation FAILED"; \
		echo "Expected (first 3 lines):"; \
		head -n 3 matrix_examples/output600; \
		echo "Actual (first 3 lines):"; \
		head -n 3 test_actual_output600.txt; \
	fi

# Stress testing with larger matrices from professor
test_stress: $(TARGET)
	@echo "--- Stress Tests ---"
	@echo "Testing with largest matrices from professor..."
	@echo
	@echo "900x900 matrix stress test:"
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < matrix_examples/input900 > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < matrix_examples/input900 > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < matrix_examples/input900 > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < matrix_examples/input900 > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < matrix_examples/input900 > /dev/null) 2>&1 | grep real
	@echo
	@echo "1200x1200 matrix stress test:"
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < matrix_examples/input1200 > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < matrix_examples/input1200 > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < matrix_examples/input1200 > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < matrix_examples/input1200 > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < matrix_examples/input1200 > /dev/null) 2>&1 | grep real

# Error testing (invalid inputs)
test_errors: $(TARGET)
	@echo "--- Error Handling Tests ---"
	@echo "Test 1: Invalid process count (P=5, not perfect square)"
	@echo "Expected: Error message about perfect square requirement"
	@mpirun -np 5 ./$(TARGET) < test_input_4x4.txt 2>&1 || true
	@echo
	@echo "Test 2: Matrix size not divisible by grid dimension"
	@echo "7" > test_input_invalid.txt
	@echo "0 1 0 0 0 0 0" >> test_input_invalid.txt
	@echo "1 0 1 0 0 0 0" >> test_input_invalid.txt
	@echo "0 1 0 1 0 0 0" >> test_input_invalid.txt
	@echo "0 0 1 0 1 0 0" >> test_input_invalid.txt
	@echo "0 0 0 1 0 1 0" >> test_input_invalid.txt
	@echo "0 0 0 0 1 0 1" >> test_input_invalid.txt
	@echo "0 0 0 0 0 1 0" >> test_input_invalid.txt
	@echo "Expected: Error about matrix divisibility"
	@mpirun -np 4 ./$(TARGET) < test_input_invalid.txt 2>&1 || true
	@rm -f test_input_invalid.txt

# Performance report generation using professor's test cases
test_report: $(TARGET) test_inputs
	@echo "=== PERFORMANCE REPORT ===" > performance_report.txt
	@echo "Generated on: $$(date)" >> performance_report.txt
	@echo "Using professor's test matrices" >> performance_report.txt
	@echo "" >> performance_report.txt
	@echo "6x6 Matrix Results:" >> performance_report.txt
	@echo "P=1:  $$((/usr/bin/time -f '%e' mpirun -np 1 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=4:  $$((/usr/bin/time -f '%e' mpirun -np 4 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=9:  $$((/usr/bin/time -f '%e' mpirun -np 9 ./$(TARGET) < matrix_examples/input6 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "" >> performance_report.txt
	@echo "300x300 Matrix Results:" >> performance_report.txt
	@echo "P=1:  $$((/usr/bin/time -f '%e' mpirun -np 1 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=4:  $$((/usr/bin/time -f '%e' mpirun -np 4 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=9:  $$((/usr/bin/time -f '%e' mpirun -np 9 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=16: $$((/usr/bin/time -f '%e' mpirun -np 16 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=25: $$((/usr/bin/time -f '%e' mpirun -np 25 ./$(TARGET) < matrix_examples/input300 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "" >> performance_report.txt
	@echo "600x600 Matrix Results:" >> performance_report.txt
	@echo "P=1:  $$((/usr/bin/time -f '%e' mpirun -np 1 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=4:  $$((/usr/bin/time -f '%e' mpirun -np 4 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=9:  $$((/usr/bin/time -f '%e' mpirun -np 9 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=16: $$((/usr/bin/time -f '%e' mpirun -np 16 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=25: $$((/usr/bin/time -f '%e' mpirun -np 25 ./$(TARGET) < matrix_examples/input600 > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "Performance report saved to: performance_report.txt"

# Clean test outputs
clean_tests:
	@rm -f test_out_*.txt test_actual_output*.txt performance_report.txt
	@echo "Test output files cleaned"

# Legacy benchmark target (for backward compatibility)
benchmark: test_performance

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build the modular fox executable"
	@echo "  clean            - Remove build artifacts"
	@echo "  clean_tests      - Remove test output files"
	@echo ""
	@echo "Professor's Test Cases:"
	@echo "  test_input       - Show available professor test cases"
	@echo "  test_inputs      - Alias for test_input"
	@echo ""
	@echo "Testing:"
	@echo "  test             - Run comprehensive test suite"
	@echo "  test_functional  - Test correctness across process counts"
	@echo "  test_performance - Benchmark execution times"
	@echo "  test_validation  - Validate against professor's expected outputs"
	@echo "  test_stress      - Stress test with largest matrices (900x900, 1200x1200)"
	@echo "  test_errors      - Test error handling"
	@echo "  test_report      - Generate comprehensive performance report"
	@echo ""
	@echo "Simple Runs (6x6 matrix):"
	@echo "  run1             - Run with 1 process"
	@echo "  run4             - Run with 4 processes"
	@echo "  run9             - Run with 9 processes"
	@echo "  run16            - Run with 16 processes"
	@echo "  run25            - Run with 25 processes"
	@echo ""
	@echo "Matrix Size Runs:"
	@echo "  run_small        - Run 6x6 matrix with P=1,4,9"
	@echo "  run_medium       - Run 300x300 matrix with P=1,4,9"
	@echo "  run_large        - Run 600x600 matrix with P=1,4,9"
	@echo ""
	@echo "Legacy:"
	@echo "  benchmark        - Alias for test_performance"
	@echo "  help             - Show this help message"

# Declare phony targets
.PHONY: all clean clean_tests test_input test_inputs test test_functional test_performance test_validation test_stress test_errors test_report run1 run4 run9 run16 run25 run_small run_medium run_large benchmark help