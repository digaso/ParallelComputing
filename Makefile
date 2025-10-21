# Makefile for All-Pairs Shortest Path using Fox's Algorithm
# Project Assignment I: Parallel Computing

# Compiler and flags
MPICC = mpicc
CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99
LDFLAGS = -lm

# Target executables
TARGET = fox
SEQ_TARGET = fox_sequential

# Source files
SOURCES = fox.c
SEQ_SOURCES = fox_sequential.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
SEQ_OBJECTS = $(SEQ_SOURCES:.c=.o)

# Default target (try MPI first, fall back to sequential)
all: 
	@if command -v $(MPICC) >/dev/null 2>&1; then \
		echo "Building MPI version..."; \
		$(MAKE) $(TARGET); \
	else \
		echo "MPI not found, building sequential version..."; \
		$(MAKE) sequential; \
	fi

# Build the MPI executable
$(TARGET): $(OBJECTS)
	$(MPICC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Build the sequential executable
sequential: $(SEQ_TARGET)

$(SEQ_TARGET): $(SEQ_OBJECTS)
	$(CC) $(SEQ_OBJECTS) -o $(SEQ_TARGET) $(LDFLAGS)

# Compile MPI source files
fox.o: fox.c
	$(MPICC) $(CFLAGS) -c $< -o $@

# Compile sequential source files
fox_sequential.o: fox_sequential.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(SEQ_OBJECTS) $(TARGET) $(SEQ_TARGET)

# Run with different number of processes (examples)
run1:
	@if [ -f ./$(TARGET) ]; then \
		mpirun -np 1 ./$(TARGET) < test_input.txt; \
	elif [ -f ./$(SEQ_TARGET) ]; then \
		./$(SEQ_TARGET) < test_input.txt; \
	else \
		echo "No executable found. Run 'make' first."; \
	fi

run4:
	@if [ -f ./$(TARGET) ]; then \
		mpirun -np 4 ./$(TARGET) < test_input.txt; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run9:
	@if [ -f ./$(TARGET) ]; then \
		mpirun -np 9 ./$(TARGET) < test_input.txt; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run16:
	@if [ -f ./$(TARGET) ]; then \
		mpirun -np 16 ./$(TARGET) < test_input.txt; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

run25:
	@if [ -f ./$(TARGET) ]; then \
		mpirun -np 25 ./$(TARGET) < test_input.txt; \
	else \
		echo "MPI version not available. Use 'make run1' for sequential version."; \
	fi

# Create test input files
test_input:
	@echo "Creating test_input.txt (6x6 matrix)..."
	@echo "6" > test_input.txt
	@echo "0 2 0 5 0 0" >> test_input.txt
	@echo "0 0 0 0 0 0" >> test_input.txt
	@echo "0 2 0 0 0 5" >> test_input.txt
	@echo "0 0 0 0 1 0" >> test_input.txt
	@echo "3 9 3 0 0 0" >> test_input.txt
	@echo "0 0 0 0 1 0" >> test_input.txt
	@echo "Test input file created: test_input.txt"

# Create additional test files
test_inputs: test_input
	@echo "Creating test_input_4x4.txt (4x4 cycle)..."
	@echo "4" > test_input_4x4.txt
	@echo "0 1 0 0" >> test_input_4x4.txt
	@echo "0 0 1 0" >> test_input_4x4.txt
	@echo "0 0 0 1" >> test_input_4x4.txt
	@echo "1 0 0 0" >> test_input_4x4.txt
	@echo "Creating test_input_8x8.txt (8x8 random)..."
	@echo "8" > test_input_8x8.txt
	@echo "0 3 0 0 0 7 0 0" >> test_input_8x8.txt
	@echo "3 0 2 0 0 0 0 0" >> test_input_8x8.txt
	@echo "0 2 0 4 0 0 0 0" >> test_input_8x8.txt
	@echo "0 0 4 0 1 0 0 0" >> test_input_8x8.txt
	@echo "0 0 0 1 0 5 0 0" >> test_input_8x8.txt
	@echo "7 0 0 0 5 0 6 0" >> test_input_8x8.txt
	@echo "0 0 0 0 0 6 0 2" >> test_input_8x8.txt
	@echo "0 0 0 0 0 0 2 0" >> test_input_8x8.txt
	@echo "Creating test_input_12x12.txt (12x12 grid)..."
	@echo "12" > test_input_12x12.txt
	@for i in $$(seq 0 11); do \
		line=""; \
		for j in $$(seq 0 11); do \
			if [ $$i -eq $$j ]; then \
				line="$$line 0"; \
			elif [ $$((i + 1)) -eq $$j ] || [ $$((j + 1)) -eq $$i ]; then \
				line="$$line 1"; \
			else \
				line="$$line 0"; \
			fi; \
		done; \
		echo "$$line" >> test_input_12x12.txt; \
	done
	@echo "All test input files created successfully"

# Comprehensive testing suite
test: $(TARGET) test_inputs
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

# Functional correctness tests
test_functional: $(TARGET) test_inputs
	@echo "--- Functional Tests ---"
	@echo "Test 1: 4x4 cycle graph (P=1,4)"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < test_input_4x4.txt > test_out_4x4_p1.txt
	@echo "P=4:" && mpirun -np 4 ./$(TARGET) < test_input_4x4.txt > test_out_4x4_p4.txt
	@if diff -q test_out_4x4_p1.txt test_out_4x4_p4.txt > /dev/null; then \
		echo "✓ 4x4 test PASSED (P=1 vs P=4 match)"; \
	else \
		echo "✗ 4x4 test FAILED (P=1 vs P=4 differ)"; \
	fi
	@echo
	@echo "Test 2: 6x6 original graph (P=1,9)"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < test_input.txt > test_out_6x6_p1.txt
	@echo "P=9:" && mpirun -np 9 ./$(TARGET) < test_input.txt > test_out_6x6_p9.txt
	@if diff -q test_out_6x6_p1.txt test_out_6x6_p9.txt > /dev/null; then \
		echo "✓ 6x6 test PASSED (P=1 vs P=9 match)"; \
	else \
		echo "✗ 6x6 test FAILED (P=1 vs P=9 differ)"; \
	fi
	@echo
	@echo "Test 3: 12x12 grid graph (P=1,4,9)"
	@echo "P=1:" && mpirun -np 1 ./$(TARGET) < test_input_12x12.txt > test_out_12x12_p1.txt
	@echo "P=4:" && mpirun -np 4 ./$(TARGET) < test_input_12x12.txt > test_out_12x12_p4.txt
	@echo "P=9:" && mpirun -np 9 ./$(TARGET) < test_input_12x12.txt > test_out_12x12_p9.txt
	@if diff -q test_out_12x12_p1.txt test_out_12x12_p4.txt > /dev/null && \
	   diff -q test_out_12x12_p1.txt test_out_12x12_p9.txt > /dev/null; then \
		echo "✓ 12x12 test PASSED (all process counts match)"; \
	else \
		echo "✗ 12x12 test FAILED (process counts differ)"; \
	fi

# Performance benchmarking
test_performance: $(TARGET) test_inputs
	@echo "--- Performance Tests ---"
	@echo "Matrix: 12x12 (144 vertices, suitable for scaling)"
	@echo "Measuring execution time (excluding I/O)..."
	@echo
	@echo "Sequential baseline (fox_sequential):"
	@if [ -f ./$(SEQ_TARGET) ]; then \
		time ./$(SEQ_TARGET) < test_input_12x12.txt > /dev/null; \
	else \
		echo "Sequential version not available"; \
	fi
	@echo
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1 | grep real

# Validation against known results
test_validation: $(TARGET) test_inputs
	@echo "--- Validation Tests ---"
	@echo "Checking against expected outputs..."
	@echo "Expected 6x6 result (first row): 0 2 9 5 6 14"
	@echo "Actual result:"
	@mpirun -np 1 ./$(TARGET) < test_input.txt | head -n 1
	@echo
	@echo "Expected 4x4 cycle distances: 0 1 2 3"
	@echo "Actual result:"
	@mpirun -np 1 ./$(TARGET) < test_input_4x4.txt | head -n 1

# Stress testing with larger matrices
test_stress: $(TARGET)
	@echo "--- Stress Tests ---"
	@echo "Creating large test matrices..."
	@echo "24" > test_input_24x24.txt
	@for i in $$(seq 0 23); do \
		line=""; \
		for j in $$(seq 0 23); do \
			if [ $$i -eq $$j ]; then \
				line="$$line 0"; \
			elif [ $$((i + 1)) -eq $$j ] || [ $$((j + 1)) -eq $$i ] || \
			     [ $$((i + 4)) -eq $$j ] || [ $$((j + 4)) -eq $$i ]; then \
				line="$$line 1"; \
			else \
				line="$$line 0"; \
			fi; \
		done; \
		echo "$$line" >> test_input_24x24.txt; \
	done
	@echo "Testing 24x24 matrix with various process counts..."
	@echo "P=1:  " && (time mpirun -np 1 ./$(TARGET) < test_input_24x24.txt > /dev/null) 2>&1 | grep real
	@echo "P=4:  " && (time mpirun -np 4 ./$(TARGET) < test_input_24x24.txt > /dev/null) 2>&1 | grep real
	@echo "P=9:  " && (time mpirun -np 9 ./$(TARGET) < test_input_24x24.txt > /dev/null) 2>&1 | grep real
	@echo "P=16: " && (time mpirun -np 16 ./$(TARGET) < test_input_24x24.txt > /dev/null) 2>&1 | grep real
	@echo "P=25: " && (time mpirun -np 25 ./$(TARGET) < test_input_24x24.txt > /dev/null) 2>&1 | grep real

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

# Performance report generation
test_report: $(TARGET) test_inputs
	@echo "=== PERFORMANCE REPORT ===" > performance_report.txt
	@echo "Generated on: $$(date)" >> performance_report.txt
	@echo "Test matrix: 12x12 (144 vertices)" >> performance_report.txt
	@echo "" >> performance_report.txt
	@echo "Execution Times:" >> performance_report.txt
	@echo "P=1:  $$((/usr/bin/time -f '%e' mpirun -np 1 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=4:  $$((/usr/bin/time -f '%e' mpirun -np 4 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=9:  $$((/usr/bin/time -f '%e' mpirun -np 9 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=16: $$((/usr/bin/time -f '%e' mpirun -np 16 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "P=25: $$((/usr/bin/time -f '%e' mpirun -np 25 ./$(TARGET) < test_input_12x12.txt > /dev/null) 2>&1)s" >> performance_report.txt
	@echo "Performance report saved to: performance_report.txt"

# Clean test outputs
clean_tests:
	@rm -f test_out_*.txt test_input_*.txt performance_report.txt
	@echo "Test output files cleaned"

# Legacy benchmark target (for backward compatibility)
benchmark: test_performance

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build the fox executable"
	@echo "  sequential       - Build sequential version"
	@echo "  clean            - Remove build artifacts"
	@echo "  clean_tests      - Remove test output files"
	@echo ""
	@echo "Test Input Generation:"
	@echo "  test_input       - Create original 6x6 test file"
	@echo "  test_inputs      - Create all test input files (4x4, 6x6, 8x8, 12x12)"
	@echo ""
	@echo "Testing:"
	@echo "  test             - Run comprehensive test suite"
	@echo "  test_functional  - Test correctness across process counts"
	@echo "  test_performance - Benchmark execution times"
	@echo "  test_validation  - Validate against expected outputs"
	@echo "  test_stress      - Stress test with larger matrices"
	@echo "  test_errors      - Test error handling"
	@echo "  test_report      - Generate performance report"
	@echo ""
	@echo "Simple Runs:"
	@echo "  run1             - Run with 1 process"
	@echo "  run4             - Run with 4 processes"
	@echo "  run9             - Run with 9 processes"
	@echo "  run16            - Run with 16 processes"
	@echo "  run25            - Run with 25 processes"
	@echo ""
	@echo "Legacy:"
	@echo "  benchmark        - Alias for test_performance"
	@echo "  help             - Show this help message"

# Declare phony targets
.PHONY: all sequential clean clean_tests test_input test_inputs test test_functional test_performance test_validation test_stress test_errors test_report run1 run4 run9 run16 run25 benchmark help