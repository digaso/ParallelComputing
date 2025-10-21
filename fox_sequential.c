#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>

#define INF INT_MAX

// Function prototypes
void read_input_matrix(int **matrix, int n);
void print_matrix(int **matrix, int n);
int **allocate_matrix(int rows, int cols);
void free_matrix(int **matrix, int rows);
void min_plus_multiply(int **A, int **B, int **C, int n);
void floyd_warshall(int **matrix, int n);
double get_time();

int main(int argc, char *argv[]) {
    int n;
    int **matrix = NULL;
    double start_time, end_time;
    
    // Read matrix dimension
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error reading matrix dimension\n");
        return 1;
    }
    
    // Allocate memory for matrix
    matrix = allocate_matrix(n, n);
    
    // Read input matrix
    read_input_matrix(matrix, n);
    
    // Start timing (excluding I/O)
    start_time = get_time();
    
    // Apply Floyd-Warshall algorithm
    floyd_warshall(matrix, n);
    
    // End timing
    end_time = get_time();
    
    // Print results
    print_matrix(matrix, n);
    fprintf(stderr, "Sequential execution time: %.2f ms\n", (end_time - start_time) * 1000);
    
    // Cleanup
    free_matrix(matrix, n);
    
    return 0;
}

void read_input_matrix(int **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scanf("%d", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix element [%d][%d]\n", i, j);
                exit(1);
            }
            // Convert 0 to INF for non-diagonal elements (no direct path)
            if (i != j && matrix[i][j] == 0) {
                matrix[i][j] = INF;
            }
        }
    }
}

void print_matrix(int **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] == INF) {
                printf("0");
            } else {
                printf("%d", matrix[i][j]);
            }
            if (j < n - 1) printf(" ");
        }
        printf("\n");
    }
}

int **allocate_matrix(int rows, int cols) {
    int **matrix = (int **)malloc(rows * sizeof(int *));
    if (matrix == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    return matrix;
}

void free_matrix(int **matrix, int rows) {
    if (matrix != NULL) {
        for (int i = 0; i < rows; i++) {
            free(matrix[i]);
        }
        free(matrix);
    }
}

void min_plus_multiply(int **A, int **B, int **C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = INF;
            for (int k = 0; k < n; k++) {
                if (A[i][k] != INF && B[k][j] != INF) {
                    int sum = A[i][k] + B[k][j];
                    if (sum < C[i][j]) {
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

void floyd_warshall(int **matrix, int n) {
    // Floyd-Warshall algorithm for all-pairs shortest paths
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (matrix[i][k] != INF && matrix[k][j] != INF) {
                    int sum = matrix[i][k] + matrix[k][j];
                    if (sum < matrix[i][j]) {
                        matrix[i][j] = sum;
                    }
                }
            }
        }
    }
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}