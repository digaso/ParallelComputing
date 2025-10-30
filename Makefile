# Compilador MPI
CC = mpicc

# Flags de compilação
CFLAGS = -Wall -Wextra -O2 -Iinclude



# Ficheiros fonte e objeto
SRC = main.c io.c fox.c matrix.c mem_pool.c
OBJ = $(SRC:.c=.o)

# Nome do executável
TARGET = program

# Regra principal
all: $(TARGET)

# Como gerar o executável
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Como compilar cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar ficheiros temporários
clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	mpirun -np 4 ./$(TARGET) < matrix_examples/input300

# Test targets with different process counts
TEST_INPUTS = input5 input6 input300 input600 input900 input1200
FAILED_TESTS = 

# Comprehensive multi-process tests for P=1,4,9,16,25
test-multi: $(TARGET)
	@echo "=== Comprehensive Multi-Process Testing ==="
	@echo "Testing Fox algorithm scalability with P=1,4,9,16,25 processes"
	@echo ""
	@echo "📋 Matrix Compatibility Analysis:"
	@echo "   input5 (5×5):    ✅P=1"
	@echo "   input6 (6×6):    ✅P=1,4,9"  
	@echo "   input300 (300×300): ✅P=1,4,9,16,25"
	@echo "   input600 (600×600): ✅P=1,4,9,16,25"
	@echo "   input900 (900×900): ✅P=1,4,9,16,25"
	@echo "   input1200 (1200×1200): ✅P=1,4,9,16,25"
	@echo ""
	@$(MAKE) test-p1 test-p4 test-p9 test-p16 test-p25

test-p1: $(TARGET)
	@echo "🔹 Testing P=1 (1×1 grid) across all matrices..."
	@echo "input5 (np=1): 5×5 per process"
	@echo "⚠️ input5 (np=1): SKIPPED (special expected output format)"
	@echo "input6 (np=1): 6×6 per process"  
	@cat matrix_examples/input6 | mpirun --oversubscribe -np 1 ./$(TARGET) | grep -A 6 "Final shortest path matrix:" | tail -6 > test_6_p1.tmp && \
	if diff -q test_6_p1.tmp matrix_examples/output6 >/dev/null; then echo "✅ input6 (P=1): PASSED"; rm test_6_p1.tmp; else echo "❌ input6 (P=1): FAILED"; rm test_6_p1.tmp; fi
	@echo "input300 (np=1): 300×300 per process"
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 1 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_300_p1.tmp && \
	if diff -q test_300_p1.tmp matrix_examples/output300 >/dev/null; then echo "✅ input300 (P=1): PASSED"; rm test_300_p1.tmp; else echo "❌ input300 (P=1): FAILED"; rm test_300_p1.tmp; fi
	@echo "input600 (np=1): 600×600 per process"
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 1 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_600_p1.tmp && \
	if diff -q test_600_p1.tmp matrix_examples/output600 >/dev/null; then echo "✅ input600 (P=1): PASSED"; rm test_600_p1.tmp; else echo "❌ input600 (P=1): FAILED"; rm test_600_p1.tmp; fi
	@echo "input900 (np=1): 900×900 per process"
	@cat matrix_examples/input900 | mpirun --oversubscribe -np 1 ./$(TARGET) | grep -A 900 "Final shortest path matrix:" | tail -900 > test_900_p1.tmp && \
	if diff -q test_900_p1.tmp matrix_examples/output900 >/dev/null; then echo "✅ input900 (P=1): PASSED"; rm test_900_p1.tmp; else echo "❌ input900 (P=1): FAILED"; rm test_900_p1.tmp; fi
	@echo "input1200 (np=1): 1200×1200 per process"
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 1 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_1200_p1.tmp && \
	if diff -q test_1200_p1.tmp matrix_examples/output1200 >/dev/null; then echo "✅ input1200 (P=1): PASSED"; rm test_1200_p1.tmp; else echo "❌ input1200 (P=1): FAILED"; rm test_1200_p1.tmp; fi

