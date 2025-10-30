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
	mpirun -np 4 ./$(TARGET) < matrix_examples/input6

# Evitar conflitos com ficheiros reais
.PHONY: all clean