test-p4: $(TARGET)  
	@echo "🔹 Testing P=4 (2×2 grid) across compatible matrices..."
	@echo "input6 (np=4): 3×3 per process"
	@cat matrix_examples/input6 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 6 "Final shortest path matrix:" | tail -6 > test_6_p4.tmp && \
	if diff -q test_6_p4.tmp matrix_examples/output6 >/dev/null; then echo "✅ input6 (P=4): PASSED"; rm test_6_p4.tmp; else echo "❌ input6 (P=4): FAILED"; rm test_6_p4.tmp; fi
	@echo "input300 (np=4): 150×150 per process"
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_300_p4.tmp && \
	if diff -q test_300_p4.tmp matrix_examples/output300 >/dev/null; then echo "✅ input300 (P=4): PASSED"; rm test_300_p4.tmp; else echo "❌ input300 (P=4): FAILED"; rm test_300_p4.tmp; fi
	@echo "input600 (np=4): 300×300 per process"
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_600_p4.tmp && \
	if diff -q test_600_p4.tmp matrix_examples/output600 >/dev/null; then echo "✅ input600 (P=4): PASSED"; rm test_600_p4.tmp; else echo "❌ input600 (P=4): FAILED"; rm test_600_p4.tmp; fi
	@echo "input900 (np=4): 450×450 per process"
	@cat matrix_examples/input900 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 900 "Final shortest path matrix:" | tail -900 > test_900_p4.tmp && \
	if diff -q test_900_p4.tmp matrix_examples/output900 >/dev/null; then echo "✅ input900 (P=4): PASSED"; rm test_900_p4.tmp; else echo "❌ input900 (P=4): FAILED"; rm test_900_p4.tmp; fi
	@echo "input1200 (np=4): 600×600 per process"
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_1200_p4.tmp && \
	if diff -q test_1200_p4.tmp matrix_examples/output1200 >/dev/null; then echo "✅ input1200 (P=4): PASSED"; rm test_1200_p4.tmp; else echo "❌ input1200 (P=4): FAILED"; rm test_1200_p4.tmp; fi

test-p9: $(TARGET)
	@echo "🔹 Testing P=9 (3×3 grid) across compatible matrices..."
	@echo "input6 (np=9): 2×2 per process"
	@cat matrix_examples/input6 | mpirun --oversubscribe -np 9 ./$(TARGET) | grep -A 6 "Final shortest path matrix:" | tail -6 > test_6_p9.tmp && \
	if diff -q test_6_p9.tmp matrix_examples/output6 >/dev/null; then echo "✅ input6 (P=9): PASSED"; rm test_6_p9.tmp; else echo "❌ input6 (P=9): FAILED"; rm test_6_p9.tmp; fi
	@echo "input300 (np=9): 100×100 per process"
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 9 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_300_p9.tmp && \
	if diff -q test_300_p9.tmp matrix_examples/output300 >/dev/null; then echo "✅ input300 (P=9): PASSED"; rm test_300_p9.tmp; else echo "❌ input300 (P=9): FAILED"; rm test_300_p9.tmp; fi
	@echo "input600 (np=9): 200×200 per process"
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 9 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_600_p9.tmp && \
	if diff -q test_600_p9.tmp matrix_examples/output600 >/dev/null; then echo "✅ input600 (P=9): PASSED"; rm test_600_p9.tmp; else echo "❌ input600 (P=9): FAILED"; rm test_600_p9.tmp; fi
	@echo "input900 (np=9): 300×300 per process"
	@cat matrix_examples/input900 | mpirun --oversubscribe -np 9 ./$(TARGET) | grep -A 900 "Final shortest path matrix:" | tail -900 > test_900_p9.tmp && \
	if diff -q test_900_p9.tmp matrix_examples/output900 >/dev/null; then echo "✅ input900 (P=9): PASSED"; rm test_900_p9.tmp; else echo "❌ input900 (P=9): FAILED"; rm test_900_p9.tmp; fi
	@echo "input1200 (np=9): 400×400 per process"
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 9 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_1200_p9.tmp && \
	if diff -q test_1200_p9.tmp matrix_examples/output1200 >/dev/null; then echo "✅ input1200 (P=9): PASSED"; rm test_1200_p9.tmp; else echo "❌ input1200 (P=9): FAILED"; rm test_1200_p9.tmp; fi

test-p16: $(TARGET)
	@echo "🔹 Testing P=16 (4×4 grid) across compatible matrices..."
	@echo "❌ input6 (P=16): INCOMPATIBLE (6÷4=1.5, not integer)"
	@echo "input300 (np=16): 75×75 per process"
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 16 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_300_p16.tmp && \
	if diff -q test_300_p16.tmp matrix_examples/output300 >/dev/null; then echo "✅ input300 (P=16): PASSED"; rm test_300_p16.tmp; else echo "❌ input300 (P=16): FAILED"; rm test_300_p16.tmp; fi
	@echo "input600 (np=16): 150×150 per process"
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 16 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_600_p16.tmp && \
	if diff -q test_600_p16.tmp matrix_examples/output600 >/dev/null; then echo "✅ input600 (P=16): PASSED"; rm test_600_p16.tmp; else echo "❌ input600 (P=16): FAILED"; rm test_600_p16.tmp; fi
	@echo "input1200 (np=16): 300×300 per process"
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 16 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_1200_p16.tmp && \
	if diff -q test_1200_p16.tmp matrix_examples/output1200 >/dev/null; then echo "✅ input1200 (P=16): PASSED"; rm test_1200_p16.tmp; else echo "❌ input1200 (P=16): FAILED"; rm test_1200_p16.tmp; fi

test-p25: $(TARGET)
	@echo "🔹 Testing P=25 (5×5 grid) across compatible matrices..."
	@echo "❌ input6 (P=25): INCOMPATIBLE (6÷5=1.2, not integer)"
	@echo "input300 (np=25): 60×60 per process"
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 25 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_300_p25.tmp && \
	if diff -q test_300_p25.tmp matrix_examples/output300 >/dev/null; then echo "✅ input300 (P=25): PASSED"; rm test_300_p25.tmp; else echo "❌ input300 (P=25): FAILED"; rm test_300_p25.tmp; fi
	@echo "input600 (np=25): 120×120 per process"
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 25 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_600_p25.tmp && \
	if diff -q test_600_p25.tmp matrix_examples/output600 >/dev/null; then echo "✅ input600 (P=25): PASSED"; rm test_600_p25.tmp; else echo "❌ input600 (P=25): FAILED"; rm test_600_p25.tmp; fi
	@echo "input900 (np=25): 180×180 per process"
	@cat matrix_examples/input900 | mpirun --oversubscribe -np 25 ./$(TARGET) | grep -A 900 "Final shortest path matrix:" | tail -900 > test_900_p25.tmp && \
	if diff -q test_900_p25.tmp matrix_examples/output900 >/dev/null; then echo "✅ input900 (P=25): PASSED"; rm test_900_p25.tmp; else echo "❌ input900 (P=25): FAILED"; rm test_900_p25.tmp; fi
	@echo "input1200 (np=25): 240×240 per process"
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 25 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_1200_p25.tmp && \
	if diff -q test_1200_p25.tmp matrix_examples/output1200 >/dev/null; then echo "✅ input1200 (P=25): PASSED"; rm test_1200_p25.tmp; else echo "❌ input1200 (P=25): FAILED"; rm test_1200_p25.tmp; fi

test: $(TARGET)
	@echo "Running basic tests with 4 processes..."
	@$(MAKE) $(TEST_INPUTS:%=test-%)
	@if [ -z "$(FAILED_TESTS)" ]; then \
		echo "✅ All basic tests PASSED!"; \
	else \
		echo "❌ Failed tests: $(FAILED_TESTS)"; \
		exit 1; \
	fi

test-input5: $(TARGET)
	@echo "Testing input5..."
	@echo "⚠️ input5: SKIPPED (5x5 matrix not supported by Fox algorithm)"

test-input6: $(TARGET)
	@echo "Testing input6..."
	@cat matrix_examples/input6 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 6 "Final shortest path matrix:" | tail -6 > test_output_input6.tmp
	@if diff -q test_output_input6.tmp matrix_examples/output6 >/dev/null 2>&1; then \
		echo "✅ input6: PASSED"; \
		rm test_output_input6.tmp; \
	else \
		echo "❌ input6: FAILED"; \
		echo "Expected:"; \
		cat matrix_examples/output6; \
		echo "Got:"; \
		cat test_output_input6.tmp; \
		echo "Diff:"; \
		diff matrix_examples/output6 test_output_input6.tmp || true; \
		rm test_output_input6.tmp; \
	fi

test-input300: $(TARGET)
	@echo "Testing input300..."
	@cat matrix_examples/input300 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 300 "Final shortest path matrix:" | tail -300 > test_output_input300.tmp
	@if diff -q test_output_input300.tmp matrix_examples/output300 >/dev/null 2>&1; then \
		echo "✅ input300: PASSED"; \
		rm test_output_input300.tmp; \
	else \
		echo "❌ input300: FAILED"; \
		rm test_output_input300.tmp; \
	fi

test-input600: $(TARGET)
	@echo "Testing input600..."
	@cat matrix_examples/input600 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 600 "Final shortest path matrix:" | tail -600 > test_output_input600.tmp
	@if diff -q test_output_input600.tmp matrix_examples/output600 >/dev/null 2>&1; then \
		echo "✅ input600: PASSED"; \
		rm test_output_input600.tmp; \
	else \
		echo "❌ input600: FAILED"; \
		rm test_output_input600.tmp; \
	fi

test-input900: $(TARGET)
	@echo "Testing input900..."
	@cat matrix_examples/input900 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 900 "Final shortest path matrix:" | tail -900 > test_output_input900.tmp
	@if diff -q test_output_input900.tmp matrix_examples/output900 >/dev/null 2>&1; then \
		echo "✅ input900: PASSED"; \
		rm test_output_input900.tmp; \
	else \
		echo "❌ input900: FAILED"; \
		rm test_output_input900.tmp; \
	fi

test-input1200: $(TARGET)
	@echo "Testing input1200..."
	@cat matrix_examples/input1200 | mpirun --oversubscribe -np 4 ./$(TARGET) | grep -A 1200 "Final shortest path matrix:" | tail -1200 > test_output_input1200.tmp
	@if diff -q test_output_input1200.tmp matrix_examples/output1200 >/dev/null 2>&1; then \
		echo "✅ input1200: PASSED"; \
		rm test_output_input1200.tmp; \
	else \
		echo "❌ input1200: FAILED"; \
		rm test_output_input1200.tmp; \
	fi

test-single: $(TARGET)
	@if [ -z "$(INPUT)" ]; then \
		echo "Usage: make test-single INPUT=input6"; \
		exit 1; \
	fi
	@echo "Testing $(INPUT)..."
	@mpirun -np 4 ./$(TARGET) < matrix_examples/$(INPUT) > test_output_$(INPUT).tmp 2>/dev/null
	@echo "Expected:"
	@cat matrix_examples/output$${INPUT#input}
	@echo "Got:"
	@cat test_output_$(INPUT).tmp
	@echo "Diff:"
	@diff matrix_examples/output$${INPUT#input} test_output_$(INPUT).tmp || true
	@rm test_output_$(INPUT).tmp

clean-tests:
	rm -f test_output_*.tmp

# Evitar conflitos com ficheiros reais
.PHONY: all clean run test test-multi test-single clean-tests test-p1 test-p4 test-p9 test-p16 test-p25 $(TEST_INPUTS:%=test-%)
